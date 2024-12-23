#include "platformer.hpp"

bool checkCollisions(Rectangle rec, Rectangles recs) {
	for (auto i : recs)
		if (CheckCollisionRecs(rec, i)) return false;
	return true;
}

Player& Player::moveX(float speed, Rectangles recs) {
	static float remainder=0;
	remainder += speed;
	int move=round(remainder);
	remainder -= move;
	int sign = move < 0? -1 : 1;

	moveAxis(move, collidingX, {(float)sign,0}, recs);
	return *this;
}

Player& Player::moveY(float speed, Rectangles recs) {
	static float remainder=0;
	remainder += speed;
	int move=round(remainder);
	remainder -= move;
	int sign = move < 0? -1 : 1;

	moveAxis(move, collidingY, {0, (float)sign}, recs);
	return *this;
}

float bounce(float speed, float bounciness) {
	if (bounciness < 0) std::cout << "warning, negative bounciness\n";
	if (fabs(speed) < MIN_BOUNCE_SPEED) return 0;
	return -speed * bounciness;
}

void fixCollision(Rectangle& player, Vector2& velocity, Rectangle structure, \
	float bounciness=0) {
	if (bounciness < 0) bounciness = -bounciness;
	if (!CheckCollisionRecs(player, structure)) return;

	auto collision = GetCollisionRec(player, structure);
	Vector2 shift{0,0};
	if (collision.width < collision.height) {
//		velocity.x = (-velocity.x) * bounciness;
		velocity.x = bounce(velocity.x, bounciness);
		if (player.x < structure.x) shift = {-1,0};
		else shift = {1,0};
	} else {
//		velocity.y = (-velocity.y) * bounciness;
		velocity.y = bounce(velocity.y, bounciness);
		if (player.y < structure.y) shift = {0,-1};
		else shift = {0,1};
	}

	while (CheckCollisionRecs(player, structure)) {
		player.x += shift.x;
		player.y += shift.y;
	}

	bool collidingDown = CheckCollisionRecs({player.x, player.y+1, \
		player.width, player.height}, structure);
	bool collidingSides = CheckCollisionRecs({player.x+1, player.y, \
		player.width, player.height}, structure) ||
		CheckCollisionRecs({player.x-1, player.y, player.width, \
		player.height}, structure);
	
	if (collidingDown && bounciness == 0
		|| fabs(velocity.y) < 60) velocity.y = 0;
	if (collidingSides && bounciness == 0
		|| fabs(velocity.x) < 60) velocity.x = 0;
}

float drag(float speed, float dragRate, float delta) {
	float newSpeed = (speed < 0)? 
		speed + dragRate * delta
	:
		speed - dragRate * delta;

	return fabs(speed - newSpeed) < fabs(speed)? newSpeed : 0;
}

float capSpeed(float speed, float speedCap) {
	if (fabs(speed) > speedCap)
		return speed < 0? -speedCap : speedCap;
	return speed;
}

void _process(float delta, GameData& gameData) {
	auto& controls = gameData.controls;
	auto& player = gameData.player;
	bool moving = false;
	static Vector2 velocity = {0,0};
	static bool collidingDown = false;
	static bool collidingSides = false;
	auto testCollidingSides = [](Rectangle r1, Rectangle r2) {
		return CheckCollisionRecs({r1.x+1, r1.y, r1.width, r1.height}, r2) \
			|| CheckCollisionRecs({r1.x-1, r1.y, r1.width, r1.height}, r2);
	};

	if (IsKeyDown(controls[up]))
		velocity.y -= gameData.accel.y * delta;
	if (IsKeyDown(controls[down]))
		velocity.y += gameData.accel.y * delta;
	if (IsKeyDown(controls[left])) {
		velocity.x -= gameData.accel.x * delta;
		moving = true;
	}
	if (IsKeyDown(controls[right])) {
		velocity.x += gameData.accel.x * delta;
		moving = true;
	}
	if (IsKeyDown(controls[jump]) && collidingDown)
		velocity.y = -gameData.jumpImpulse;
	if (IsKeyPressed(controls[reset]))
		gameData = {};

	collidingDown = false;
	collidingSides = false;
	bool collidingSpd = false;
	for (auto& structure : gameData.structures) {
		fixCollision(player,velocity,structure,0);
		collidingDown = collidingDown || CheckCollisionRecs({player.x, \
			player.y+1, player.width, player.height}, structure);
		collidingSides = collidingSides || testCollidingSides(player,structure);
		collidingSpd = collidingSpd || CheckCollisionRecs({player.x + velocity.x, 
			player.y, player.width, player.height}, structure);
	}
	collidingDown = collidingDown \
		&& fabs(velocity.y) < fabs(gameData.jumpImpulse);

	if (!collidingDown) velocity += gameData.gravity * delta;

	if (collidingDown && !moving)
		velocity.x = drag(velocity.x, gameData.accel.x * 2, delta);

	velocity.x = capSpeed(velocity.x, gameData.maxVel.x);
	velocity.y = capSpeed(velocity.y, gameData.maxVel.y);

	for (auto& structure : gameData.structures) {
		DrawRectangleRec(structure, BLUE);
	}
	
	DrawRectangleRec(gameData.player, LIGHTGRAY);

	if (velocity.x > 40);
		player.x += velocity.x * delta;
	player.y += velocity.y * delta;

	std::stringstream ss;
	ss << "Y speed: " << velocity.y << \
		"\nX speed: " << velocity.x << '\n';
	DrawText(ss.str().c_str(), 30, 30, 20, WHITE);
}
