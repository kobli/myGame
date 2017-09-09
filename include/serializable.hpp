#ifndef SERIALIZABLE_HPP_16_12_02_22_22_57
#define SERIALIZABLE_HPP_16_12_02_22_22_57 

class BodyComponent;
class GraphicsComponent;
class SphereGraphicsComponent;
class MeshGraphicsComponent;
class ParticleSystemGraphicsComponent;
class CollisionComponent;
class AttributeStoreComponent;
class WizardComponent;

class KeyValueStore;
class WorldMap;


class SerDesBase
{
	public: 
		// overloads for all Serializables
		virtual void serDes(BodyComponent&) = 0;
		virtual void serDes(GraphicsComponent&) = 0;
		virtual void serDes(SphereGraphicsComponent&) = 0;
		virtual void serDes(MeshGraphicsComponent&) = 0;
		virtual void serDes(ParticleSystemGraphicsComponent&) = 0;
		virtual void serDes(CollisionComponent&) = 0;
		virtual void serDes(AttributeStoreComponent&) = 0;
		virtual void serDes(WizardComponent&) = 0;

		virtual void serDes(KeyValueStore&) = 0;
		virtual void serDes(WorldMap&) = 0;
};

class Serializable
{
	public:
		virtual void serDes(SerDesBase& s) = 0;
};

#endif /* SERIALIZABLE_HPP_16_12_02_22_22_57 */
