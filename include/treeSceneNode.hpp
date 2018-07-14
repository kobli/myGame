#ifndef TREESCENENODE_HPP_17_08_19_13_09_19
#define TREESCENENODE_HPP_17_08_19_13_09_19 
#include "main.hpp"

class TreeSceneNode: public irr::scene::ISceneNode
{
	public:
		TreeSceneNode(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id, scene::IMesh* mesh)
			: scene::ISceneNode(parent, mgr, id), Mesh(mesh), Imposter(nullptr)
		{
			Material.Lighting = false;
			Box = mesh->getBoundingBox();

			Imposter = SceneManager->addBillboardSceneNode(this, core::vector2df(Box.getExtent().Y, Box.getExtent().Y), core::vector3df(0,Box.getExtent().Y/2, 0));
			Imposter->setMaterialType(video::E_MATERIAL_TYPE::EMT_TRANSPARENT_ALPHA_CHANNEL_REF);
			Imposter->setMaterialTexture(0, getImposterTexture());
			Imposter->setMaterialFlag(video::EMF_LIGHTING, false);
		}

		virtual void OnRegisterSceneNode()
		{
			if(IsVisible)
				SceneManager->registerNodeForRendering(this);

			irr::scene::ISceneNode::OnRegisterSceneNode();
		}

		virtual void render()
		{
			video::IVideoDriver* driver = SceneManager->getVideoDriver();
			float distanceFromCamera = SceneManager->getActiveCamera()->getPosition().getDistanceFrom(getPosition());
			if(distanceFromCamera < 80) {
				driver->setMaterial(Material);
				driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
				for(u32 i = 0; i < Mesh->getMeshBufferCount(); i++)
					driver->drawMeshBuffer(Mesh->getMeshBuffer(i));
				Imposter->setVisible(false);
			}
			else
				Imposter->setVisible(true);
		}

		virtual const core::aabbox3d<f32>& getBoundingBox() const
		{
			return Box;
		}

		virtual u32 getMaterialCount() const
		{
			return 1;
		}

		virtual video::SMaterial& getMaterial(u32 i)
		{
			return Material;
		}   

	private:
		scene::IMesh* Mesh;
		core::aabbox3d<f32> Box;
		video::SMaterial Material;
		scene::IBillboardSceneNode* Imposter;

		video::ITexture* getImposterTexture() {
			video::ITexture* tt = SceneManager->getVideoDriver()->addRenderTargetTexture(core::dimension2d<u32>(128, 128), "TT1");
			auto driver = SceneManager->getVideoDriver();

			auto altSceneManager = SceneManager->createNewSceneManager(false);
			altSceneManager->addMeshSceneNode(Mesh);
			float meshHalfHeight = Box.getExtent().Y/2;
			altSceneManager->addCameraSceneNode(0, vec3f(10,meshHalfHeight,0), vec3f(0,meshHalfHeight,0), true);
			driver->setRenderTarget(tt, true, true, video::SColor(0,0,0,0));
			altSceneManager->drawAll();

			driver->setRenderTarget(0, true, true, 0);
			
			return tt;
		}
};
#endif /* TREESCENENODE_HPP_17_08_19_13_09_19 */
