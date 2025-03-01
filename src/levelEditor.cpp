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

Rectangle getOuterRectangle(const std::vector<Rectangle*>& recs) {
	if (recs.size() == 0) return {0.0f, 0.0f, 0.0f, 0.0f};
	Vector2 origin = {recs.at(0)->x, recs.at(0)->y}, end = {0,0};
	for (auto& rec : recs) {
		if (rec->x < origin.x) origin.x = rec->x;
		if (rec->y < origin.y) origin.y = rec->y;
		if (rec->x + rec->width > end.x) end.x = rec->x + rec->width;
		if (rec->y + rec->height > end.y) end.y = rec->y + rec->height;
	}

	return {origin.x, origin.y, end.x-origin.x, end.y-origin.y};
}

// checks if a point (vector2) is inside a rectangle
bool inRectangle(Vector2 pos, Rectangle rec) {
	return pos.x >= rec.x && pos.x <= (rec.x + rec.width)
		&& pos.y >= rec.y && pos.y <= (rec.y + rec.height);
}

// selects the rectangles that collide with "selector"
void updateSelected(std::vector<Rectangle>& structures,
	std::vector<Rectangle*>& selected, Rectangle selector) {
	if (selected.capacity() < structures.size())
		selected.reserve(structures.size());

//	selected.resize(0);
	for (auto& rec : structures)
		if (CheckCollisionRecs(selector, rec))
			selected.push_back(&rec);

	#ifdef DEBUG
	std::cout << "updateSelected(recs, selected, selector)\n";
	#endif
}

// TODO: delete this function; moving, resizing and copying should be separate
// functions
void editStructure(EditData& data, Rectangle& structure, TimeSeconds delta,
	Vector2 mouseDelta) {
	auto& controls = data.controls;

	if (IsMouseButtonDown(controls[select])) {
		structure.x += mouseDelta.x;
		structure.y += mouseDelta.y;
	}
	bool repos=false; // re-position
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

	auto isKeyBuffered = [delta](KeyboardKey key, TimeSeconds& timePressed,
		TimeSeconds bufferDelay) {
		if (IsKeyDown(key)) timePressed += delta;
		else timePressed = 0;
		return timePressed >= bufferDelay;
	};

	static std::map<EditControls, float> keyPressTime{
		{move_right, 0},
		{move_left, 0},
		{move_down, 0},
		{move_up, 0},
	};
	auto delay = data.inputBufferDelay;
#define K_BUFFER(k) isKeyBuffered((KeyboardKey)controls[k], keyPressTime[k], delay)
	structure.x += K_BUFFER(move_right);
	structure.x -= K_BUFFER(move_left);
	structure.y += K_BUFFER(move_down);
	structure.y -= K_BUFFER(move_up);
#undef K_BUFFER
}

void editor::loadMap(std::vector<Rectangle>& mapData, const char* mapFileName) {
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

// TODO: It should run on a separate thread
void editor::saveMap(const std::vector<Rectangle>& mapData, const char* mapFileName) {
	std::ofstream mapFile;
	mapFile.open(mapFileName);
	std::cout << "saving " << mapFileName << '\n';
	for (auto& rec : mapData)
		mapFile << floor(rec.x) << ' ' << floor(rec.y) << ' ' \
			<< floor(rec.width) << ' ' << floor(rec.height) << '\n';

	mapFile.close();
	return;
}

void editor::process(EditData& editData, TimeSeconds delta) {
	Camera2D& camera = editData.camera;
//	auto& selectedStructure = editData.selectedStructure;
	Rectangle& newStructure = editData.newStructure;
	Rectangle& selector = editData.selector;
	bool& creating = editData.creating;
	Vector2 mouseMapPos = GetMousePosition() + camera.target;

	if (!creating) {
		newStructure.x = roundf(mouseMapPos.x);
		newStructure.y = roundf(mouseMapPos.y);
	}
	newStructure.width = roundf(mouseMapPos.x - newStructure.x);
	newStructure.height = roundf(mouseMapPos.y - newStructure.y);

	if (IsMouseButtonDown(editData.controls[scroll])) {
		editData.camera.target -= GetMouseDelta();
	}

	{
		bool selecting = false;
		if (IsMouseButtonPressed(editData.controls[select])) {
			selector.x = mouseMapPos.x;
			editData.selector.y = mouseMapPos.y;
			if (!IsKeyDown(editData.controls[select_multiple]))
				editData.selectedStructures.resize(0);
			auto rec = getRectangleAt(mouseMapPos, editData.structures);
			if (rec) editData.selectedStructures.push_back(rec);
		} else if (IsMouseButtonDown(editData.controls[select])) {
			selector.width = mouseMapPos.x - selector.x;
			selector.height = mouseMapPos.y - selector.y;
		} else {
			selector.width = 0;
			selector.height = 0;
		}

		if ((selector.width != 0 && selector.height != 0) || selecting)
			updateSelected(editData.structures, editData.selectedStructures,
				fixNegativeDims(selector));
	}


	if (IsKeyDown(editData.controls[create]))
		creating = true;
	else
		creating = false;
	if (IsKeyPressed(editData.controls[_delete])) {

	}
	if (IsKeyPressed(editData.controls[copy])) {

	}
	if (IsKeyPressed(editData.controls[save])) {
		saveMap(editData.structures, "resources/map.txt");
	}
	if (IsKeyPressed(editData.controls[load])) {
		loadMap(editData.structures, "resources/map.txt");
	}
}

void editor::draw(EditData& editData, TimeSeconds delta, Rectangle player) {
	bool& creating = editData.creating;
	Rectangle& newStructure = editData.newStructure;
	Rectangle& selector = editData.selector;
	Rectangle*& selectedStructure = editData.selectedStructure;

	BeginMode2D(editData.camera);
	DrawRectangleRec(player, WHITE);

	for (auto& structure : editData.structures)
		DrawRectangleRec(structure, BROWN);

	if (!creating && newStructure.width != 0) {
		editData.structures.push_back(fixNegativeDims(newStructure));
	}

	const static Color transparentWhite = {255, 255, 255, 40};

	DrawRectangleRec(fixNegativeDims(newStructure), transparentWhite);
	DrawRectangleRec(fixNegativeDims(selector), transparentWhite);

	for (auto& rec : editData.selectedStructures) {
		DrawRectangleLinesEx(*rec, 4, WHITE);
	}

	EndMode2D();

	DrawText(constData.controlsDisplay,30,30,20,WHITE);	
}
