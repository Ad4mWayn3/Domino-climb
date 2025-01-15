#include "platformer.hpp"

// this one is a bit screwed because it returns false when there IS a \
	collision, which is actually less intuitive, I won't change it since \
	plenty of the logic is built around this problem.
bool checkCollisions(Rectangle rec, Rectangles recs) {
	for (auto i : recs)
		if (CheckCollisionRecs(rec, i)) return false;
	return true;
}


// this function can probably be optimized to check a single rectangle \
	(see Player::moveAxis).
bool checkAdjacents(Rectangle rec, Rectangles recs, Axis axis, bool& onGround) {
	auto shift = [](Rectangle rec, Vector2 shift)->Rectangle{
		return {rec.x + shift.x, rec.y + shift.y, rec.width, rec.height};
	};
	Vector2 left{1,0}, right{-1,0}, down{0,1}, up{0,-1};

	if (axis == horizontal)
		return !checkCollisions(shift(rec, left), recs)
			|| !checkCollisions(shift(rec, right), recs);
	if (axis == vertical) {
		onGround = !checkCollisions(shift(rec, down), recs);
		return !checkCollisions(shift(rec, up), recs) || onGround;
	}
} 

Player& Player::moveX(float speed, Rectangles recs) {
	static float remainder=0;
	remainder += speed;
	int move=round(remainder);
	remainder -= move;
	int sign = move < 0? -1 : 1;

	moveAxis(move, collidingX, {(float)sign,0}, recs);
	collidingX = checkAdjacents(rectangle(), recs, horizontal, onGround);
	return *this;
}

Player& Player::moveY(float speed, Rectangles recs) {
	static float remainder=0;
	remainder += speed;
	int move=round(remainder);
	remainder -= move;
	int sign = move < 0? -1 : 1;

	moveAxis(move, collidingY, {0, (float)sign}, recs);
	collidingY = checkAdjacents(rectangle(), recs, vertical, onGround);
	return *this;
}

float bounce(float speed, float bounciness) {
	if (bounciness < 0) std::cout << "warning, negative bounciness\n";
	if (fabs(speed) < MIN_BOUNCE_SPEED) return 0;
	return -speed * bounciness;
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

// *** this function is called assuming a Mode2D context ***
void cameraFollowRadius(Camera2D& camera, Player& player, float maxOffset) {
	Vector2 camCenter = camera.target + (resolutionV / 2);
	Vector2 diff = player.position() - camCenter;
	
	// camera moves in the player's direction until the offset is in bounds.
	while (Vector2LengthSqr(diff) > (maxOffset * maxOffset)) {
		camCenter = camera.target + (resolutionV / 2);
		diff = player.position() - camCenter;
		camera.target += Vector2Normalize(diff);
	}
}

void cameraCursorLook(Camera2D& camera, Player& player, float maxOffset) {
	Vector2 camCenter = camera.target + (resolutionV / 2);
	camera.target = player.position() - resolutionV / 2 ;
	Vector2 mouse = {GetMouseX(), GetMouseY()};
	Vector2 playerToMouse = mouse - resolutionV / 2;
	camera.target += Vector2LengthSqr(playerToMouse) > (maxOffset*maxOffset) ?
		Vector2Normalize(playerToMouse) * maxOffset : playerToMouse;
}

void process(float delta, GameData& gameData) {
	auto& player = gameData._player;
	float friction = 5000;
	bool moving = false;

	BeginMode2D(gameData.camera);

	DrawRectangleRec(player.rectangle(), WHITE);
	for (auto& structure : gameData.structures) {
		DrawRectangleRec(structure, BLUE);
	}

	EndMode2D();

	static Vector2 vel{0,0};

	if (IsKeyPressed(gameData.controls[jump]) && player.onGround) {
		vel.y = -gameData.jumpImpulse;
	}
	if (IsKeyPressed(gameData.controls[toggleCamera])) {
		gameData.cameraMode += 1;
		gameData.cameraMode %= gameData.cameraModeCount;
	}
	if (IsKeyDown(gameData.controls[right])) {
		vel.x += gameData.accel.x * delta;
		if (vel.x < 0 && player.onGround)
			vel.x = drag(vel.x, friction, delta);
		moving = true;
	}
	if (IsKeyDown(gameData.controls[left])) {
		vel.x -= gameData.accel.x * delta;
		if (vel.x > 0 && player.onGround)
			vel.x = drag(vel.x, friction, delta);
		moving = true;
	}

	vel.x = capSpeed(vel.x, gameData.maxVel.x);
	vel.y = capSpeed(vel.y, gameData.maxVel.y);

	player.moveX(vel.x * delta, gameData.structures)
		.moveY(vel.y * delta, gameData.structures);
	
	switch (gameData.cameraMode) {
	case mouselook:
		cameraCursorLook(gameData.camera, player, 400);
		break;
	case follow:
		cameraFollowRadius(gameData.camera, player, 400);
		break;
	}


	if (player.collidingY) vel.y = 0;
	if (!player.onGround) vel.y += gameData.gravity.y * delta;
	else if (!moving)
		vel.x = drag(vel.x, friction, delta);

	if (player.collidingX) vel.x = 0;
	
	std::stringstream ss;
	ss << "colliding x: " << (player.collidingX? "true" : "false")
		<< "\ncolliding y: " << (player.collidingY? "true" : "false")
		<< "\nx: " << player.position().x
		<< "\ny: " << player.position().y
		<< "\nmouse x: " << GetMouseX()
		<< "\nmouse y: " << GetMouseY()
	;
	
	DrawText(ss.str().c_str(), 30, 30, 20, WHITE);
}

#pragma region legacy engine
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
#pragma endregion
