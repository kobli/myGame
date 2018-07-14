#ifndef CROSSHAIR_HPP_17_09_14_16_33_58
#define CROSSHAIR_HPP_17_09_14_16_33_58 

#include <irrlicht.h>
namespace irr {
	namespace gui {

		class CrossHair: public irr::gui::IGUIElement
		{
			public:
				CrossHair(IGUIEnvironment* guienv, const std::string& imagePath, int size = 5, IGUIElement* parent=0, s32 id=-1):
					irr::gui::IGUIElement(EGUIET_ELEMENT, guienv, parent, id, irr::core::rect<irr::s32>()), _env{guienv}, _size{size}
				{
					_texture = guienv->getVideoDriver()->getTexture(imagePath.c_str());
				}

				virtual void draw() {
					irr::video::IVideoDriver* driver = _env->getVideoDriver();
					vec2i mid = vec2i(driver->getScreenSize().Width, driver->getScreenSize().Height)/2;
 
					driver->draw2DImage(_texture, irr::core::rect<irr::s32>(mid.X-_size, mid.Y-_size, mid.X+_size, mid.Y+_size), irr::core::rect<irr::s32>(0, 0, _texture->getOriginalSize().Width, _texture->getOriginalSize().Height), 0, 0, true);
				}

				~CrossHair()
				{
					_texture->drop();
				}

			private:
				irr::gui::IGUIEnvironment* _env;
				irr::video::ITexture* _texture;
				int _size;
		};
	}
}
#endif /* CROSSHAIR_HPP_17_09_14_16_33_58 */
