#include "levelEditor.hpp"
#define DEBUG

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

// resizes a rectangle inside a resizeBox
Rectangle resizeInnerRec(Rectangle inner, Rectangle bounds, Rectangle newBounds) {
	// vectors inner1, inner2, outer1, outer2, outer1', outer2'
	Vector2
		i1{inner.x - bounds.x, inner.y - bounds.y}, i2{inner.width, inner.height},
		o1{bounds.x, bounds.y}, o2{bounds.width, bounds.height},
		oP1{newBounds.x, newBounds.y}, oP2{newBounds.width, newBounds.height};

	return {
		i1.x * oP1.x / o1.x,
		i1.y * oP1.y / o1.y,
		i2.x * oP2.x / o2.x,
		i2.y * oP2.y / o2.y,
	};
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

// gets smallest rectangle that fully contains all of "recs"
Rectangle getOuterRectangle(const std::vector<Rectangle*>& recs) {
	Vector2 origin{1e10f, 1e10f};
	Vector2 end = origin * -1;
	if (recs.size() == 0) return {0.0f, 0.0f, 0.0f, 0.0f};
	for (auto& rec : recs) {
		if (rec->x < origin.x) origin.x = rec->x;
		if (rec->y < origin.y) origin.y = rec->y;
		if (rec->width + rec->x > end.x) end.x = rec->width + rec->x;
		if (rec->height + rec->y > end.y) end.y = rec->height + rec->y;
	}
	return {origin.x, origin.y, end.x - origin.x, end.y - origin.y};
}

Vector2 getSelectorPoint(Rectangle rec, Position pos) {
	using p = Position;
	constexpr static auto 
		ul = p::upper_left, uc = p::upper_center, ur = p::upper_right,
		cl = p::center_left, cr = p::center_right, ll = p::lower_left,
		lc = p::lower_center, lr = p::lower_right;

	Vector2 out{0,0};
	if (pos == ul || pos == uc || pos == ur) {
		out.y = rec.y;
		if (pos == ul)	out.x = rec.x; else
		if (pos == uc)	out.x = rec.x + rec.width / 2; else
		if (pos == ur)	out.x = rec.x + rec.width;
	} else if (pos == cl || pos == cr) {
		out.y = rec.y + rec.height / 2;
		if (pos == cl)	out.x = rec.x; else
		if (pos == cr)	out.x = rec.x + rec.width;
	} else if (pos == ll || pos == lc || pos == lr) {
		out.y = rec.y + rec.height;
		if (pos == ll)	out.x = rec.x; else
		if (pos == lc)	out.x = rec.x + rec.width / 2; else
		if (pos == lr)	out.x = rec.x + rec.width;
	}

	return out;
}

void drawResizeBox(Rectangle rec, float lineThick, float circleRadius, Color color) {
	if (rec.width * rec.height == 0) return;
	DrawRectangleLinesEx(rec, lineThick, color);

	for (int i=0; i < 9; ++i) {
		Vector2 point = getSelectorPoint(rec, (Position)i);
		DrawCircle(point.x, point.y, circleRadius, color);
	}
}

// checks if a point (vector2) is inside a rectangle
bool inRectangle(Vector2 pos, Rectangle rec) {
	return pos.x >= rec.x && pos.x <= (rec.x + rec.width)
		&& pos.y >= rec.y && pos.y <= (rec.y + rec.height);
}

bool resizing(Vector2 pos, Rectangle rec, float resizeButtonSize) {

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

// i'm probably being too verbose in this definition, there has to be a simpler
// way to resize
Rectangle resize(Rectangle rec, Vector2 delta, Position position,
	bool keepAspectRatio) {
	switch (position) {
	case Position::lower_right:
		if (keepAspectRatio) {
			rec.width += std::min(delta.x, delta.y);
			rec.height += std::min(delta.x, delta.y);
			break;
		}
		rec.width += delta.x;
		rec.height += delta.y;
		break;
	case Position::lower_center:
		rec.height += delta.y;
		break;
	case Position::center_right:
		rec.width += delta.x;
		break;
	case Position::upper_left:
		rec.x += delta.x;
		rec.y += delta.y;
		rec.width += delta.x;
		rec.height += delta.y;
		break;
	case Position::upper_center:
		rec.y += delta.y;
		rec.height += delta.y;
		break;
	case Position::upper_right:
		rec.y += delta.y;
		rec.width += delta.x;
		rec.height += delta.y;
		break;
	case Position::center_left:
		rec.x += delta.x;
		rec.width += delta.x;
		break;
	case Position::lower_left:
		rec.x += delta.x;
		rec.width += delta.x;
		rec.height += delta.y;
	}
	return rec;
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
	Vector2 mouseMapPos = GetMousePosition() / camera.zoom + camera.target;

	if (!creating) {
		newStructure.x = roundf(mouseMapPos.x);
		newStructure.y = roundf(mouseMapPos.y);
	}
	newStructure.width = roundf(mouseMapPos.x - newStructure.x);
	newStructure.height = roundf(mouseMapPos.y - newStructure.y);

	if (IsMouseButtonDown(editData.controls[scroll])) {
		editData.camera.target -= GetMouseDelta() / camera.zoom;
	}


	// selector logic
	{
		bool selecting = false;
		if (IsMouseButtonPressed(editData.controls[select])) {
			selector.x = mouseMapPos.x;
			editData.selector.y = mouseMapPos.y;
			selecting = true;
			editData.selectedStructures.resize(0);
			auto rec = getRectangleAt(mouseMapPos, editData.structures);
			if (rec) editData.selectedStructures.push_back(rec);
		} else if (IsMouseButtonDown(editData.controls[select])) {
			selector.width = mouseMapPos.x - selector.x;
			selector.height = mouseMapPos.y - selector.y;
			selecting = true;
		} else {
			selector.width = 0;
			selector.height = 0;
		}
		if (!IsKeyDown(editData.controls[select_multiple]) && selecting) {
//			selector.width = 0;
			editData.selectedStructures.resize(0);
//				editData.resizeBox.x = 0;
//				editData.resizeBox.y = 0;
		}
		
		if ((selector.width != 0 && selector.height != 0) || selecting) {
			updateSelected(editData.structures, editData.selectedStructures,
				fixNegativeDims(selector));
			#ifdef DEBUG
			std::cout << "editor::process: resizeBox = { " << editData.resizeBox.x
				<< ", " << editData.resizeBox.y << ", " << editData.resizeBox.width
				<< "}\n\n";
			#endif
		}
		if (editData.selectedStructures.size() != 0) {
			editData.resizeBox = getOuterRectangle(editData.selectedStructures);
		} else {
			editData.resizeBox.width = 0;
		}
	}


	if (IsKeyDown(editData.controls[create]))
		creating = true;
	else
		creating = false;
	if (IsKeyPressed(editData.controls[_delete])) {
		std::cout << "editor::process: delete " << editData.selectedStructures.size()
			<< " structures\n";
		for (auto& rec : editData.selectedStructures) {
			int i = rec - &editData.structures[0];
			std::swap(*rec, editData.structures[editData.structures.size()-1]);
			editData.structures.pop_back();
		}
		editData.selectedStructures.resize(0);
		editData.resizeBox.width = 0;
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
	DrawCircle(0,0, 8, WHITE);
	DrawRectangleRec(player, WHITE);

	for (auto& structure : editData.structures)
		DrawRectangleRec(structure, BROWN);

	if (!creating && newStructure.width != 0) {
		editData.structures.push_back(fixNegativeDims(newStructure));
	}
	const static Color transparentWhite = {255, 255, 255, 40};

	DrawRectangleRec(fixNegativeDims(newStructure), {127, 106, 79, 100});
	DrawRectangleRec(fixNegativeDims(selector), transparentWhite);

	for (auto& rec : editData.selectedStructures) {
		DrawRectangleLinesEx(*rec, 4, WHITE);
	}
	drawResizeBox(editData.resizeBox, 3, 10, {255,255,255,100});

	EndMode2D();

	DrawText(constData.controlsDisplay,30,30,20,WHITE);	
}
