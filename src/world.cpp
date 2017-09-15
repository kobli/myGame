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

// // // // // // // // // // // // // // // // // // // //

ParticleSystemGraphicsComponent::ParticleSystemGraphicsComponent(ID parentEntID, ID effectID, vec3f posOffset, vec3f rotOffset, vec3f scale)
	: GraphicsComponent{parentEntID, ComponentType::GraphicsParticleSystem, posOffset, rotOffset, scale}, _effectID{effectID}
{}

void ParticleSystemGraphicsComponent::serDes(SerDesBase& s)
{
	s.serDes(*static_cast<GraphicsComponent*>(this));
	s.serDes(*this);
}

ID ParticleSystemGraphicsComponent::getEffectID()
{
	return _effectID;
}

////////////////////////////////////////////////////////////

CollisionComponent::CollisionComponent(ID parentEntID, float radius,
	 	float height, vec3f posOffset, float mass, bool kinematic, float gravity)
	: ObservableComponentBase(parentEntID, ComponentType::Collision)
		, _radius{radius}, _height{height}, _posOff{posOffset}
		, _mass{mass}, _kinematic{kinematic}, _gravity{gravity}
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

float CollisionComponent::getMass()
{
	return _mass;
}

float CollisionComponent::getGravity()
{
	return _gravity;
}

////////////////////////////////////////////////////////////

WizardComponent::WizardComponent(ID parentEntID):
 	ObservableComponentBase(parentEntID, ComponentType::Wizard), _availableBodyC{0}, _totalBodyC{0}
{
}

void WizardComponent::serDes(SerDesBase& s)
{
	s.serDes(*this);
}

void WizardComponent::setCurrentJobStatus(std::string job, float duration, float progress)
{
	bool changed = _currentJob != job || _currentJobDuration != duration || _currentJobProgress != progress;
	_currentJob = job;
	_currentJobDuration = duration;
	_currentJobProgress = progress;
	if(changed)
		notifyObservers();
}

void WizardComponent::setSpellInHandsData(float power, float radius, float speed)
{
	bool changed = _spellInHandsPower != power || _spellInHandsRadius != radius || _spellInHandsSpeed != speed;
	_spellInHandsPower = power;
	_spellInHandsRadius = radius;
	_spellInHandsSpeed = speed;
	if(changed)
		notifyObservers();
}

void WizardComponent::setBodyStatus(unsigned available, unsigned total)
{
	bool changed = _availableBodyC != available || _totalBodyC != total;
	_availableBodyC = available;
	_totalBodyC = total;
	if(changed)
		notifyObservers();
}

std::string WizardComponent::getCurrentJob()
{
	return _currentJob;
}

float WizardComponent::getCurrentJobDuration()
{
	return _currentJobDuration;
}

float WizardComponent::getCurrentJobProgress()
{
	return _currentJobProgress;
}

float WizardComponent::getSpellInHandsPower()
{
	return _spellInHandsPower;
}

float WizardComponent::getSpellInHandsRadius()
{
	return _spellInHandsRadius;
}

float WizardComponent::getSpellInHandsSpeed()
{
	return _spellInHandsSpeed;
}

unsigned WizardComponent::getAvailableBodyC()
{
	return _availableBodyC;
}

unsigned WizardComponent::getTotalBodyC()
{
	return _totalBodyC;
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

void AttributeStoreComponent::setOrAddAttribute(std::string key, float value)
{
	if(!hasAttribute(key))
		addAttribute(key, value);
	else
		setAttribute(key, value);
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
		float mul = 1;
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

World::World(const WorldMap& wm): _map{wm}, _entManager{ObjStaticID::FIRSTFREE}
{
	_entManager.registerComponentType<BodyComponent>(ComponentType::Body);
	_entManager.registerComponentType<SphereGraphicsComponent>(ComponentType::GraphicsSphere);
	_entManager.registerComponentType<MeshGraphicsComponent>(ComponentType::GraphicsMesh);
	_entManager.registerComponentType<ParticleSystemGraphicsComponent>(ComponentType::GraphicsParticleSystem);
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
	e.addComponent<CollisionComponent>(0.4, 1, vec3f(0, -0.9, 0), 80);
	e.addComponent<WizardComponent>();
	e.addComponent<AttributeStoreComponent>();
	return eID;
}

const WorldMap& World::getMap()
{
	return _map;
}

IterateOnly<SolidVector<Entity,ID,NULLID>> World::getEntities()
{
	return _entManager.getEntities();
}
