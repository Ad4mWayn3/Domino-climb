#include "levelEditor.hpp"

Rectangle fixNegativeDims(Rectangle rec) {
	if (rec.width < 0) {
		rec.x += rec.width;
		rec.width = -rec.width;
	}
	if (rec.height < 0) {
		rec.y += rec.height;
		rec.height = -rec.height;
	}
	return rec;
}

Rectangle* getRectangleAt(Vector2 position, \
	std::vector<Rectangle>& rectangles) {
	for (Rectangle& rec : rectangles) {
		if (position.x > rec.x && position.x < (rec.x + rec.width))
		if (position.y > rec.y && position.y < (rec.y + rec.height))
			return &rec;	
	}

	return nullptr;
}

void loadMap(std::vector<Rectangle>& mapData, const char* mapFileName) {
	std::ifstream mapFile{mapFileName};
	if (mapData.size() != 0) mapData = {};

	Rectangle rec;
	while (true) {
		if (!(mapFile >> rec.x)) break;
		if (!(mapFile >> rec.y)) break;
		if (!(mapFile >> rec.width)) break;
		if (!(mapFile >> rec.height)) break;
		mapData.push_back(rec);
	}

	mapFile.close();
	return;
}

void saveMap(const std::vector<Rectangle>& mapData, const char* mapFileName) {
	std::ofstream mapFile;
	mapFile.open(mapFileName);
	std::cout << "saving " << mapFileName << '\n';
	for (auto& rec : mapData)
		mapFile << rec.x << ' ' << rec.y << ' ' << rec.width <<  ' '
			<< rec.height << '\n';

	mapFile.close();
	return;
}

bool inRectangle(Vector2 pos, Rectangle rec) {
	return pos.x >= rec.x && pos.x <= (rec.x + rec.width)
		&& pos.y >= rec.y && pos.y <= (rec.y + rec.height);
}

void editStructure(Rectangle& structure, int* controls, Vector2 mouseDelta) {
	bool repos=false; // re-position
	if (IsMouseButtonDown(controls[select])) {
		structure.x += mouseDelta.x;
		structure.y += mouseDelta.y;
	}
	if (IsKeyDown(controls[resize_w])) {
		repos = true;
		structure.width += mouseDelta.x;
		if (structure.width < constData.minLength)
			structure.width = constData.minLength;
	}
	if (IsKeyDown(controls[resize_h])) {
		repos = true;
		structure.height += mouseDelta.y;
		if (structure.height < constData.minLength)
			structure.height = constData.minLength;
	}

	auto keyBehavior = IsKeyPressed;

	structure.x += keyBehavior(controls[move_right]);
	structure.x -= keyBehavior(controls[move_left]);
	structure.y += keyBehavior(controls[move_down]);
	structure.y -= keyBehavior(controls[move_up]);
}

void editProcess(float delta, EditData& editData) {
	auto& camera = editData.camera;
	auto& selectedStructure = editData.selectedStructure;
	Vector2 mouseMapPos = GetMousePosition() + camera.target;
	static bool creating = false;
	static Rectangle newStructure{mouseMapPos.x, mouseMapPos.y, 0.0f, 0.0f};

	if (!creating) {
		newStructure.x = roundf(mouseMapPos.x);
		newStructure.y = roundf(mouseMapPos.y);
	}
	newStructure.width = roundf(mouseMapPos.x - newStructure.x);
	newStructure.height = roundf(mouseMapPos.y - newStructure.y);

	if (IsMouseButtonDown(editData.controls[scroll])) {
		editData.camera.target -= GetMouseDelta();
	}
	if (IsMouseButtonPressed(editData.controls[select])) {
		auto rectangleAtMouse = getRectangleAt(mouseMapPos, editData.structures);
		if (selectedStructure) {
			if (!inRectangle(mouseMapPos, *selectedStructure))
				selectedStructure = rectangleAtMouse;
		} else selectedStructure = rectangleAtMouse;
	}
	if (IsKeyDown(editData.controls[create]))
		creating = true;
	else
		creating = false;
	if (IsKeyPressed(editData.controls[_delete]) && selectedStructure) {
		auto& xs = editData.structures;
		std::swap(*selectedStructure, xs[xs.size()-1]);
		xs.pop_back();
		selectedStructure = nullptr;
	}
	if (IsKeyPressed(editData.controls[copy])) {
		auto& newRec = selectedStructure;
		if (newRec) editData.structures.push_back(
			{newRec->x + 10, newRec->y + 10, newRec->width, newRec->height}
		);
		if (newRec)
			selectedStructure = &editData.structures[editData.structures.size()-1];
	}
	if (IsKeyPressed(editData.controls[save])) {
		saveMap(editData.structures, "resources/map.txt");
	}
	if (IsKeyPressed(editData.controls[load])) {
		loadMap(editData.structures, "resources/map.txt");
	}

	BeginMode2D(editData.camera);

	for (auto& structure : editData.structures)
		DrawRectangleRec(structure, BROWN);

	if (!creating && newStructure.width != 0) {
		editData.structures.push_back(fixNegativeDims(newStructure));
	}

	DrawRectangleLinesEx(fixNegativeDims(newStructure), 5, WHITE);
	if (selectedStructure) {
		DrawRectangleLinesEx(*selectedStructure, 5, WHITE);
		editStructure(*selectedStructure, editData.controls, GetMouseDelta());
	}

	EndMode2D();

	DrawText(constData.controlsDisplay,30,30,20,WHITE);	
}
