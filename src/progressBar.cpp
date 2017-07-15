#include "progressBar.hpp"

irr::gui::ProgressBar::ProgressBar(IGUIEnvironment* guiEnv, const irr::core::rect<s32>& rectangle, IGUIElement* parent, s32 id):
	irr::gui::IGUIElement(EGUIET_ELEMENT, guiEnv, parent, id, rectangle), _bar{rectangle}, _vdriver{guiEnv->getVideoDriver()}
{
	setColors();
}

void irr::gui::ProgressBar::setProgress(float progress)
{
	_progress = progress;
}

void irr::gui::ProgressBar::setColors(irr::video::SColor background, irr::video::SColor progress)
{
	_backgroundColor = background;
	_progressColor = progress;
}

void irr::gui::ProgressBar::draw()
{
	if(!this->IsVisible)
		return;

	_vdriver->draw2DRectangle(_backgroundColor,_bar);
	irr::core::rect<s32> progress(_bar.UpperLeftCorner, _bar.UpperLeftCorner+core::vector2di(_bar.getWidth()*_progress,_bar.getHeight()));
	_vdriver->draw2DRectangle(_progressColor,progress);
}
