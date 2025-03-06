#include "levelEditor.hpp"

#define dbg(...) printf(__VA_ARGS__)
#define dbg(...) // debug logging is turned off 

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
	float x1 = (inner.x - bounds.x) * newBounds.width / bounds.width,
		y1 = (inner.y - bounds.y) * newBounds.height / bounds.height,
		x2 = (inner.x + inner.width - bounds.x) * newBounds.width / bounds.width,
		y2 = (inner.y + inner.height - bounds.y) * newBounds.height / bounds.height;

	Rectangle out {
		x1 + newBounds.x,
		y1 + newBounds.y,
		x2 - x1,
		y2 - y1
	};

	dbg(
		"resizeInnerRec: input = { %f, %f, %f, %f }\n"
		"resizeInnerRec: output = { %f, %f, %f, %f }\n",
		inner.x, inner.y, inner.width, inner.height,
		out.x, out.y, out.width, out.height);

	return out;
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
		cl = p::center_left, c = p::center, cr = p::center_right, 
		ll = p::lower_left, lc = p::lower_center, lr = p::lower_right;

	Vector2 out{0,0};
	if (pos == ul || pos == uc || pos == ur) {
		out.y = rec.y;
		if (pos == ul)	out.x = rec.x; else
		if (pos == uc)	out.x = rec.x + rec.width / 2; else
		if (pos == ur)	out.x = rec.x + rec.width;
	} else if (pos == cl || pos == cr || pos == c) {
		out.y = rec.y + rec.height / 2;
		if (pos == cl)	out.x = rec.x; else
		if (pos == c)	out.x = rec.x + rec.width / 2; else
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

// returns the position of the resize button that is being pressed, -1 if none
Position resizeButtonPressed(Vector2 pos, Rectangle rec, float resizeButtonSize) {
	float x = rec.x, y = rec.y, width = rec.width, height = rec.height;
	using p = Position;
	std::map<Position,Vector2> buttonAt {
		{p::upper_left, {x,y}},							{p::upper_center, {x + width/2, y}},
		{p::upper_right, {x + width, y}},				{p::center_left, {x, y + height/2}},
		{p::center_right, {x + width, y + height/2}},	{p::center, {x + width/2, y + height/2}},
		{p::lower_left, {x, y + height}},				{p::lower_center, {x + width/2, y + height}},
		{p::lower_right, {x + width, y + height}},
	};
	
	for (int i=0; i < (int)Position::none; ++i) {
		if (CheckCollisionPointCircle(pos, buttonAt[(p)i], resizeButtonSize)) {
			return (Position)i;
		}
	}

	return Position::none;
}

// selects the rectangles that collide with "selector"
void updateSelected(std::vector<Rectangle*>& selected,
	std::vector<Rectangle>& structures, Rectangle selector, bool append) {
	if (selected.capacity() < structures.size())
		selected.reserve(structures.size());

	if (!append) {
		selected.resize(0);
#ifdef DEBUG
		std::cout << "updateSelected: resizing selected to 0\n";
#endif
	}
#ifdef DEBUG
	std::cout << "updateSelected: checking for collisions in " << structures.size()
		<< " structures\n";
#endif
	if (!append) for (auto& rec : structures) {
		if (CheckCollisionRecs(selector, rec)) {
#ifdef DEBUG			
			printf("updateSelected: found rectangle { %f, %f, %f, %f }\n",
				rec.x, rec.y, rec.width, rec.height
			);
#endif
			selected.push_back(&rec);
		}
	} else for (auto& rec : structures) {
#ifdef DEBUG
		std::cout << "updateSelected: verifying copies\n";
#endif
		if (CheckCollisionRecs(selector, rec)) {
			for (auto address : selected)
				if (&rec == address) goto next;
			selected.push_back(&rec);
		}
		next:
	}
#ifdef DEBUG
	std::cout << "updateSelected: currently selected " << selected.size()
		<< " structures\n\n";
#endif
}

// resizes a rectangle by displacing the resize node by delta.
// TODO: implement keepAspectRatio
Rectangle resize(Rectangle rec, Vector2 delta, Position position) {
	dbg("resize(rec, delta, position);\n");

	switch (position) {
	case Position::center:
		rec.x += delta.x;
		rec.y += delta.y;
		break;
	case Position::lower_right:
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
		rec.width -= delta.x;
		rec.height -= delta.y;
		break;
	case Position::upper_center:
		rec.y += delta.y;
		rec.height -= delta.y;
		break;
	case Position::upper_right:
		rec.y += delta.y;
		rec.width += delta.x;
		rec.height -= delta.y;
		break;
	case Position::center_left:
		rec.x += delta.x;
		rec.width -= delta.x;
		break;
	case Position::lower_left:
		rec.x += delta.x;
		rec.width -= delta.x;
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
	std::cout << "editor::loadMap\n\n";
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
	std::cout << "editor::saveMap\n\n";
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
	Vector2 mouseDelta = GetMouseDelta() / camera.zoom;

	if (!creating) {
		newStructure.x = roundf(mouseMapPos.x);
		newStructure.y = roundf(mouseMapPos.y);
	}
	newStructure.width = roundf(mouseMapPos.x - newStructure.x);
	newStructure.height = roundf(mouseMapPos.y - newStructure.y);

	if (IsMouseButtonDown(editData.controls[scroll])) {
		editData.camera.target -= GetMouseDelta() / camera.zoom;
	}
	
	// resize only if not already selecting
	{
		if (editData.selector.width * editData.selector.height == 0)
		if (editData.resizePos != Position::none) {
			Rectangle newSize = 
				resize(editData.resizeBox, mouseDelta, editData.resizePos);
			for (auto& rec : editData.selectedStructures) {
				*rec = resizeInnerRec(*rec, editData.resizeBox, newSize);
			}
			editData.resizeBox = newSize;
		}

		if (IsMouseButtonPressed(editData.controls[select])) {
			editData.resizePos =
				resizeButtonPressed(mouseMapPos, editData.resizeBox,
				editor::resizeButtonSize);
			dbg("editor::process: resizePos = %i\n", (int)editData.resizePos);
		} else if (IsMouseButtonReleased(editData.controls[select])) {
			editData.resizePos = Position::none;
		}
	}

	editData.creating = [&editData, &newStructure, mouseMapPos]{
		bool out = false;
		if (IsKeyPressed(editData.controls[create])) {
			newStructure.x = mouseMapPos.x;
			newStructure.y = mouseMapPos.y;
			out = true;
		} else if (IsKeyDown(editData.controls[create])) {
			newStructure.width = mouseMapPos.x - newStructure.x;
			newStructure.height = mouseMapPos.y - newStructure.y;
			out = true;
		} else if (IsKeyReleased(editData.controls[create])) {
			out = false;
			editData.structures.push_back(fixNegativeDims(newStructure));
		} else {
			out = false;
			newStructure.width = 0;
		}

		return out;
	}();
	
	// selector logic
	if (editData.resizePos == Position::none)
	editData.resizeBox = [&]{
		bool selectMultiple = IsKeyDown(editData.controls[select_multiple]);
		if (IsMouseButtonPressed(editData.controls[select])) {
			if (!selectMultiple) editData.selectedStructures.resize(0);

			selector.x = mouseMapPos.x;
			selector.y = mouseMapPos.y;
			
			auto recAtMouse = getRectangleAt(mouseMapPos,editData.structures);
			if (recAtMouse) editData.selectedStructures.push_back(recAtMouse);
		} else if (IsMouseButtonDown(editData.controls[select])) {
			selector.width = mouseMapPos.x - selector.x;
			selector.height = mouseMapPos.y - selector.y;
		} else { selector.width = 0; }

		if (selector.width * selector.height != 0) {
			updateSelected(editData.selectedStructures, editData.structures,
				fixNegativeDims(selector), selectMultiple);
		}

		if (editData.selectedStructures.size() > 0)
			return getOuterRectangle(editData.selectedStructures);

		return Rectangle{0.0f, 0.0f, 0.0f, 0.0f};
	}();


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

	const static Color transparentWhite = {255, 255, 255, 40};

	DrawRectangleRec(fixNegativeDims(newStructure), {127, 106, 79, 100});
	DrawRectangleRec(fixNegativeDims(selector), transparentWhite);

	for (auto& rec : editData.selectedStructures) {
		DrawRectangleLinesEx(*rec, 4, WHITE);
	}
	drawResizeBox(editData.resizeBox, 3, editor::resizeButtonSize, {255,255,255,100});

	EndMode2D();

	DrawText(constData.controlsDisplay,30,30,20,WHITE);	
}
