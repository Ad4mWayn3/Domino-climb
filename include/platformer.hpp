#pragma once

#include "data.hpp"

namespace platformer {
	void process(GameData& gameData, TimeSeconds delta);
	
	void draw(GameData& gameData);
};