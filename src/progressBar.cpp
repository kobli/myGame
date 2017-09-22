#include "progressBar.hpp"
#include <algorithm>
#include <cassert>
#include <string>

irr::gui::ProgressBar::ProgressBar(IGUIEnvironment* guiEnv, const irr::core::rect<s32>& rectangle, IGUIElement* parent, s32 id):
	irr::gui::IGUIElement(EGUIET_ELEMENT, guiEnv, parent, id, rectangle), _bar{rectangle},
	_maxVal{0}, _valDisplayMode{ValueDisplayMode::None}, _vdriver{guiEnv->getVideoDriver()}
{
	setColors();
	_label = guiEnv->addStaticText(L"", core::rect<s32>(0, 0, _bar.getSize().Width, _bar.getSize().Height), false, false, this);
	_value = guiEnv->addStaticText(L"", core::rect<s32>(0, 0, _bar.getSize().Width, _bar.getSize().Height), false, false, this);

	core::vector2di barSize(_bar.getWidth(), _bar.getHeight());
	_label->setRelativePosition(core::vector2di(5, (_bar.getSize().Height-_label->getTextHeight())/2));
	_value->setRelativePosition((barSize-core::vector2di(_value->getTextWidth(), _value->getTextHeight()))/2);

	using irr::gui::EGUI_ALIGNMENT;
	_label->setAlignment(EGUIA_UPPERLEFT, EGUIA_SCALE, EGUIA_UPPERLEFT, EGUIA_LOWERRIGHT);
	_value->setAlignment(EGUIA_SCALE, EGUIA_LOWERRIGHT, EGUIA_UPPERLEFT, EGUIA_LOWERRIGHT);
}

irr::gui::ProgressBar::~ProgressBar()
{
	_label->drop();
	_value->drop();
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

void irr::gui::ProgressBar::setLabel(const wchar_t* label)
{
	_label->setText(label);
}

void irr::gui::ProgressBar::setMaxValue(int maxVal)
{
	_maxVal = maxVal;
}

void irr::gui::ProgressBar::setValueDisplayMode(ValueDisplayMode m)
{
	_valDisplayMode = m;
}

void irr::gui::ProgressBar::draw()
{
	if(!this->IsVisible)
		return;
	
	std::wstring w;
	if(_valDisplayMode == None)
		w = L"";
	else if(_valDisplayMode == Perc)
		w = std::to_wstring(int(_progress*100)) + L" %";
	else if(_valDisplayMode == Abs)
		w = std::to_wstring(int(_progress*_maxVal)) + L" / " + std::to_wstring(int(_maxVal));
	else
		assert(false);
	_value->setText(w.c_str());

	irr::core::rect<s32> bar(this->getAbsolutePosition().UpperLeftCorner, this->getAbsolutePosition().UpperLeftCorner+core::vector2di(_bar.getWidth(),_bar.getHeight()));
	irr::core::rect<s32> progress(bar.UpperLeftCorner, bar.UpperLeftCorner+core::vector2di(bar.getWidth()*_progress,bar.getHeight()));
		
	_vdriver->draw2DRectangle(_backgroundColor,bar);
	_vdriver->draw2DRectangle(_progressColor,progress);

	for(auto& c: this->getChildren())
		c->draw();
}
