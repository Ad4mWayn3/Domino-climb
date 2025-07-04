#include "platformer.hpp"

// returns true when there are no collissions. should be backwards but too much \
	logic is already built around this behavior
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

void Player::draw(Image& tex) {
	//	DrawRectangleRec(rectangle(), WHITE);
	//	static int prevRot = rotation;
	assert(tex.format != PIXELFORMAT_UNCOMPRESSED_GRAYSCALE
		&& tex.format != PIXELFORMAT_COMPRESSED_DXT1_RGB
		&& "texture does not have alpha channel"
	);
	
	//	if (prevRot != rotation) ImageRotate(&tex, rotation);
	DrawTexturePro( LoadTextureFromImage(tex),
		{0,0,(float)tex.width,(float)tex.height}, rectangle(),
		{0,0}, rotation, WHITE
	);
}

Rectangle Player::rectangle() {
	if (rotation % 180 == 0) {
		Vector2 axis = {x + width/2, y + height/2};
		Vector2 newOrigin = {-axis.y, axis.x - height}; // this is the axis rotated
		// 90 degrees counterclockwise, doesn't look intuitive but trust the math :)
		Rectangle rotateRec = {x + axis.x + newOrigin.x, y - (axis.y + newOrigin.y),
			height, width};
		assert(rotateRec.width > 0 && rotateRec.height > 0);
		return rotateRec;
	}

	return {(float)x, (float)y, (float)width, (float)height};
}

// the axis is relative to the player's position
bool Player::rotate(Vector2 axis, Rectangles recs, bool forceRotate) {
	Vector2 newOrigin = {-axis.y, axis.x - height}; // this is the axis rotated
	// 90 degrees counterclockwise, doesn't look intuitive but trust the math :)
	Rectangle rotateRec = {x + axis.x + newOrigin.x, y - (axis.y + newOrigin.y),
		height, width};

	bool rotated = checkCollisions(rotateRec, recs);

	if (!rotated && forceRotate) {
		int d = maxFixCollisionSize();
		for (int i=0; i < d; ++i) {
			rotateRec.y -= 1;
			if (checkCollisions(rotateRec, recs)) goto assign;
		}
		if (!checkCollisions(rotateRec, recs)){
			rotated = false;
			forceRotate = false;
		}
	}

	if (rotated || forceRotate) {
	assign:
		rotation += 90;
		if (rotation > 360) rotation -= 360;
//		x = rotateRec.x;
//		y = rotateRec.y;
//		width = rotateRec.width;
//		height = rotateRec.height;
	} else DrawRectangleRec(rotateRec, BROWN);

	return rotated || forceRotate;
}

void Player::moveAxis(int moveAm, bool& axisCollide, Vector2 shift, Rectangles recs) {
	while (moveAm) {
		Rectangle box = rectangle();
		assert(box.width > 0 && box.height > 0);
		box.x += shift.x;
		box.y += shift.y;

		if (!checkCollisions(box, recs)) {
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
	return Clamp(speed, -speedCap, speedCap);
}

// should be called inside a Mode2D context
void cameraFollowRadius(Camera2D& camera, Vector2 target, float maxOffset) {
	Vector2 camCenter = camera.target + (resolutionV / 2);
	Vector2 diff = target - camCenter;
	
	// camera moves in the player's direction until the offset is in bounds.
	while (Vector2LengthSqr(diff) > (maxOffset * maxOffset)) {
		camCenter = camera.target + (resolutionV / 2);
		diff = target - camCenter;
		camera.target += Vector2Normalize(diff);
	}
}

// should be called inside a Mode2D context
void cameraCursorLook(Camera2D& camera, Vector2 target, float maxOffset) {
	Vector2 camCenter = camera.target + (resolutionV / 2);
	camera.target = target - resolutionV / 2 ;
	Vector2 mouse = {(float)GetMouseX(), (float)GetMouseY()};
	Vector2 playerToMouse = mouse - resolutionV / 2;
	camera.target += Vector2LengthSqr(playerToMouse) > (maxOffset*maxOffset) ?
		Vector2Normalize(playerToMouse) * maxOffset : playerToMouse;
}

Vector2 cameraFollow(Vector2 target, Vector2 offset) {
	return target - resolutionV / 2 + offset;
}

// note: drawing can be handled inside process, but doing it separately \
	allows the game to be paused and still be rendered
void platformer::draw(GameData& gameData) {
	BeginMode2D(gameData.camera);

	gameData.player.draw(gameData.domino);

	for (auto& structure : gameData.structures) {
		DrawRectangleRec(structure, BLUE);
	}

	EndMode2D();
}

void platformer::process(GameData& gameData, float delta) {
	auto& player = gameData.player;
	float friction = 5000;
	bool moving = false;

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

	static bool prone = false;
	BeginMode2D(gameData.camera);
	if (IsKeyDown(gameData.controls[down])) {
		if (!prone)
			prone = player.rotate({player.width / 2.0f, player.height / 2.0f},
				gameData.structures);
//		prone = true;
	} else {
		if (prone)
			prone = !player.rotate({player.width / 2.0f, player.height / 2.0f},
				gameData.structures, true);
	}
	EndMode2D();

	vel.x = capSpeed(vel.x, gameData.maxVel.x);
	vel.y = capSpeed(vel.y, gameData.maxVel.y);

	player.moveX(vel.x * delta, gameData.structures)
		.moveY(vel.y * delta, gameData.structures);

	gameData.camera.target = cameraFollow(player.center(), {0.0f, 0.0f});

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
