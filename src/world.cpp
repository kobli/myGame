#include <world.hpp>
#include <cassert>

EntityEvent::EntityEvent(u32 entityID, ComponentType componentModifiedType
		, WorldEntityComponent* componentModified, bool created, bool destroyed)
	: _entityID{entityID}, _componentModifiedType{componentModifiedType}
	, _componentModified{componentModified}, _created{created}, _destroyed{destroyed}
{}

////////////////////////////////////////////////////////////

WorldMap::WorldMap(unsigned patchSize, scene::ISceneManager* scene): _patchSize{patchSize}, _scene{scene}, _heightScale{0.1}
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

bool WorldEntity::isEmpty()
{
	return !(_body || _collision || _graphics || _input);
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

shared_ptr<InputComponent> WorldEntity::setInputComponent(shared_ptr<InputComponent> ic)
{
	_input.swap(ic);
	if(_input)
		observe(*_input);
	return _input;
}

shared_ptr<InputComponent> WorldEntity::getInputComponent()
{
	return _input;
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

BodyComponent::BodyComponent(WorldEntity& parent, vec3f position, vec3f rotation, vec3f velocity)
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

vec3f BodyComponent::getRotation() const
{
	return _rotation;
}

vec3f BodyComponent::getVelocity() const
{
	return _velocity;
}

i32 BodyComponent::getRotDir() const
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

void BodyComponent::setRotation(vec3f r)
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

void BodyComponent::setRotDir(i32 rotDir)
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

CollisionComponent::CollisionComponent(WorldEntity& parent, float radius, float height)
	: WorldEntityComponent(parent, ComponentType::Collision), _radius{radius}, _height{height}
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

void CollisionComponent::serDes(SerDesBase& s)
{
	s.serDes(*this);
}

////////////////////////////////////////////////////////////

InputComponent::InputComponent(WorldEntity& parent): WorldEntityComponent(parent, ComponentType::Input)
{}

void InputComponent::handleCommand(Command& c)
{
	switch(c._type)
	{
		case Command::Type::STRAFE_DIR_SET:
			{
				auto b = _parent.getBodyComponent();
				if(b)
					b->setStrafeDir(c._vec2f);
				break;
			}
		case Command::Type::ROT_DIR_SET:
			{
				auto b = _parent.getBodyComponent();
				if(b)
					b->setRotDir(c._i32);
				break;
			}
		case Command::Type::STR:
			{
				if(c._str.find("spell_") == 0)
				{
					auto w = _parent.getWizardComponent();
					if(w)
						w->cast(c._str);
				}
				break;
			}
		default:
			cerr << "unknown command type to handle\n";
	}
}

void InputComponent::serDes(SerDesBase& s)
{
	s.serDes(*this);
}

////////////////////////////////////////////////////////////

std::shared_ptr<lua_State> WizardComponent::_luaState(nullptr);
WizardComponent::WizardComponent(WorldEntity& parent): WorldEntityComponent(parent, ComponentType::Wizard)
{
	if(true)//TODO _luaState == nullptr)
	{
		_luaState.reset(luaL_newstate(), [](lua_State* luaState){ lua_close(luaState); });
		luaL_openlibs(_luaState.get());
		luaL_dofile(_luaState.get(), "lua/spellSystem.lua");

		auto callLaunchSpell = [](lua_State* s)->int {
			int argc = lua_gettop(s);
			if(argc != 3)
			{
				std::cerr << "callLaunchSpell: wrong number of arguments\n";
				return 0;		
			}
			u32 ID = lua_tointeger(s, 1);
			float sRadius = lua_tonumber(s, 2);
			float sSpeed = lua_tonumber(s, 3);
			World* w = (World*)lua_touserdata(s, lua_upvalueindex(1));
			WorldEntity* e = w->getEntityByID(ID);
			if(e == nullptr)
			{
				std::cerr << "callLaunchSpell: invalid entity ID\n";
				return 0;		
			}
			auto wc = e->getWizardComponent();
			if(!wc)
			{
				std::cerr << "callLaunchSpell: wizard component missing\n";
				return 0;		
			}
			u32 sID = wc->launchSpell(sRadius, sSpeed);
			lua_pushinteger(s, sID);
			return 1;
		};
		lua_pushlightuserdata(_luaState.get(), &_parent._world);
		lua_pushcclosure(_luaState.get(), callLaunchSpell, 1);
		lua_setglobal(_luaState.get(), "wizardLaunchSpell");
	}
	if(_luaState != nullptr)
	{
		lua_getglobal(_luaState.get(), "addWizard");
		lua_pushinteger(_luaState.get(), _parent.getID());
		if(lua_pcall(_luaState.get(), 1, 0, 0) != 0)
		{
			cerr << "something went wrong with addWizard: " << lua_tostring(_luaState.get(), -1) << endl;
			lua_pop(_luaState.get(), 1);
		}
	}
}

WizardComponent::~WizardComponent()
{
	if(_luaState != nullptr)
	{
		lua_getglobal(_luaState.get(), "removeWizard");
		lua_pushinteger(_luaState.get(), _parent.getID());
		if(lua_pcall(_luaState.get(), 1, 0, 0) != 0)
		{
			cerr << "something went wrong with removeWizard: " << lua_tostring(_luaState.get(), -1) << endl;
			lua_pop(_luaState.get(), 1);
		}
	}
}

void WizardComponent::update(float timeDelta)
{
	if(_luaState != nullptr)
	{
		lua_getglobal(_luaState.get(), "update");
		lua_pushnumber(_luaState.get(), timeDelta);
		if(lua_pcall(_luaState.get(), 1, 0, 0) != 0)
		{
			cerr << "something went wrong with spell update: " << lua_tostring(_luaState.get(), -1) << endl;
			lua_pop(_luaState.get(), 1);
		}
	}
}

void WizardComponent::cast(std::string& incantation)
{
	lua_getglobal(_luaState.get(), "handleIncantation");
	lua_pushinteger(_luaState.get(), _parent.getID());
	lua_pushstring(_luaState.get(), incantation.c_str());
	if(lua_pcall(_luaState.get(), 2, 0, 0) != 0)
	{
		cerr << "something went wrong with handleIncantation: " << lua_tostring(_luaState.get(), -1) << endl;
		lua_pop(_luaState.get(), 1);
	}
}
	
void WizardComponent::serDes(SerDesBase& s)
{
	//s.serDes(*this);
}

u32 WizardComponent::launchSpell(float radius, float speed)
{
	auto myBody = _parent.getBodyComponent();
	if(myBody == nullptr)
		return 0; //TODO fail in a better way

	WorldEntity& e = _parent._world.createEntity();
	vec3f pos = myBody->getPosition() + vec3f(0,70,0);
	e.setBodyComponent(make_shared<BodyComponent>(e, pos)); // parentEntity, TODO position, rotation, velocity

	// TODO e.setCollisionComponent():
	
	auto gc = make_shared<SphereGraphicsComponent>(e, radius);
	e.setGraphicsComponent(gc);

	return e.getID();
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
	auto it = _entities.begin();
	while(&*it != &e && it != _entities.end())
		it++;
	_entities.erase(it);
}

WorldEntity& World::createCharacter(vec3f position)
{
	WorldEntity& e = createEntity();
	e.setBodyComponent(make_shared<BodyComponent>(e, position));
	//scene::IAnimatedMeshSceneNode* sceneNode = _smgr->addAnimatedMeshSceneNode(_smgr->getMesh("./media/ninja.b3d"));
	//e.setGraphicsComponent(make_shared<GraphicsComponent>(e, sceneNode, vec3f(0), vec3f(0,90,0), vec3f(17), true));
	e.setGraphicsComponent(make_shared<MeshGraphicsComponent>(e, "ninja.b3d", true, vec3f(0), vec3f(0,90,0), vec3f(0.4)));
	//e.setGraphicsComponent(make_shared<SphereGraphicsComponent>(e, 1));
	//vec3f colliderSize = sceneNode->getTransformedBoundingBox().MaxEdge - sceneNode->getTransformedBoundingBox().MinEdge;
	//colliderSize /= 2;
	e.setCollisionComponent(make_shared<CollisionComponent>(e, 0.4, 1.8));
	e.setInputComponent(make_shared<InputComponent>(e));
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
