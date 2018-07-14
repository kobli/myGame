#ifndef CGUIPROGRESSBAR_HPP_18_05_01_12_53_00
#define CGUIPROGRESSBAR_HPP_18_05_01_12_53_00 
#include "progressBar.hpp"

namespace irr {
	namespace gui {

		class CGUIProgressBar: public irr::gui::IGUIElement, public ProgressBar
		{
			public:
				CGUIProgressBar(IGUIEnvironment* guienv, const irr::core::rect<s32>& rectangle, IGUIElement* parent=0, s32 id=-1);
				~CGUIProgressBar();

				virtual void setLabel(const wchar_t* label) override;
				virtual void draw();

			private:
				irr::video::IVideoDriver* _vdriver;
				IGUIStaticText* _label;
				IGUIStaticText* _value;
		};
	}
}

#endif /* CGUIPROGRESSBAR_HPP_18_05_01_12_53_00 */
