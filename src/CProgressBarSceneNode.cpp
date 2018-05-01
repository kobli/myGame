#include "CProgressBarSceneNode.hpp"

CProgressBarSceneNode::CProgressBarSceneNode(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id, core::vector3df position, core::vector2di size, gui::IGUIFont* font)
	: scene::ISceneNode(parent, mgr, id), ProgressBar(size), Position{position}, Font{font}
{
	float hw = getSize().X/2.;
	float hh = getSize().Y/2.;
	Box.MinEdge.set(-hw, -hh, -1.0); 
	Box.MaxEdge.set(hw, hh, 1.0); 
}

void CProgressBarSceneNode::OnRegisterSceneNode()
{
	if(IsVisible)
		SceneManager->registerNodeForRendering(this);

	irr::scene::ISceneNode::OnRegisterSceneNode();
}

void CProgressBarSceneNode::setLabel(const wchar_t* label)
{
	Label = label;
}

void CProgressBarSceneNode::render()
{
	if(!this->IsVisible)
		return;

	video::IVideoDriver* driver = SceneManager->getVideoDriver();
	core::position2d<s32> pos = SceneManager->getSceneCollisionManager()->getScreenCoordinatesFrom3DPosition(getAbsolutePosition()+Position, SceneManager->getActiveCamera()); 

	core::rect<s32> back(pos, core::dimension2di(getSize()));
	s32 halfWidth = getSize().X/2; 
	back.UpperLeftCorner.X -= halfWidth; 
	back.LowerRightCorner.X -= halfWidth; 

	irr::core::rect<s32> progress(back.UpperLeftCorner, back.UpperLeftCorner + core::vector2di(getSize().X*getProgress(),getSize().Y));

	driver->draw2DRectangle(getBackgroundColor(), back, &back);
	driver->draw2DRectangle(getProgressColor(), progress, &progress);

	Font->draw(Label.c_str(), back, video::SColor(255, 0, 0, 0), false, true, &back);
	Font->draw(getValueString().c_str(), back, video::SColor(255, 0, 0, 0), true, true, &back);
}

const core::aabbox3d<f32>& CProgressBarSceneNode::getBoundingBox() const
{
	return Box;
}

u32 CProgressBarSceneNode::getMaterialCount() const
{
	return 1;
}

video::SMaterial& CProgressBarSceneNode::getMaterial(u32 /*i*/)
{
	return Material;
}   
