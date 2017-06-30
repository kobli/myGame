#include <controller.hpp>

Command::Command(Type type): _type{type}
{}

////////////////////////////////////////////////////////////

Controller::Controller(): _commandHandler{[](Command&){}}, _lastMovD{0,0}
{
	for(u32 i=0; i<KEY_KEY_CODES_COUNT; ++i)
		_keyPressed[i] = false;
	_LMBdown = false;
}

bool Controller::OnEvent(const SEvent& event)
{
	if(event.EventType == irr::EET_KEY_INPUT_EVENT)
		_keyPressed[event.KeyInput.Key] = event.KeyInput.PressedDown;
	if (event.EventType == irr::EET_MOUSE_INPUT_EVENT)
	{
		switch(event.MouseInput.Event)
		{
			case EMIE_LMOUSE_PRESSED_DOWN:
				{
					Command c(Command::Type::STR);
					c._str = "spell_body_create 1 2 3 hello";
					//_commandHandler(c);
					_LMBdown = true;
					break;
				}
			case EMIE_LMOUSE_LEFT_UP:
				_LMBdown = false;
				break;
			case EMIE_MOUSE_MOVED:
				{
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
	if(event.EventType == irr::EET_KEY_INPUT_EVENT)
	{
		std::string spell_c = "";
		if(event.KeyInput.PressedDown)
			switch (event.KeyInput.Key)
			{
				case KEY_F1:
					spell_c = "spell_body_create 1";
					break;
				case KEY_F2:
					spell_c = "spell_body_create_now 0.9 die{player}";
					break;
				case KEY_F3:
					spell_c = "spell_launch_direct_now $LOOK_ELEVATION";
					break;
				case KEY_F4:
					spell_c = "spell_launch_direct_now 100";
					break;
				case KEY_F5:
					spell_c = "spell_effect_create fire";
					break;
				default:
					break;
			}
		switch (event.KeyInput.Key)
		{
			case KEY_KEY_W: 
			case KEY_KEY_S: 
			case KEY_KEY_A: 
			case KEY_KEY_D: 
				{
					Command c(Command::Type::STRAFE_DIR_SET);
					vec2f& movD = c._vec2f = vec2f{0,0};

					if(_keyPressed[KEY_KEY_W])
						movD.X = 1;
					if(_keyPressed[KEY_KEY_S])
						movD.X = -1;
					if(_keyPressed[KEY_KEY_A])
						movD.Y = 1;
					if(_keyPressed[KEY_KEY_D])
						movD.Y = -1;
					if(movD != _lastMovD)
					{
						_lastMovD = movD;
						_commandHandler(c);
					}
					return true;
				}
			case KEY_KEY_Q: 
			case KEY_KEY_E: 
				{
					Command c(Command::Type::ROT_DIR_SET);
					i32& rotD = c._i32= 0;

					if(_keyPressed[KEY_KEY_Q])
						rotD = -1;
					if(_keyPressed[KEY_KEY_E])
						rotD = 1;
					_commandHandler(c);
					return true;
				}
				/*
					 case irr::KEY_KEY_X: // toggle debug information
					 showDebug=!showDebug;
					 Terrain->setDebugDataVisible(showDebug?scene::EDS_BBOX_ALL:scene::EDS_OFF);
					 return true;
					 */
			default:
				break;
		}
		if(spell_c != "")
		{
			Command c(Command::Type::STR);
			c._str = spell_c;
			_commandHandler(c);
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
