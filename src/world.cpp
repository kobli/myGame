#include <world.hpp>
#include <cassert>

EntityEvent::EntityEvent(ID eID, ComponentType compT, bool c, bool d)
	: base_t{eID, compT}, created{c}, destroyed{d} {
}

////////////////////////////////////////////////////////////

ObservableComponentBase::ObservableComponentBase(ID parentEntID, ComponentType realCompType)
 	: Observable<EntityEvent>{EntityEvent{parentEntID, realCompType}, EntityEvent{parentEntID, realCompType}}
{
}

void ObservableComponentBase::notifyObservers()
{
	//TODO .. store entID and compT in ctor
	//here create and send event
	//or create upd msg and store it in ctor .. !!!better
}

////////////////////////////////////////////////////////////

BodyComponent::BodyComponent(ID parentEntID, vec3f position, quaternion rotation, vec3f velocity)
	: ObservableComponentBase{parentEntID, ComponentType::Body}, _position{position}, _rotation{rotation}, _velocity{velocity}
	, _strafeDir{vec2f(0,0)}, _strafeSpeed{120}, _rotDir{0}, _posRotChanged{false}
{}
	
vec3f BodyComponent::getPosition() const
{
	return _position;
}

quaternion BodyComponent::getRotation() const
{
	return _rotation;
}

vec3f BodyComponent::getVelocity() const
{
	return _velocity;
}

i8 BodyComponent::getRotDir() const
{
	return _rotDir;
}

void BodyComponent::setPosition(vec3f p)
{
	if(_position == p)
		return;
	_position = p;
	_posRotChanged = true;
	notifyObservers();
	_posRotChanged = false;
}

void BodyComponent::setRotation(quaternion r)
{
	if(_rotation == r)
		return;
	_rotation = r;
	_posRotChanged = true;
	notifyObservers();
	_posRotChanged = false;
}

void BodyComponent::setVelocity(vec3f v)
{
	if(_velocity == v)
		return;
	_velocity = v;
	notifyObservers();
}

void BodyComponent::setStrafeDir(vec2f strafeDir)
{
	if(_strafeDir == strafeDir)
		return;
	_strafeDir = strafeDir;
	notifyObservers();
}

void BodyComponent::setRotDir(i8 rotDir)
{
	if(_rotDir == rotDir)
		return;
	_rotDir = rotDir;
	notifyObservers();
}

bool BodyComponent::posOrRotChanged()
{
	return _posRotChanged;
}

vec2f BodyComponent::getStrafeDir() const
{
	return _strafeDir;
}

float BodyComponent::getStrafeSpeed() const
{
	return _strafeSpeed;
}

void BodyComponent::serDes(SerDesBase& s)
{
	s.serDes(*this);
}

////////////////////////////////////////////////////////////

GraphicsComponent::GraphicsComponent(ID parentEntID, ComponentType t
		, vec3f posOffset, vec3f rotOffset, vec3f scale)
	: ObservableComponentBase(parentEntID, t)
	, _posOff{posOffset}, _rotOff{rotOffset}, _scale{scale}
{}

vec3f GraphicsComponent::getPosOffset()
{
	return _posOff;
}

vec3f GraphicsComponent::getRotOffset()
{
	return _rotOff;
}

vec3f GraphicsComponent::getScale()
{
	return _scale;
}

void GraphicsComponent::setPosOffset(vec3f p)
{
	if(_posOff == p)
		return;
	_posOff = p;
	notifyObservers();
}

void GraphicsComponent::setRotOffset(vec3f r)
{
	if(_rotOff == r)
		return;
	_rotOff = r;
	notifyObservers();
}

void GraphicsComponent::setScale(vec3f s)
{
	if(_scale == s)
		return;
	_scale = s;
	notifyObservers();
}

void GraphicsComponent::serDes(SerDesBase& s)
{
	s.serDes(*this);
}

// // // // // // // // // // // // // // // // // // // //

SphereGraphicsComponent::SphereGraphicsComponent(ID parentEntID, float radius, vec3f posOffset, vec3f rotOffset, vec3f scale)
	: GraphicsComponent{parentEntID, ComponentType::GraphicsSphere, posOffset, rotOffset, scale}, _radius{radius}
{}

float SphereGraphicsComponent::getRadius()
{
	return _radius;
}

void SphereGraphicsComponent::setRadius(float radius)
{
	if(radius == _radius)
		return;
	_radius = radius;
	notifyObservers();
}

void SphereGraphicsComponent::serDes(SerDesBase& s)
{
	s.serDes(*static_cast<GraphicsComponent*>(this));
	s.serDes(*this);
}

// // // // // // // // // // // // // // // // // // // //

MeshGraphicsComponent::MeshGraphicsComponent(ID parentEntID, string fileName, bool animated, vec3f posOffset, vec3f rotOffset, vec3f scale)
	: GraphicsComponent{parentEntID, ComponentType::GraphicsMesh, posOffset, rotOffset, scale}, _fileName{fileName}, _animated{animated}
{}

void MeshGraphicsComponent::serDes(SerDesBase& s)
{
	s.serDes(*static_cast<GraphicsComponent*>(this));
	s.serDes(*this);
}

string MeshGraphicsComponent::getFileName()
{
	return _fileName;
}

bool MeshGraphicsComponent::isAnimated()
{
	return _animated;
}

////////////////////////////////////////////////////////////

CollisionComponent::CollisionComponent(ID parentEntID
		, float radius, float height, vec3f posOffset, bool kinematic)
	: ObservableComponentBase(parentEntID, ComponentType::Collision)
		, _radius{radius}, _height{height}, _posOff{posOffset}
		, _kinematic{kinematic}
{}

float CollisionComponent::getRadius() const
{
	return _radius;
}

void CollisionComponent::setRadius(float radius)
{
	_radius = radius;
}

float CollisionComponent::getHeight() const
{
	return _height;
}

void CollisionComponent::setHeight(float height)
{
	_height = height;
}

vec3f CollisionComponent::getPosOffset() const
{
	return _posOff;
}

void CollisionComponent::setPosOffset(vec3f pO)
{
	_posOff = pO;
}

void CollisionComponent::serDes(SerDesBase& s)
{
	s.serDes(*this);
}

bool CollisionComponent::isKinematic()
{
	return _kinematic;
}

////////////////////////////////////////////////////////////

WizardComponent::WizardComponent(ID parentEntID):
 	ObservableComponentBase(parentEntID, ComponentType::Wizard)
{
}

void WizardComponent::serDes(SerDesBase&)
{
	//s.serDes(*this);
}

////////////////////////////////////////////////////////////

WorldMap::WorldMap(float patchSize, scene::ISceneManager* scene)
	: _patchSize{patchSize}, _scene{scene}, _heightScale{0.05}
{
	scene::ITerrainSceneNode* terrain = _scene->addTerrainSceneNode(
		"./media/terrain-heightmap.bmp",
		0,					// parent node
		-1,					// node id
		core::vector3df(0),		// position
		core::vector3df(0),		// rotation
		core::vector3df(1)		// scale
		);
	terrain->setScale(core::vector3df(_patchSize/terrain->getBoundingBox().getExtent().X, 
				_heightScale,
				_patchSize/terrain->getBoundingBox().getExtent().Z));
	terrain->setPosition(terrain->getBoundingBox().getCenter()*core::vector3df(-1,0,-1));


	terrain->setMaterialFlag(video::EMF_LIGHTING, false);
	terrain->setMaterialTexture(0, _scene->getVideoDriver()->getTexture("./media/terrain-texture.jpg"));
	terrain->setMaterialTexture(1, _scene->getVideoDriver()->getTexture("./media/detailmap3.jpg"));
	terrain->setMaterialType(video::EMT_DETAIL_MAP);
	terrain->scaleTexture(1.0f, 40.0f);

	scene::CDynamicMeshBuffer* buffer = new scene::CDynamicMeshBuffer(video::EVT_2TCOORDS, video::EIT_16BIT);
	terrain->getMeshBufferForLOD(*buffer, 0);
	video::S3DVertex2TCoords* data = (video::S3DVertex2TCoords*)buffer->getVertexBuffer().getData();
	_vertexC = buffer->getVertexBuffer().size();
	unsigned w = sqrt(_vertexC);
	_heightMap.reset(new float[_vertexC]);
	assert((((w-1) & ((w-1)-1)) == 0) && "heightmap size must be 2^n+1");
	float min = data[0].Pos.Y;
	float max = data[0].Pos.Y;
	for(unsigned y = 0; y<w; y++)
	{
		for(unsigned x = 0; x<w; x++)
		{
			// data is stored top-to-bottom, right-to-left
			core::vector3df p = data[x*w + y].Pos;
			// need to remap it back to left-to-right, top-to-bottom
			(_heightMap.get())[x + y*w] = p.Y;

			if(p.Y > max)
				max = p.Y;
			if(p.Y < min)
				min = p.Y;
		}
	}
}

float WorldMap::getPatchSize()
{
	return _patchSize;
}

float WorldMap::getHeightScale()
{
	return _heightScale;
}

float* WorldMap::getHeightMap()
{
	return _heightMap.get();
}

unsigned WorldMap::getVertexCount()
{
	return _vertexC;
}

////////////////////////////////////////////////////////////


World::World(WorldMap& wm): _map{wm}
{
	/* TODO .. just for fun / test
	_entManager.registerComponentType<BodyComponent>(ComponentType::Body);
	_entManager.registerComponentType<SphereGraphicsComponent>(ComponentType::GraphicsSphere);
	_entManager.registerComponentType<MeshGraphicsComponent>(ComponentType::GraphicsMesh);
	_entManager.registerComponentType<CollisionComponent>(ComponentType::Collision);
	_entManager.registerComponentType<WizardComponent>(ComponentType::Wizard);
	*/
}

ID World::createEntity(ID hintEntID)
{
	// TODO hint entID
	// TODO check for existing entities with same ID
	return _entManager.createEntity();
}

Entity& World::createAndGetEntity(ID hintEntID = 0)
{
	// TODO hint entID
	// TODO check for existing entities with same ID
	return _entManager.createAndGetEntity();
}

void World::removeEntity(ID entID)
{
	cout << "REMOVING ENTITY WITH ID " << entID << endl;
	_entManager.removeEntity(entID);
}

Entity* World::getEntity(ID entID)
{
	return _entManager.getEntity(entID);
}

ID World::createCharacter(vec3f position)
{
	ID eID = createEntity();
	Entity& e = *getEntity(eID);

	e.addComponent<BodyComponent>(position);
	/*TODO
	e.setBodyComponent(make_shared<BodyComponent>(e, position));
	e.setGraphicsComponent(make_shared<MeshGraphicsComponent>(e, "ninja.b3d", true, vec3f(0), vec3f(0,90,0), vec3f(0.2)));
	e.setCollisionComponent(make_shared<CollisionComponent>(e, 0.4, 1, vec3f(0, -0.9, 0)));
	e.setWizardComponent(make_shared<WizardComponent>(e));
	*/
	return eID;
}

WorldMap& World::getMap()
{
	return _map;
}

IterateOnly<SolidVector<Entity>> World::getEntities()
{
	return _entManager.getEntities();
}
