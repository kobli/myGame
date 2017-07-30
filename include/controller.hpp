#ifndef CONTROLLER_HPP_16_11_18_13_06_55
#define CONTROLLER_HPP_16_11_18_13_06_55 
#include <functional>
#include <vector>
#include <map>
#include <set>
#include "main.hpp"

class Command
{
	public:
		enum Type: u16 {
			Null,							// empty
			STRAFE_DIR_SET,		// vec2f: x+ = forward, x- = backward, y+ = left, y- = right
			ROT_DIR_SET,			// i32: -1 = left, 0 = stop, 1 = right
			ROT_diff,         // vec2f: rotation difference in radians
			Y_ANGLE_SET,      // float: Y rotation angle in radians
			STR,							// command and data in one string
		};

		Command(Type type = Type::Null);

		Type _type;
		union {
			vec2f _vec2f;
			vec3f _vec3f;
			float _float;
			i32 _i32;
			u32 _u32;
			i64 _i64;
		};
		std::string _str;
};

////////////////////////////////////////////////////////////

class Controller: public IEventReceiver
{
	public:
		typedef std::function<vec2i()> GetScreenSize;
		typedef std::function<void()> Exit;
		Controller();

		virtual bool OnEvent(const SEvent& event);
		void setCommandHandler(std::function<void(Command& c)> commandHandler);
		void setScreenSizeGetter(GetScreenSize screenSizeGetter);
		void setExit(Exit exit);
		void loadSpellBook(std::string fileName);
		void loadControls(std::string fileName);

	private:
		typedef std::map<std::string, std::vector<std::string>> SpellBook;
		typedef std::map<irr::EKEY_CODE, std::string> KeyMap;
		bool _keyPressed[KEY_KEY_CODES_COUNT];
		bool _LMBdown;
		std::function<void(Command& c)> _commandHandler;
		std::set<std::string> _activeActions;
		vec2f _lastSentMovD;
		GetScreenSize _getScreenSize;
		Exit _exit;
		SpellBook _spellBook;
		KeyMap _keyMap;
};
#endif /* CONTROLLER_HPP_16_11_18_13_06_55 */
