#include <sstream>
#include <regex>
#include "gui.hpp"
#include "CGUITTFont.h"
#include "crosshair.hpp"

const std::map<unsigned, std::string> EFFECT_ICONS{{0, "body"}, {1,"fire"}, {3, "heal"}};

using namespace irr::gui;

GUI::GUI(irr::IrrlichtDevice* device, World& world, const KeyValueStore& sharedRegistry, const KeyValueStore& gameRegistry)
	: _device{device}, _gameWorld{world}, _sharedRegistry{sharedRegistry}, _gameRegistry{gameRegistry}
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

	_healthBar = new gui::CGUIProgressBar(env, core::rect<s32>(20, 20, 220, 60), env->getRootGUIElement());
	_healthBar->setColors(video::SColor(155, 255,255,255), video::SColor(200, 255,0,0));
	_healthBar->setLabel(L"HP:");
	_healthBar->setValueDisplayMode(irr::gui::CGUIProgressBar::ValueDisplayMode::Abs);

	int castIndLen = 200;
	_castingIndicator = new gui::CGUIProgressBar(env, core::rect<s32>(0, 0, castIndLen, 20), env->getRootGUIElement());
	_castingIndicator->setRelativePosition(vec2i((screenSize.Width-castIndLen)/2, screenSize.Height-165));
	_castingIndicator->setAlignment(gui::EGUI_ALIGNMENT::EGUIA_CENTER, gui::EGUI_ALIGNMENT::EGUIA_CENTER, gui::EGUI_ALIGNMENT::EGUIA_LOWERRIGHT, gui::EGUI_ALIGNMENT::EGUIA_LOWERRIGHT);
	_castingIndicator->setColors(video::SColor(155, 255,255,255), video::SColor(255, 255,140,70));
	_castingIndicator->setLabel(L"Casting ...");

	_castingEffect = env->addImage(
								_device->getVideoDriver()->getTexture(("./media/spell_icon_"+EFFECT_ICONS.at(0)+"_32.png").c_str()),
								vec2i(0,0), false);
	_castingEffect->setAlignment(gui::EGUI_ALIGNMENT::EGUIA_CENTER, gui::EGUI_ALIGNMENT::EGUIA_CENTER, gui::EGUI_ALIGNMENT::EGUIA_LOWERRIGHT, gui::EGUI_ALIGNMENT::EGUIA_LOWERRIGHT);
	_castingEffect->setRelativePosition(_castingIndicator->getAbsolutePosition().UpperLeftCorner-vec2i(32, 0));

	int spellAttributesInfoPanelWidth = castIndLen;
	int spellAttributeProgBarHeight = 20;
	auto spellAttributesInfo = new GUIPanelFlowVertical(env, env->getRootGUIElement(), -1, core::rect<s32>(0, 0, spellAttributesInfoPanelWidth, spellAttributeProgBarHeight*3));
	
	_spellAttrPowInfo = new CGUIProgressBar(env, core::rect<s32>(0, 0, spellAttributesInfoPanelWidth, spellAttributeProgBarHeight));
	_spellAttrPowInfo->setColors(video::SColor(155, 255,255,255), video::SColor(200, 255,100,100));
	_spellAttrPowInfo->setLabel(L"Power");
	spellAttributesInfo->addChild(_spellAttrPowInfo);

	_spellAttrSizeInfo = new CGUIProgressBar(env, core::rect<s32>(0, 0, spellAttributesInfoPanelWidth, spellAttributeProgBarHeight));
	_spellAttrSizeInfo->setColors(video::SColor(155, 255,255,255), video::SColor(200, 100,255,100));
	_spellAttrSizeInfo->setLabel(L"Size");
	spellAttributesInfo->addChild(_spellAttrSizeInfo);

	_spellAttrSpeedInfo = new CGUIProgressBar(env, core::rect<s32>(0, 0, spellAttributesInfoPanelWidth, spellAttributeProgBarHeight));
	_spellAttrSpeedInfo->setColors(video::SColor(155, 255,255,255), video::SColor(200, 100,100,255));
	_spellAttrSpeedInfo->setLabel(L"Speed");
	spellAttributesInfo->addChild(_spellAttrSpeedInfo);

	_spellInHandsInfo = new GUIPanelFlowHorizontal(env, env->getRootGUIElement(), -1, core::rect<s32>(0, 0, spellAttributesInfoPanelWidth+400, spellAttributesInfo->getAbsolutePosition().getHeight()));
	_spellInHandsInfo->addChild(spellAttributesInfo);
	_spellInHandsInfo->setRelativePosition(vec2i((screenSize.Width-spellAttributesInfoPanelWidth)/2., screenSize.Height-100));
	_spellInHandsInfo->setAlignment(gui::EGUI_ALIGNMENT::EGUIA_CENTER, gui::EGUI_ALIGNMENT::EGUIA_CENTER, gui::EGUI_ALIGNMENT::EGUIA_LOWERRIGHT, gui::EGUI_ALIGNMENT::EGUIA_LOWERRIGHT);

	_spellEffectsInfo = new GUIPanelFlowHorizontal(env, env->getRootGUIElement(), -1, core::rect<s32>(0, 0, 200, spellAttributeProgBarHeight*3));
	_spellInHandsInfo->addChild(_spellEffectsInfo);

	_spellCommandQInfo = new GUIPanelFlowHorizontal(env, env->getRootGUIElement(), -1, core::rect<s32>(0, 0, 320, 32));
	_spellCommandQInfo->setRelativePosition(vec2i((screenSize.Width-spellAttributesInfoPanelWidth)/2., screenSize.Height-100-40));
	_spellCommandQInfo->setAlignment(gui::EGUI_ALIGNMENT::EGUIA_CENTER, gui::EGUI_ALIGNMENT::EGUIA_CENTER, gui::EGUI_ALIGNMENT::EGUIA_LOWERRIGHT, gui::EGUI_ALIGNMENT::EGUIA_LOWERRIGHT);

	int gameModeInfoXMargin = 300;
	int gameModeInfoSizeX = 700;
	_gameModeInfo = new GUIPanelFlowHorizontal(env, env->getRootGUIElement(), -1, core::rect<s32>(0, 0, gameModeInfoSizeX, 25));
	_gameModeInfo->setRelativePosition(vec2i((screenSize.Width-gameModeInfoSizeX)/2., 30));
	_gameModeInfo->setAlignment(gui::EGUI_ALIGNMENT::EGUIA_CENTER, gui::EGUI_ALIGNMENT::EGUIA_CENTER, gui::EGUI_ALIGNMENT::EGUIA_UPPERLEFT, gui::EGUI_ALIGNMENT::EGUIA_UPPERLEFT);
	_gameModeInfo->setBackgroundColor(video::SColor(155, 255, 255, 255));
	_gameModeInfo->setDrawBackground(true);


	auto bodyCountInfo = new GUIPanelFlowHorizontal(env, env->getRootGUIElement(), -1, core::rect<s32>(20, 100, 220, 132));
	bodyCountInfo->addChild(env->addImage(_device->getVideoDriver()->getTexture("./media/spell_icon_body_32.png"), vec2i(0)));
	bodyCountInfo->addChild(env->addStaticText(L"<bodyC> / <bodyTotal>", core::rect<s32>(0,0,100,20)));
	bodyCountInfo->setPadding(70);
	bodyCountInfo->setBackgroundColor(video::SColor(155, 255, 255, 255));
	bodyCountInfo->setDrawBackground(true);
	_textSetters["bodyStatus"] = [bodyCountInfo](const wchar_t* text){
		(*(bodyCountInfo->getChildren().begin()+1))->setText(text);
	};

	new irr::gui::CrossHair(env, "./media/crosshair.png", 5, env->getRootGUIElement());

	int chatBoxWidth = 350;
	int chatBoxHeight = 300;
	int chatInputHeight = 30;
	auto chat = new GUIPanelFlowVertical(env, env->getRootGUIElement(), -1, core::rect<s32>(0, 0, chatBoxWidth, chatBoxHeight));
	chat->setRelativePosition(vec2i(20, screenSize.Height-chatBoxHeight-20));
	chat->setAlignment(gui::EGUI_ALIGNMENT::EGUIA_UPPERLEFT, gui::EGUI_ALIGNMENT::EGUIA_UPPERLEFT, gui::EGUI_ALIGNMENT::EGUIA_LOWERRIGHT, gui::EGUI_ALIGNMENT::EGUIA_LOWERRIGHT);

	auto chatContent = new GUIChat(env, nullptr, GUIElementID::ChatContent, core::rect<s32>(0, 0, chatBoxWidth, chatBoxHeight-chatInputHeight));
	chatContent->setDrawBackground(true);
	chatContent->setBackgroundColor(video::SColor(155, 255, 255, 255));
	chatContent->setPadding(5);
	auto chatInput = env->addEditBox(L"", core::rect<s32>(0, 0, chatBoxWidth, chatInputHeight), false, nullptr, GUIElementID::ChatInput);
	chat->addChild(chatContent);
	chat->addChild(chatInput);
}

GUI::~GUI()
{
	_device->getGUIEnvironment()->clear();
}

void GUI::update(float timeDelta)
{
	updateCastingIndicator(timeDelta);
	updateGameModeInfo();
}

unsigned breakStringToWidthAndGetTotalHeight(std::wstring& s, unsigned maxWidth, IGUIFont* f)
{
	size_t head = 0;
	std::wstring r;
	core::dimension2du dim;
	while(head < s.length()) {
		size_t wordEnd = s.find(L" ", head);
		std::wstring w = s.substr(head, wordEnd-head);
		dim = f->getDimension((r+L" "+w).c_str());
		if(dim.Width > maxWidth) {
			//TODO break word if its longer than maxWidth
			while(f->getDimension((r+L" "+w).c_str()).Width > maxWidth)
				w.pop_back();
			r += w+L"\n";
			head += w.length();
		}
		else {
			r += L" "+w;
			if(wordEnd == string::npos)
				head = string::npos;
			else
				head += wordEnd-head+1;
		}
	}
	s = r;
	return dim.Height;	
}

void GUI::displayMessage(std::string message, std::string /*tag*/)
{
	auto chatContent = static_cast<GUIChat*>(_device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIElementID::ChatContent, true));
	std::wstring m(message.begin(), message.end());
	auto t = _device->getGUIEnvironment()->addStaticText(m.c_str(), core::rect<s32>(0, 0, chatContent->getRelativePosition().getWidth(), 10));
	t->setWordWrap(true);
	auto s = core::dimension2du(chatContent->getRelativePosition().getWidth(), t->getTextHeight());
	t->setMaxSize(s);
	t->setMinSize(s);
	chatContent->addChild(t);
}

void GUI::updateGameModeInfo()
{
	clearGUIElement(_gameModeInfo);
	if(!_gameRegistry.hasKey("gm_info_template"))
		return;
	auto env = _device->getGUIEnvironment();
	std::string gmInfo = fillGamemodeInfoStr(_gameRegistry.getValue<std::string>("gm_info_template"));
	std::istringstream iss(gmInfo);
	std::string part;
	int c = 0;
	int totalTextWidth = 0;
	while(std::getline(iss, part, '|')) {
		auto text = env->addStaticText(core::stringw(part.c_str()).c_str(), core::rect<s32>(0,0,0,0));
		text->setWordWrap(false);
		text->setMinSize(core::dimension2du(text->getTextWidth(), 20));
		totalTextWidth += text->getTextWidth();
		_gameModeInfo->addChild(text);
		c++;
	}
	if(c > 1)
		_gameModeInfo->setPadding((_gameModeInfo->getAbsolutePosition().getWidth()-totalTextWidth)/(c-1));
	if(totalTextWidth > _gameModeInfo->getAbsolutePosition().getWidth())
		std::cerr << "GameModeInfo text too long: " << totalTextWidth << " / " << _gameModeInfo->getAbsolutePosition().getWidth() << std::endl;
}

void GUI::onMsg(const EntityEvent& m)
{
	auto env = _device->getGUIEnvironment();
	Entity* controlledE = nullptr;
	if(_sharedRegistry.hasKey("controlled_object_id")) {
		ID id = _sharedRegistry.getValue<ID>("controlled_object_id");
		controlledE = _gameWorld.getEntity(id);
	}
	if(m.componentT == ComponentType::AttributeStore) {
		_healthBar->setProgress(0);
		if(controlledE != nullptr) {
			AttributeStoreComponent* as = controlledE->getComponent<AttributeStoreComponent>();
			if(as != nullptr) {
				if(as->hasAttribute("health") && as->hasAttribute("max-health")) {
					_healthBar->setProgress(as->getAttribute<float>("health")/as->getAttribute<float>("max-health"));
					_healthBar->setMaxValue(as->getAttribute<float>("max-health"));
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

					clearGUIElement(_spellEffectsInfo);
					for(unsigned effectID: wc->getSpellInHandsEffects())
						_spellEffectsInfo->addChild(
								env->addImage(
									_device->getVideoDriver()->getTexture(("./media/spell_icon_"+EFFECT_ICONS.at(effectID)+"_64.png").c_str()),
									vec2i(0,0)
									));
					_spellInHandsInfo->setVisible(true);
				}
				else
					_spellInHandsInfo->setVisible(false);

				clearGUIElement(_spellCommandQInfo);
				for(unsigned effectID : wc->getCommandQueue())
					_spellCommandQInfo->addChild(
							env->addImage(
								_device->getVideoDriver()->getTexture(("./media/spell_icon_"+EFFECT_ICONS.at(effectID)+"_32.png").c_str()),
								vec2i(0,0)
								));

				if(!wc->getCurrentJob().empty()) {
					_castingIndicator->setVisible(true);
					_castingIndicator->setProgress(wc->getCurrentJobProgress()/wc->getCurrentJobDuration());
					_castingEffect->setVisible(true);
					_castingEffect->setImage(_device->getVideoDriver()->getTexture(("./media/spell_icon_"+EFFECT_ICONS.at(wc->getCurrentJobEffectId())+"_32.png").c_str()));
				}
				else {
					_castingIndicator->setVisible(false);
					_castingEffect->setVisible(false);
				}
			}
		}
}

void GUI::updateCastingIndicator(float timeDelta)
{
	if(_sharedRegistry.hasKey("controlled_object_id")) {
		ID id = _sharedRegistry.getValue<ID>("controlled_object_id");
		Entity* e = _gameWorld.getEntity(id);
		if(e != nullptr) {
			WizardComponent* wc = e->getComponent<WizardComponent>();
			if(wc != nullptr)
				_castingIndicator->setProgress(_castingIndicator->getProgress() + timeDelta/wc->getCurrentJobDuration());
		}
	}
}

void GUI::clearGUIElement(IGUIElement* e)
{
	while(!e->getChildren().empty())
		e->removeChild(*e->getChildren().begin());
}

std::string GUI::fillGamemodeInfoStr(std::string s)
{
	AttributeStoreComponent* asc = nullptr;
	if(_sharedRegistry.hasKey("controlled_object_id")) {
		ID id = _sharedRegistry.getValue<ID>("controlled_object_id");
		Entity* e = _gameWorld.getEntity(id);
		if(e != nullptr)
			asc = e->getComponent<AttributeStoreComponent>();
	}
	std::string r;
	auto callback = [&](std::string const& m){
		if(m.size() > 0 && m.front() == '<' && m.back() == '>') {
			std::string tagWithoutBrackets = m.substr(1, m.size()-2);
			if(asc && asc->hasAttribute(tagWithoutBrackets))
				r += asc->getAttribute<std::string>(tagWithoutBrackets);
			else if(_gameRegistry.hasKey(tagWithoutBrackets))
				r += _gameRegistry.getValue<std::string>(tagWithoutBrackets);
			else
				r += m;
		}
		else
			r += m;
	};
	std::regex tag("<[^>]*>");
	std::sregex_token_iterator
		begin(s.begin(), s.end(), tag, {-1,0}),
		end;
	std::for_each(begin,end,callback);
	return r;
}
