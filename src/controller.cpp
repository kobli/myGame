#include <memory>
#include "controller.hpp"

Command::Command(Type type): _type{type}
{}

////////////////////////////////////////////////////////////

Controller::Controller(): _commandHandler{[](Command&){}}, _lastSentMovD{0,0}, _freeCamera{false}
{
	loadSpellBook("spellBook.lua");
	loadControls("controls.lua");
	for(u32 i=0; i<KEY_KEY_CODES_COUNT; ++i)
		_keyPressed[i] = false;
	_LMBdown = false;
}

void Controller::loadSpellBook(std::string fileName) {
	std::cout << "loading spellBook from " << fileName << " ...\n";
	std::unique_ptr<lua_State,std::function<void(lua_State*)>> l(luaL_newstate(), [](lua_State* L) { lua_close(L); });
	lua_State* L = l.get();
	if(luaL_loadfile(L, fileName.c_str()) || lua_pcall(L, 0, 0, 0))
		std::cerr << "cannot run " << fileName << " configuration file: " << lua_tostring(L, -1);

	lua_getglobal(L, "spellBook");
	lua_pushnil(L);
	while(lua_next(L, -2) != 0)
	{
		std::string spell = lua_valueAsStr(L, -2);
		Table t = lua_loadTable(L);
		std::vector<std::string> steps;
		for(auto& s: t)
			steps.push_back(s.second);
		_spellBook[spell] = steps;
		lua_pop(L, 1);
	}
}

void Controller::loadControls(std::string fileName) {
	std::cout << "loading controls from " << fileName << " ...\n";
	std::unique_ptr<lua_State,std::function<void(lua_State*)>> l(luaL_newstate(), [](lua_State* L) { lua_close(L); });
	lua_State* L = l.get();
	luaL_openlibs(L);
	if(luaL_loadfile(L, fileName.c_str()) || lua_pcall(L, 0, 0, 0))
		std::cerr << "cannot run " << fileName << " configuration file: " << lua_tostring(L, -1);

	lua_getglobal(L, "controls");
	Table t = lua_loadTable(L);
	for(auto& r: t) {
		size_t end;
		try {
			unsigned long keyCode = std::stoul(r.first, &end);
			_keyMap[static_cast<irr::EKEY_CODE>(keyCode)] = r.second;
		}
		catch(invalid_argument&) {
			std::cerr << "error in control settings: \"" << r.first << " = " << r.second << "\"\n";
		}
	}
}

bool Controller::OnEvent(const SEvent& event)
{
	std::string c;

	irr::EKEY_CODE key = irr::EKEY_CODE::KEY_KEY_CODES_COUNT;
	bool pressedDown = false;

	if(event.EventType == irr::EET_MOUSE_INPUT_EVENT) {
		if(event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN) {
			key = irr::EKEY_CODE::KEY_LBUTTON;
			pressedDown = true;
		}
		else if(event.MouseInput.Event == EMIE_LMOUSE_LEFT_UP) {
			key = irr::EKEY_CODE::KEY_LBUTTON;
			pressedDown = false;
		}
		else if(event.MouseInput.Event == EMIE_RMOUSE_PRESSED_DOWN) {
			key = irr::EKEY_CODE::KEY_RBUTTON; 
			pressedDown = true;
		}
		else if(event.MouseInput.Event == EMIE_RMOUSE_LEFT_UP) {
			key = irr::EKEY_CODE::KEY_RBUTTON; 
			pressedDown = false;
		}
	}
	else if(event.EventType == irr::EET_KEY_INPUT_EVENT) {
		pressedDown = event.KeyInput.PressedDown;
		key = event.KeyInput.Key;
	}
	
	if(key != irr::EKEY_CODE::KEY_KEY_CODES_COUNT) {
		if(_keyPressed[key] != pressedDown)
			if(_keyMap.count(key)) {
				c = _keyMap[key];
				if(pressedDown)
					_activeActions.insert(c);
				else
					_activeActions.erase(c);
			}
		_keyPressed[key] = pressedDown;
	}

	if(c == "FORWARD" || c == "BACKWARD" || c == "LEFT" || c == "RIGHT") {
		vec2f movD;
		if(_activeActions.count("FORWARD"))
			movD.X += 1;
		if(_activeActions.count("BACKWARD"))
			movD.X -= 1;
		if(_activeActions.count("LEFT"))
			movD.Y += 1;
		if(_activeActions.count("RIGHT"))
			movD.Y -= 1;

		if(movD != _lastSentMovD)
		{
			Command command(Command::Type::STRAFE_DIR_SET);
			command._vec2f = movD;
			_lastSentMovD = movD;
			_commandHandler(command);
		}
	}
	else if(c == "LAUNCH_SPELL_DIRECT" && pressedDown) {
		Command command(Command::Type::STR);
		command._str = "spell_launch_direct_now $LOOK_ELEVATION";
		_commandHandler(command);
	}
	else if(c.find("CAST ") == 0 && pressedDown) {
		Command command(Command::Type::STR);
		std::string spellName = c.substr(5);
		auto spell = _spellBook.end();
		if((spell = _spellBook.find(spellName)) != _spellBook.end()) {
			for(auto& step : spell->second) {
				command._str = step;
				_commandHandler(command);
			}
		}
	}

	if (event.EventType == irr::EET_MOUSE_INPUT_EVENT)
	{
		switch(event.MouseInput.Event)
		{
			case EMIE_MOUSE_MOVED:
				{
					if(_freeCamera)
						break;
					vec2i screenCenter = _getScreenSize()/2;
					vec2i mousePos = vec2i{event.MouseInput.X, event.MouseInput.Y};
					if(mousePos == screenCenter) // TODO better idea?
						return true;
					Command c(Command::Type::ROT_diff);
					float sensitivity = 0.001;
					vec2i mouseMovDiff = mousePos-screenCenter;
					c._vec2f = vec2f(mouseMovDiff.X, mouseMovDiff.Y)*sensitivity;
					_commandHandler(c);
					return true;
					break;
				}
			default:
				break;
		}
	}
	if (event.EventType == irr::EET_KEY_INPUT_EVENT)
	{
		switch(event.KeyInput.Key)
		{
			case irr::KEY_F4:
				{
					if(_keyPressed[EKEY_CODE::KEY_LMENU])
						_exit();
					break;
				}
			default:
				break;
		}
	}
	return false;
}

void Controller::setCommandHandler(std::function<void(Command& c)> commandHandler)
{
	_commandHandler = commandHandler;
}

void Controller::setScreenSizeGetter(GetScreenSize screenSizeGetter)
{
	_getScreenSize = screenSizeGetter;
}

void Controller::setExit(Exit exit)
{
	_exit = exit;
}

bool Controller::isCameraFree()
{
	return _freeCamera;
}
