#ifndef PROGRESSBAR_HPP_17_07_15_13_34_08
#define PROGRESSBAR_HPP_17_07_15_13_34_08 
#include <irrlicht.h>
namespace irr {
	namespace gui {

		class ProgressBar: public irr::gui::IGUIElement
		{
			public:
				ProgressBar(IGUIEnvironment* guienv, const irr::core::rect<s32>& rectangle, IGUIElement* parent=0, s32 id=-1);

				// progess must be in range 0. - 1. (otherwise clipped)
				void setProgress(float progress);
				float getProgress();

				void setColors(irr::video::SColor background=irr::video::SColor(255,255,255,255), irr::video::SColor progress=irr::video::SColor(255,0,0,0));

				virtual void draw();

			private:
				irr::core::rect<s32> _bar;
				float _progress;

				irr::video::SColor _backgroundColor;
				irr::video::SColor _progressColor;
				irr::video::IVideoDriver* _vdriver;
		};
	}
}

#endif /* PROGRESSBAR_HPP_17_07_15_13_34_08 */
