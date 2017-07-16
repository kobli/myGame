#include "progressBar.hpp"
#include <algorithm>

irr::gui::ProgressBar::ProgressBar(IGUIEnvironment* guiEnv, const irr::core::rect<s32>& rectangle, IGUIElement* parent, s32 id):
	irr::gui::IGUIElement(EGUIET_ELEMENT, guiEnv, parent, id, rectangle), _bar{rectangle}, _vdriver{guiEnv->getVideoDriver()}
{
	setColors();
}

void irr::gui::ProgressBar::setProgress(float progress)
{
	_progress = std::max(0.f, std::min(1.f, progress));
}

float irr::gui::ProgressBar::getProgress()
{
	return _progress;
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

	irr::core::rect<s32> bar(this->getAbsolutePosition().UpperLeftCorner, this->getAbsolutePosition().UpperLeftCorner+core::vector2di(_bar.getWidth(),_bar.getHeight()));
	irr::core::rect<s32> progress(bar.UpperLeftCorner, bar.UpperLeftCorner+core::vector2di(bar.getWidth()*_progress,bar.getHeight()));
		
	_vdriver->draw2DRectangle(_backgroundColor,bar);
	_vdriver->draw2DRectangle(_progressColor,progress);
}
