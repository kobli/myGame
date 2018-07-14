#include "CGUIProgressBar.hpp"

irr::gui::CGUIProgressBar::CGUIProgressBar(IGUIEnvironment* guiEnv, const irr::core::rect<s32>& rectangle, IGUIElement* parent, s32 id):
	irr::gui::IGUIElement(EGUIET_ELEMENT, guiEnv, parent, id, rectangle), ProgressBar({rectangle.getWidth(), rectangle.getHeight()}),
	_vdriver{guiEnv->getVideoDriver()}
{
	_label = guiEnv->addStaticText(L"", core::rect<s32>(0, 0, getSize().X, getSize().Y), false, false, this);
	_value = guiEnv->addStaticText(L"", core::rect<s32>(0, 0, getSize().X, getSize().Y), false, false, this);

	_label->setRelativePosition(core::vector2di(5, (getSize().Y-_label->getTextHeight())/2));
	_value->setRelativePosition((getSize()-core::vector2di(_value->getTextWidth(), _value->getTextHeight()))/2);

	using irr::gui::EGUI_ALIGNMENT;
	_label->setAlignment(EGUIA_UPPERLEFT, EGUIA_SCALE, EGUIA_UPPERLEFT, EGUIA_LOWERRIGHT);
	_value->setAlignment(EGUIA_SCALE, EGUIA_LOWERRIGHT, EGUIA_UPPERLEFT, EGUIA_LOWERRIGHT);
}

irr::gui::CGUIProgressBar::~CGUIProgressBar()
{
	_label->drop();
	_value->drop();
}

void irr::gui::CGUIProgressBar::setLabel(const wchar_t* label)
{
	_label->setText(label);
}

void irr::gui::CGUIProgressBar::draw()
{
	if(!this->IsVisible)
		return;
	
	_value->setText(getValueString().c_str());

	irr::core::rect<s32> bar(this->getAbsolutePosition().UpperLeftCorner, this->getAbsolutePosition().UpperLeftCorner+getSize());
	irr::core::rect<s32> progress(bar.UpperLeftCorner, bar.UpperLeftCorner+core::vector2di(bar.getWidth()*getProgress(),bar.getHeight()));
		
	_vdriver->draw2DRectangle(getBackgroundColor(),bar);
	_vdriver->draw2DRectangle(getProgressColor(),progress);

	for(auto& c: this->getChildren())
		c->draw();
}

