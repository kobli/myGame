#include <world.hpp>
#include <cassert>

EntityEvent::EntityEvent(u32 entityID, ComponentType componentModifiedType
		, WorldEntityComponent* componentModified, bool created, bool destroyed)
	: _entityID{entityID}, _componentModifiedType{componentModifiedType}
	, _componentModified{componentModified}, _created{created}, _destroyed{destroyed}
{}

////////////////////////////////////////////////////////////

WorldMap::WorldMap(float patchSize, scene::ISceneManager* scene): _patchSize{patchSize}, _scene{scene}, _heightScale{0.05}
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
			core::vector3df p = data[x*w + y].Pos; // data is stored top-to-bottom, right-to-left
			(_heightMap.get())[x + y*w] = p.Y; // need to remap it back to left-to-right, top-to-bottom

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

WorldEntityComponent::WorldEntityComponent(WorldEntity& parent, ComponentType compType)
	//TODO /this/ should not be passed, because the object is invalid both during construction and destruction
	: Observable(EntityEvent(parent.getID(), compType, this,  true)
			, EntityEvent(parent.getID(), compType, this, false, true))
		, _parent{parent}, _compType{compType}
{}

void WorldEntityComponent::notifyObservers()
{
	EntityEvent m;
	m._entityID =  _parent.getID();
	m._componentModifiedType = _compType;
	m._componentModified = this;
	Observable<EntityEvent>::notifyObservers(m);
}

////////////////////////////////////////////////////////////

WorldEntity::WorldEntity(World& w, u32 ID)
	: Observabler(EntityEvent(ID, ComponentType::None, nullptr, true)
			, EntityEvent(ID, ComponentType::None, nullptr, false, true)), _world{w}, _ID{ID}
{}

u32 WorldEntity::getID()
{
	return _ID;
}

shared_ptr<BodyComponent> WorldEntity::setBodyComponent(shared_ptr<BodyComponent> bc)
{
	_body.swap(bc);
	if(_body)
		observe(*_body);
	return _body;
}

shared_ptr<BodyComponent> WorldEntity::getBodyComponent()
{
	return _body;
}

shared_ptr<GraphicsComponent> WorldEntity::setGraphicsComponent(shared_ptr<GraphicsComponent> gc)
{
	_graphics.swap(gc);
	if(_graphics)
		observe(*_graphics);
	return _graphics;
}

shared_ptr<GraphicsComponent> WorldEntity::getGraphicsComponent()
{
	return _graphics;
}

shared_ptr<CollisionComponent> WorldEntity::setCollisionComponent(shared_ptr<CollisionComponent> cc)
{
	_collision.swap(cc);
	if(_collision)
		observe(*_collision);
	return _collision;
}

shared_ptr<CollisionComponent> WorldEntity::getCollisionComponent()
{
	return _collision;
}

shared_ptr<WizardComponent> WorldEntity::setWizardComponent(shared_ptr<WizardComponent> cc)
{
	_wizard.swap(cc);
	if(_wizard)
		observe(*_wizard);
	return _wizard;

}

shared_ptr<WizardComponent> WorldEntity::getWizardComponent()
{
	return _wizard;
}

////////////////////////////////////////////////////////////

BodyComponent::BodyComponent(WorldEntity& parent, vec3f position, quaternion rotation, vec3f velocity)
	: WorldEntityComponent{parent, ComponentType::Body}, _position{position}, _rotation{rotation}, _velocity{velocity}
	, _strafeDir{vec2f(0,0)}, _strafeSpeed{120}, _rotDir{0}, _posRotChanged{false}
{}
	
/*
void BodyComponent::update(float timeDelta)
{
	vec3f newRot = _rotation,
				newPos = _position;
	newRot.Y = fmod(_rotation.Y+(_rotSpeed*_rotDir*timeDelta), 360);
	setRotation(newRot);
	auto cc = _parent.getCollisionComponent();
	if(cc)
		newPos = cc->getCollisionResultPosition(timeDelta);
	else
		newPos += getTotalVelocity()*timeDelta;
	setPosition(newPos);
}
*/

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

/*
vec3f BodyComponent::getTotalVelocity() const
{
	vec3f vel = _velocity;
	if(_strafeDir.getLength() != 0)
	{
		vec3f strDir{_strafeDir.X, 0, _strafeDir.Y};
		strDir.rotateYZBy(-_rotation.X);
		strDir.rotateXZBy(-_rotation.Y);
		strDir.rotateXYBy(-_rotation.Z);
		strDir.normalize();
		strDir.Y = 0;
		vel += strDir*_strafeSpeed;
	}
	return vel;
}
*/

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

GraphicsComponent::GraphicsComponent(ComponentType t, WorldEntity& parent
		, vec3f posOffset, vec3f rotOffset, vec3f scale)
	: WorldEntityComponent(parent, t)
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

SphereGraphicsComponent::SphereGraphicsComponent(WorldEntity& parent, float radius, vec3f posOffset, vec3f rotOffset, vec3f scale)
	: GraphicsComponent{ComponentType::GraphicsSphere, parent, posOffset, rotOffset, scale}, _radius{radius}
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

MeshGraphicsComponent::MeshGraphicsComponent(WorldEntity& parent, string fileName, bool animated, vec3f posOffset, vec3f rotOffset, vec3f scale)
	: GraphicsComponent{ComponentType::GraphicsMesh, parent, posOffset, rotOffset, scale}, _fileName{fileName}, _animated{animated}
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

CollisionComponent::CollisionComponent(WorldEntity& parent, float radius, float height, vec3f posOffset, bool kinematic)
	: WorldEntityComponent(parent, ComponentType::Collision), _radius{radius}, _height{height}, _posOff{posOffset}, _kinematic{kinematic}
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

WizardComponent::WizardComponent(WorldEntity& parent): WorldEntityComponent(parent, ComponentType::Wizard)
{
}

void WizardComponent::serDes(SerDesBase&)
{
	//s.serDes(*this);
}

////////////////////////////////////////////////////////////

World::World(WorldMap& wm): Observabler(EntityEvent(0, ComponentType::None, nullptr,  true)
			, EntityEvent(0, ComponentType::None, nullptr, false, true)), _map{wm}, _nextEntityID{1}
{}

WorldEntity& World::createEntity(u32 ID)
{
	// TODO check for existing entities with same ID
	u32 nID = ID;
	if(nID == 0)
	{
		nID = _nextEntityID;
		_nextEntityID++;	
	}
	_entities.emplace_back(*this, nID);
	observe(_entities.back());
	return _entities.back();
}

void World::removeEntity(WorldEntity& e)
{
	cout << "REMOVING ENTITY WITH ID " << e.getID() << endl;
	auto it = _entities.begin();
	while(&*it != &e && it != _entities.end())
		it++;
	_entities.erase(it);
}

void World::removeEntity(u32 ID)
{
	removeEntity(*getEntityByID(ID));
}

WorldEntity& World::createCharacter(vec3f position)
{
	WorldEntity& e = createEntity();
	e.setBodyComponent(make_shared<BodyComponent>(e, position));
	/*
	e.setGraphicsComponent(make_shared<SphereGraphicsComponent>(e, 1));
	e.setCollisionComponent(make_shared<CollisionComponent>(e, 1, 0));
	*/
	e.setGraphicsComponent(make_shared<MeshGraphicsComponent>(e, "ninja.b3d", true, vec3f(0), vec3f(0,90,0), vec3f(0.2)));
	e.setCollisionComponent(make_shared<CollisionComponent>(e, 0.4, 1, vec3f(0, -0.9, 0)));
	e.setWizardComponent(make_shared<WizardComponent>(e));
	return e;
}

WorldMap& World::getMap()
{
	return _map;
}

std::list<WorldEntity>& World::getEntities()
{
	return _entities;
}

WorldEntity* World::getEntityByID(u32 ID)
{
	for(auto& e: _entities)
		if(e.getID() == ID)
			return &e;
	return nullptr;
}
