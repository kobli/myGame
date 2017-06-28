#include <world.hpp>
#include <cassert>

ObservableComponentBase::ObservableComponentBase(ID parentEntID, ComponentType realCompType)
 	: Observable<EntityEvent>{EntityEvent{parentEntID, realCompType, true}, EntityEvent{parentEntID, realCompType, false, true}},
	_updMsg{parentEntID, realCompType}
{
}

void ObservableComponentBase::notifyObservers()
{
	broadcastMsg(_updMsg);
}

////////////////////////////////////////////////////////////

BodyComponent::BodyComponent(ID parentEntID, vec3f position, quaternion rotation, vec3f velocity)
	: ObservableComponentBase{parentEntID, ComponentType::Body}, _position{position}, _rotation{rotation}, _velocity{velocity}
	, _strafeDir{vec2f(0,0)}, _strafeSpeed{120}, _rotDir{0}
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
	notifyObservers();
}

void BodyComponent::setRotation(quaternion r)
{
	if(_rotation == r)
		return;
	_rotation = r;
	notifyObservers();
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
	//s.serDes(*this); currently not serialized
}

////////////////////////////////////////////////////////////

AttributeAffector::AttributeAffector(std::string attribute, ModifierType modifierType
				, float modifierValue, bool permanent, float period): 
	_attribute{attribute}, _modifierType{modifierType}, _modifierValue{modifierValue},
	_permanent{permanent}, _period{period} {
	}

std::string AttributeAffector::getAffectedAttribute() {
	return _attribute;
}

AttributeAffector::ModifierType AttributeAffector::getModifierType() {
	return _modifierType;
}

float AttributeAffector::getModifierValue() {
	return _modifierValue;
}

bool AttributeAffector::isPermanent() {
	return _permanent;
}

float AttributeAffector::getPeriod() {
	return _period;
}

// // // // // // // // // // // // // // // // // // // // 

AttributeStoreComponent::AttributeStoreComponent(ID parentEntID): ObservableComponentBase(parentEntID, ComponentType::AttributeStore)
{}

void AttributeStoreComponent::addAttribute(std::string key, float value)
{
	addPair(key, value);
	notifyObservers();
}

bool AttributeStoreComponent::hasAttribute(std::string key)
{
	return hasKey(key);
}

float AttributeStoreComponent::getAttribute(std::string key)
{
	return getValue(key);
}

void AttributeStoreComponent::setAttribute(std::string key, float value)
{
	setValue(key, value);
	notifyObservers();
}

ID AttributeStoreComponent::addAttributeAffector(AttributeAffector aa)
{
	if(aa.isPermanent()) {
		assert(hasAttribute(aa.getAffectedAttribute()));
		float attr = getAttribute(aa.getAffectedAttribute());
		if(aa.getModifierType() == AttributeAffector::ModifierType::Mul)
			attr *= aa.getModifierValue();
		else if(aa.getModifierType() == AttributeAffector::ModifierType::Add)
			attr += aa.getModifierValue();
		attr = std::max(attr, 0.f);
		setAttribute(aa.getAffectedAttribute(), attr);
		return NULLID;
	}
	else
		return _attributeAffectors.insert(std::move(aa));
}

bool AttributeStoreComponent::removeAttributeAffector(ID affectorID)
{
	if(_attributeAffectors.indexValid(affectorID)) {
		_attributeAffectors.remove(affectorID);
		notifyObservers();
		return true;
	}
	else
		return false;
}

float AttributeStoreComponent::getAttributeAffected(std::string key)
{
	if(!hasAttribute(key))
		return -1;
	else {
		float base = getAttribute(key);
		float add = 0;
		float mul = 0;
		for(auto& a : _attributeAffectors)
			if(a.getAffectedAttribute() == key) {
				if(a.getModifierType() == AttributeAffector::ModifierType::Add)
					add += a.getModifierValue();
				else if(a.getModifierType() == AttributeAffector::ModifierType::Mul)
					mul += a.getModifierValue();
			}
		return std::max(base*mul + add, 0.f);
	}
}

void AttributeStoreComponent::serDes(SerDesBase& s)
{
	s.serDes(*static_cast<KeyValueStore*>(this));
	s.serDes(*this);
}

////////////////////////////////////////////////////////////

WorldMap::WorldMap(float patchSize, scene::ISceneManager* scene)
	: _patchSize{patchSize}, _scene{scene}, _heightScale{0.05}
{
	scene::ITerrainSceneNode* terrain = _scene->addTerrainSceneNode(
		"./media/terrain-heightmap.bmp",
		nullptr,					// parent node
		ObjStaticID::Map,			// node id
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


World::World(WorldMap& wm): _map{wm}, _entManager{ObjStaticID::FIRSTFREE}
{
	_entManager.registerComponentType<BodyComponent>(ComponentType::Body);
	_entManager.registerComponentType<SphereGraphicsComponent>(ComponentType::GraphicsSphere);
	_entManager.registerComponentType<MeshGraphicsComponent>(ComponentType::GraphicsMesh);
	_entManager.registerComponentType<CollisionComponent>(ComponentType::Collision);
	_entManager.registerComponentType<WizardComponent>(ComponentType::Wizard);
	_entManager.registerComponentType<AttributeStoreComponent>(ComponentType::AttributeStore);
	_entManager.addObserver(*this);
}

ID World::createEntity(ID hintEntID)
{
	ID eID = _entManager.createEntity(hintEntID);
	assert(hintEntID == NULLID || eID == hintEntID);
	return eID;
}

Entity& World::createAndGetEntity(ID hintEntID)
{
	auto& e = _entManager.createAndGetEntity(hintEntID);
	assert(hintEntID == NULLID || e.getID() == hintEntID);
	return e;
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
	e.addComponent<MeshGraphicsComponent>("ninja.b3d", true, vec3f(0), vec3f(0,90,0), vec3f(0.2));
	e.addComponent<CollisionComponent>(0.4, 1, vec3f(0, -0.9, 0));
	e.addComponent<WizardComponent>();
	e.addComponent<AttributeStoreComponent>();
	AttributeStoreComponent& attStore = *e.getComponent<AttributeStoreComponent>();
	attStore.addAttribute("health", 100);
	attStore.addAttribute("max-health", 100);
	return eID;
}

WorldMap& World::getMap()
{
	return _map;
}

IterateOnly<SolidVector<Entity,ID,NULLID>> World::getEntities()
{
	return _entManager.getEntities();
}
