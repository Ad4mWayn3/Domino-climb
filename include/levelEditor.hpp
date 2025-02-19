#pragma once

#include "data.hpp"

void editProcess(float delta, EditData& editData);

void loadMap(std::vector<Rectangle>& mapData, const char* mapFileName);

void saveMap(const std::vector<Rectangle>& mapData, const char* mapFileName);
