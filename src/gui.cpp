#include "gui.hpp"
#include "CGUITTFont.h"
#include "crosshair.hpp"

using namespace irr::gui;

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
	_castingIndicator = new gui::ProgressBar(env, core::rect<s32>(0, 0, castIndLen, 20), env->getRootGUIElement());
	_castingIndicator->setRelativePosition(vec2i((screenSize.Width-castIndLen)/2, screenSize.Height-160));
	_castingIndicator->setAlignment(gui::EGUI_ALIGNMENT::EGUIA_CENTER, gui::EGUI_ALIGNMENT::EGUIA_CENTER, gui::EGUI_ALIGNMENT::EGUIA_UPPERLEFT, gui::EGUI_ALIGNMENT::EGUIA_UPPERLEFT);
	_castingIndicator->setColors(video::SColor(155, 255,255,255), video::SColor(220, 80,80,255));
	_castingIndicator->setLabel(L"Casting ...");

	int spellAttributesInfoPanelWidth = castIndLen;
	int spellAttributeProgBarHeight = 20;
	auto spellAttributesInfo = new GUIPanelFlowVertical(env, env->getRootGUIElement(), -1, core::rect<s32>(0, 0, spellAttributesInfoPanelWidth, spellAttributeProgBarHeight*3));
	
	_spellAttrPowInfo = new ProgressBar(env, core::rect<s32>(0, 0, spellAttributesInfoPanelWidth, spellAttributeProgBarHeight));
	_spellAttrPowInfo->setColors(video::SColor(155, 255,255,255), video::SColor(200, 255,100,100));
	_spellAttrPowInfo->setLabel(L"Power");
	spellAttributesInfo->addChild(_spellAttrPowInfo);

	_spellAttrSizeInfo = new ProgressBar(env, core::rect<s32>(0, 0, spellAttributesInfoPanelWidth, spellAttributeProgBarHeight));
	_spellAttrSizeInfo->setColors(video::SColor(155, 255,255,255), video::SColor(200, 100,255,100));
	_spellAttrSizeInfo->setLabel(L"Size");
	spellAttributesInfo->addChild(_spellAttrSizeInfo);

	_spellAttrSpeedInfo = new ProgressBar(env, core::rect<s32>(0, 0, spellAttributesInfoPanelWidth, spellAttributeProgBarHeight));
	_spellAttrSpeedInfo->setColors(video::SColor(155, 255,255,255), video::SColor(200, 100,100,255));
	_spellAttrSpeedInfo->setLabel(L"Speed");
	spellAttributesInfo->addChild(_spellAttrSpeedInfo);

	_spellInHandsInfo = new GUIPanelFlowHorizontal(env, env->getRootGUIElement(), -1, core::rect<s32>(0, 0, spellAttributesInfoPanelWidth+400, spellAttributesInfo->getAbsolutePosition().getHeight()));
	_spellInHandsInfo->addChild(spellAttributesInfo);
	_spellInHandsInfo->setRelativePosition(vec2i((screenSize.Width-spellAttributesInfoPanelWidth)/2., screenSize.Height-100));
	_spellInHandsInfo->setAlignment(gui::EGUI_ALIGNMENT::EGUIA_CENTER, gui::EGUI_ALIGNMENT::EGUIA_CENTER, gui::EGUI_ALIGNMENT::EGUIA_UPPERLEFT, gui::EGUI_ALIGNMENT::EGUIA_UPPERLEFT);

	_spellEffectsInfo = new GUIPanelFlowHorizontal(env, env->getRootGUIElement(), -1, core::rect<s32>(0, 0, 200, spellAttributeProgBarHeight*3));
	_spellInHandsInfo->addChild(_spellEffectsInfo);


	auto bodyCountInfo = new GUIPanelFlowHorizontal(env, env->getRootGUIElement(), -1, core::rect<s32>(20, 100, 220, 164));
	
	bodyCountInfo->addChild(env->addImage(_device->getVideoDriver()->getTexture("./media/spell_icon_body.png"), vec2i(0)));
	bodyCountInfo->addChild(env->addStaticText(L"<bodyC> / <bodyTotal>", core::rect<s32>(0,0,100,20)));
	bodyCountInfo->setPadding(70);
	bodyCountInfo->setBackgroundColor(video::SColor(155, 255, 255, 255));
	bodyCountInfo->setDrawBackground(true);
	_textSetters["bodyStatus"] = [bodyCountInfo](const wchar_t* text){
		(*(bodyCountInfo->getChildren().begin()+1))->setText(text);
	};

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
				_textSetters["bodyStatus"]((std::to_wstring(wc->getAvailableBodyC()) + L" / " + std::to_wstring(wc->getTotalBodyC())).c_str());

				if(wc->hasSpellInHands()) {
					_spellAttrPowInfo->setProgress(wc->getSpellInHandsPower());
					_spellAttrSizeInfo->setProgress(wc->getSpellInHandsRadius());
					_spellAttrSpeedInfo->setProgress(wc->getSpellInHandsSpeed());

					auto env = _device->getGUIEnvironment();
					while(!_spellEffectsInfo->getChildren().empty())
						_spellEffectsInfo->removeChild(*_spellEffectsInfo->getChildren().begin());
					static std::map<unsigned, std::string> effectIcons{{1,"fire"}, {3, "heal"}};
					for(unsigned effectID: wc->getSpellInHandsEffects())
						_spellEffectsInfo->addChild(
								env->addImage(
									_device->getVideoDriver()->getTexture(("./media/spell_icon_"+effectIcons[effectID]+".png").c_str()),
									vec2i(0,0)
									));
					_spellInHandsInfo->setVisible(true);
				}
				else
					_spellInHandsInfo->setVisible(false);

				if(!wc->getCurrentJob().empty()) {
					_castingIndicator->setVisible(true);
					_castingIndicator->setProgress(wc->getCurrentJobProgress()/wc->getCurrentJobDuration());
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
