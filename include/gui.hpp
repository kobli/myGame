#ifndef GUI_HPP_17_09_14_21_57_18
#define GUI_HPP_17_09_14_21_57_18 
#include "world.hpp"
#include "progressBar.hpp"

class GUIPanel: public irr::gui::IGUIElement
{
	public:
		GUIPanel(irr::gui::IGUIEnvironment* environment, IGUIElement* parent = nullptr, s32 id = -1, core::rect<s32> rectangle = core::rect<s32>(0,0,0,0)): IGUIElement(irr::gui::EGUIET_ELEMENT, environment, parent, id, rectangle), _drawBackground{false}
	{}

		void addChild(IGUIElement* e)
		{
			IGUIElement::addChild(e);
			recalculateElementPositions();
		}

		virtual void setDrawBackground(bool draw=true)
		{
			_drawBackground = draw;
		}

		virtual void setBackgroundColor(video::SColor c)
		{
			_backgroundColor = c;
		}

		virtual bool isDrawingBackground() const
		{
			return _drawBackground;
		}

		virtual video::SColor getBackgroundColor() const
		{
			return _backgroundColor;
		}

		virtual void draw()
		{
			if(!this->IsVisible)
				return;

			if(_drawBackground)
				Environment->getVideoDriver()->draw2DRectangle(_backgroundColor, getAbsolutePosition());

			for(auto& c: this->getChildren())
				c->draw();
		}

	private:
		bool _drawBackground;
		video::SColor _backgroundColor;

		virtual void recalculateElementPositions() = 0;
};

class GUIPanelFlowHorizontal: public GUIPanel
{
	public:
		GUIPanelFlowHorizontal(irr::gui::IGUIEnvironment* environment, IGUIElement* parent = nullptr, s32 id = -1, core::rect<s32> rectangle = core::rect<s32>(0,0,0,0)): GUIPanel(environment, parent, id, rectangle), _padding{0}
	{}
		void setPadding(int padding)
		{
			_padding = padding;
			recalculateElementPositions();
		}

		virtual void recalculateElementPositions()
		{
			int xPos = 0;
			int panelHeight = getAbsolutePosition().getHeight();
			for(IGUIElement* e : getChildren()) {
				vec2i pos(xPos, (panelHeight-e->getAbsolutePosition().getHeight())/2);
				xPos += e->getAbsolutePosition().getWidth() + _padding;
				e->setRelativePosition(pos);
			}
		}

	private:
		int _padding;
};

class GUIPanelFlowVertical: public GUIPanel
{
	public:
		GUIPanelFlowVertical(irr::gui::IGUIEnvironment* environment, IGUIElement* parent = nullptr, s32 id = -1, core::rect<s32> rectangle = core::rect<s32>(0,0,0,0)): GUIPanel(environment, parent, id, rectangle), _padding{0}
	{}
		void setPadding(int padding)
		{
			_padding = padding;
			recalculateElementPositions();
		}

		virtual void recalculateElementPositions()
		{
			int yPos = 0;
			int panelWidth = getAbsolutePosition().getWidth();
			for(IGUIElement* e : getChildren()) {
				vec2i pos((panelWidth-e->getAbsolutePosition().getWidth())/2, yPos);
				yPos += e->getAbsolutePosition().getHeight() + _padding;
				e->setRelativePosition(pos);
			}
		}

	private:
		int _padding;
};


class GUI: public Observer<EntityEvent>
{
	public:
		GUI(irr::IrrlichtDevice* device, World& world, const KeyValueStore& sharedRegistry);
		void update(float timeDelta);

	private:
		irr::IrrlichtDevice* _device;
		World& _gameWorld;
		const KeyValueStore& _sharedRegistry;

		typedef std::function<void(const wchar_t* l)> TextSetter;
		std::map<std::string, TextSetter> _textSetters;

		gui::ProgressBar* _healthBar;
		gui::ProgressBar* _castingIndicator;
		gui::IGUIElement* _spellInHandsInfo;

		irr::gui::ProgressBar* _spellAttrPowInfo;
		irr::gui::ProgressBar* _spellAttrSpeedInfo;
		irr::gui::ProgressBar* _spellAttrSizeInfo;
		GUIPanelFlowHorizontal* _spellEffectsInfo;
		GUIPanelFlowHorizontal* _spellCommandQInfo;

		void onMsg(const EntityEvent& m) override;
		void updateCastingIndicator(float timeDelta);
};
#endif /* GUI_HPP_17_09_14_21_57_18 */
