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

	return true;
} 

void Player::moveAxis(int moveAm, bool& axisCollide, Vector2 shift, Rectangles recs) {
	while (moveAm) {
		if (!checkCollisions({float(x+shift.x), float(y+shift.y), \
			(float)width, (float)height}, recs)) {
			axisCollide = true;
			return;
		}
		x += shift.x;
		y += shift.y;
		moveAm -= shift.x + shift.y;
	}
	axisCollide = false;
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
	Vector2 mouse = {(float)GetMouseX(), (float)GetMouseY()};
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

#ifdef DEBUG
	std::stringstream ss;
	ss << "colliding x: " << (player.collidingX? "true" : "false")
		<< "\ncolliding y: " << (player.collidingY? "true" : "false")
		<< "\nx: " << player.position().x
		<< "\ny: " << player.position().y
		<< "\nmouse x: " << GetMouseX()
		<< "\nmouse y: " << GetMouseY()
	;
	
	DrawText(ss.str().c_str(), 30, 30, 20, WHITE);
#endif

}
