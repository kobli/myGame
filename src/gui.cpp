#include "gui.hpp"
#include "CGUITTFont.h"
#include "crosshair.hpp"

GUI::GUI(irr::IrrlichtDevice* device, World& world, const KeyValueStore& sharedRegistry): _device{device}, _gameWorld{world}, _sharedRegistry{sharedRegistry}
{
	_device->getCursorControl()->setVisible(false);

	auto screenSize = _device->getVideoDriver()->getScreenSize();
	gui::IGUIEnvironment* env = _device->getGUIEnvironment();

	auto* skin = env->getSkin();
	gui::IGUIFont* font = gui::CGUITTFont::createTTFont(_device->getGUIEnvironment(), "./media/OpenSans-Bold.ttf", 16);
	if(!font)
		std::cerr << "FAILED TO LOAD THE FONT\n";
	skin->setFont(font);
	font->drop();

	_healthBar = new gui::ProgressBar(env, core::rect<s32>(20, 20, 220, 60), env->getRootGUIElement());
	_healthBar->setColors(video::SColor(155, 255,255,255), video::SColor(200, 255,0,0));
	_healthBar->setLabel(L"HP:");
	_healthBar->setValueDisplayMode(irr::gui::ProgressBar::ValueDisplayMode::Abs);
	_healthBar->drop();

	int castIndLen = 200;
	_castingIndicator = new gui::ProgressBar(env, core::rect<s32>(0, 0, castIndLen, 40), env->getRootGUIElement());
	_castingIndicator->setRelativePosition(vec2i((screenSize.Width-castIndLen)/2, 30));
	_castingIndicator->setAlignment(gui::EGUI_ALIGNMENT::EGUIA_CENTER, gui::EGUI_ALIGNMENT::EGUIA_CENTER, gui::EGUI_ALIGNMENT::EGUIA_UPPERLEFT, gui::EGUI_ALIGNMENT::EGUIA_UPPERLEFT);
	_castingIndicator->setColors(video::SColor(155, 255,255,255), video::SColor(200, 0,0,255));
	env->addStaticText(L"", core::rect<s32>(0, 0, _castingIndicator->getRelativePosition().getWidth(), _castingIndicator->getRelativePosition().getHeight()), false, false, _castingIndicator);
	_castingIndicator->drop();

	int spellInHandsInfoPanelWidth = 100;
	_spellInHandsInfo = _device->getGUIEnvironment()->addStaticText(L"SIH info", core::rect<s32>(0, 0, spellInHandsInfoPanelWidth, 80), false, false, env->getRootGUIElement(), -1, true);
	_spellInHandsInfo->setBackgroundColor(video::SColor(200, 255,255,255));
	_spellInHandsInfo->setRelativePosition(vec2i(screenSize.Width-spellInHandsInfoPanelWidth-30, 30));
	_spellInHandsInfo->setAlignment(gui::EGUI_ALIGNMENT::EGUIA_LOWERRIGHT, gui::EGUI_ALIGNMENT::EGUIA_LOWERRIGHT, gui::EGUI_ALIGNMENT::EGUIA_UPPERLEFT, gui::EGUI_ALIGNMENT::EGUIA_UPPERLEFT);

	new irr::gui::CrossHair(env, "./media/crosshair.png", 5, env->getRootGUIElement());
}

void GUI::update(float timeDelta)
{
	updateCastingIndicator(timeDelta);
}

void GUI::onMsg(const EntityEvent& m)
{
	Entity* controlledE = nullptr;
	if(_sharedRegistry.hasKey("controlled_object_id")) {
		ID id = _sharedRegistry.getValue("controlled_object_id");
		controlledE = _gameWorld.getEntity(id);
	}
	if(m.componentT == ComponentType::AttributeStore) {
		_healthBar->setProgress(0);
		if(controlledE != nullptr) {
			AttributeStoreComponent* as = controlledE->getComponent<AttributeStoreComponent>();
			if(as != nullptr) {
				if(as->hasAttribute("health") && as->hasAttribute("max-health")) {
					_healthBar->setProgress(as->getAttribute("health")/as->getAttribute("max-health"));
					_healthBar->setMaxValue(as->getAttribute("max-health"));
					return;
				}
			}
		}
	}
	else if(m.componentT == ComponentType::Wizard)
		if(controlledE != nullptr) {
			WizardComponent* wc = controlledE->getComponent<WizardComponent>();
			if(wc != nullptr) {
				_spellInHandsInfo->setText(std::wstring(
						L"Power:   "+std::to_wstring(wc->getSpellInHandsPower()) + L"\n" +
						L"Radius:  "+std::to_wstring(wc->getSpellInHandsRadius()) + L"\n" +
						L"Speed:   "+std::to_wstring(wc->getSpellInHandsSpeed())
						).c_str());
				if(!wc->getCurrentJob().empty()) {
					_castingIndicator->setVisible(true);
					_castingIndicator->setProgress(wc->getCurrentJobProgress()/wc->getCurrentJobDuration());
					std::string job = wc->getCurrentJob();
					std::wstring w;
					w.assign(job.begin(), job.end());
					auto text = static_cast<gui::IGUIStaticText*>(*_castingIndicator->getChildren().begin());
					text->setText(w.c_str());
					vec2i ciSize(_castingIndicator->getAbsolutePosition().getWidth(), _castingIndicator->getAbsolutePosition().getHeight());
					text->setRelativePosition((ciSize-vec2i(text->getTextWidth(), text->getTextHeight()))/2);
				}
				else
					_castingIndicator->setVisible(false);
			}
		}
}

void GUI::updateCastingIndicator(float timeDelta)
{
	if(_sharedRegistry.hasKey("controlled_object_id")) {
		ID id = _sharedRegistry.getValue("controlled_object_id");
		Entity* e = _gameWorld.getEntity(id);
		if(e != nullptr) {
			WizardComponent* wc = e->getComponent<WizardComponent>();
			if(wc != nullptr)
				_castingIndicator->setProgress(_castingIndicator->getProgress() + timeDelta/wc->getCurrentJobDuration());
		}
	}
}
