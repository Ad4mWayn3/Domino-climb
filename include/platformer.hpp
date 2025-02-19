#pragma once

#include "data.hpp"

bool checkCollisions(Rectangle rec, Rectangles recs);

bool checkAdjacents(Rectangle rec, Rectangles recs, Axis axis, bool& onGround);

void process(float delta, GameData& gameData);