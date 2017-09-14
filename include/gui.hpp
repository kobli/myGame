#ifndef GUI_HPP_17_09_14_21_57_18
#define GUI_HPP_17_09_14_21_57_18 
#include "world.hpp"
#include "progressBar.hpp"

class GUI: public Observer<EntityEvent>
{
	public:
		GUI(irr::IrrlichtDevice* device, World& world, const KeyValueStore& sharedRegistry);
		void update(float timeDelta);

	private:
		irr::IrrlichtDevice* _device;
		World& _gameWorld;
		const KeyValueStore& _sharedRegistry;

		gui::ProgressBar* _healthBar;
		gui::ProgressBar* _castingIndicator;
		gui::IGUIStaticText* _spellInHandsInfo;

		void onMsg(const EntityEvent& m) override;
		void updateCastingIndicator(float timeDelta);
};
#endif /* GUI_HPP_17_09_14_21_57_18 */
