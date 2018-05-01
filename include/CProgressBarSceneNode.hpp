#ifndef CPROGRESSBARSCENENODE_HPP_18_05_01_10_13_37
#define CPROGRESSBARSCENENODE_HPP_18_05_01_10_13_37 
#include "main.hpp"
#include "progressBar.hpp"

class CProgressBarSceneNode: public irr::scene::ISceneNode, public ProgressBar
{
	public:
		CProgressBarSceneNode(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id, core::vector3df position, core::vector2di size, gui::IGUIFont* font = nullptr);
		virtual void OnRegisterSceneNode();
		virtual void setLabel(const wchar_t* label) override;
		virtual void render();
		virtual const core::aabbox3d<f32>& getBoundingBox() const;
		virtual u32 getMaterialCount() const;
		virtual video::SMaterial& getMaterial(u32 i);

	private:
		core::aabbox3d<f32> Box;
		video::SMaterial Material;
		core::vector3df Position;
		gui::IGUIFont* Font;
		std::wstring Label;
};


#endif /* CPROGRESSBARSCENENODE_HPP_18_05_01_10_13_37 */
