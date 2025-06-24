#pragma once

#include "data.hpp"

#include <cassert>

enum class Direction { none=0, up, down, left, right };

using Distance = float;

namespace platformer {
	inline float cameraShiftVel = 40; // pixels per second when multiplied by delta

	void process(GameData& gameData, TimeSeconds delta);
	
	void draw(GameData& gameData);
};