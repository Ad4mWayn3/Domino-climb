#define RAYGUI_IMPLEMENTATION
#include "gui.hpp"

const Vector2 defaultButtonSize = {160, 60};
Vector2 center() { return resolutionV / 2; }

Rectangle rectangleCenter(float x, float y, float width, float height) {
	return {x - width/2, y - height/2, width, height};
}

using cstrlist = std::vector<const char*>;
std::vector<Button> gui::buttonGrid(const cstrlist& buttonNames,
	Vector2 origin, Vector2 buttonSize, Vector2 spacing, size_t wrapSize,
	Axis wrapAxis) {
	std::vector<Button> out(buttonNames.size());

	Vector2 jump = buttonSize + spacing;
	int a=0, b=0;
	int* x = &a, * y = &b;
	if (wrapAxis == vertical) std::swap(x,y);
	for (int i=0; i < buttonNames.size(); ++i) {
		a = i % wrapSize, b = i / wrapSize;
		Rectangle bounds {
			origin.x + jump.x* (*x),
			origin.y + jump.y* (*y),
			buttonSize.x, buttonSize.y };
		out[i] = {bounds, buttonNames[i]};
	}

	return out;
}

void gui::init() {
	GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
	GuiSetStyle(DEFAULT, TEXT_WRAP_MODE, TEXT_WRAP_WORD);
	GuiSetStyle(DEFAULT, TEXT_SIZE, 18);

	Vector2 buttonSize{200, 60};
	Vector2 screenCenter = resolutionV / 2;
	Vector2 origin{screenCenter.x - buttonSize.x/2, 80};

	pauseButtons = buttonGrid({"resume", "toggle gamemode", "controls",
		"save map", "load map", "exit game"}, origin, buttonSize,
		{30,30}, 6, vertical);

	std::vector<const char*> gameControlsNames(controls_count);
	std::vector<const char*> editControlsNames(edit_count);
	for (int i=0; i < controls_count; ++i)
		gameControlsNames[i] = gameControls[(Controls)i].name;
	for (int i=0; i < edit_count; ++i)
		editControlsNames[i] = editControls[(EditControls)i].name;

	gameControlsButtons = buttonGrid(gameControlsNames, {1400,40}, buttonSize,
		{20,20}, controls_count, vertical);
	editControlsButtons = buttonGrid(editControlsNames, {300, 40}, buttonSize,
		{20,20}, controls_count, vertical);
}

Menu gui::pauseMenu(int& gameMode, bool& pause, GameData& gameData,
	EditData& editData) {
	using pb = PauseButton;
	Button
		& resume			= pauseButtons[(int)pb::resume],
		& toggleGamemode	= pauseButtons[(int)pb::toggle_gamemode],
		& controls			= pauseButtons[(int)pb::controls],
		& saveMap			= pauseButtons[(int)pb::save_map],
		& loadMap			= pauseButtons[(int)pb::load_map],
		& exitGame			= pauseButtons[(int)pb::exit_game];
	
	if (GuiButton(resume.bounds, resume.text)) {
		pause = false;
		return Menu::menu_pause;
	}
	if (GuiButton(toggleGamemode.bounds, toggleGamemode.text)) {
		gameMode = (gameMode + 1) % modeCount;
	}
	if (GuiButton(controls.bounds, controls.text)) {
		return Menu::menu_controls;
	}
	if (GuiButton(saveMap.bounds, saveMap.text)) {
		editor::saveMap(editData.structures, "resources/map.txt");
	}
	if (GuiButton(loadMap.bounds, loadMap.text)) {
		editor::loadMap(editData.structures, "resources/map.txt");
	}
	if (GuiButton(exitGame.bounds, exitGame.text)) {
		// should there be an exitGame() function?
		gameMode = (int)Modes::exit;
	}

	return Menu::menu_pause;
}

Menu gui::controlsMenu() {
	static Rectangle centerBounds = rectangleCenter(center().x, center().y, 1400, 600);
	static bool gameControlIsSelected = false,
		editControlIsSelected = false; 
	static Controls selectedControl; // probably initalizes to zero which is \
		the "up" control. Semantically this is nonsense, but no side effect is \
		expected since "selected" is set to false.
	static EditControls selectedEditControl; // zero is the "scroll" control \
		in editor mode. Nonsense again, but same thing as before.

	for (size_t i=0; i < gameControlsButtons.size(); ++i) {
		Button& controlBtn = gameControlsButtons[(Controls)i];
		if (GuiButton(controlBtn.bounds, controlBtn.text)) {
			gameControlIsSelected = true;
			selectedControl = (Controls)i;
			// the loop could probably break here since there's no need to \
				check the remaining buttons, given that a single button can be \
				pressed per frame, but that would leave the rest of the buttons \
				not rendered.
		}
	}

	if (gameControlIsSelected) {
		std::stringstream ss;
		ss << "Bind key " << (char)GetKeyPressed() << " to " \
			<< gameControls[selectedControl].name;
		GuiLabel(centerBounds, ss.str().c_str());

		int keyPress = GetKeyPressed();
		if (keyPress != KEY_NULL)
			gameControls[selectedControl].key = keyPress;

		static Vector2 bottomCenter {center().x, center().y + resolutionV.y / 4};
		static Rectangle bottomCenterBounds =
			{bottomCenter.x, bottomCenter.y,
			defaultButtonSize.x, defaultButtonSize.y};
		if (GuiButton(bottomCenterBounds, "done")) {
			gameControlIsSelected = false;
		}
	}
	
	for (auto& button : editControlsButtons)
		GuiButton(button.bounds, button.text);

	return Menu::menu_controls;
}
