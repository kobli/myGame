#ifndef SERIALIZABLE_HPP_16_12_02_22_22_57
#define SERIALIZABLE_HPP_16_12_02_22_22_57 

class BodyComponent;
class GraphicsComponent;
class SphereGraphicsComponent;
class MeshGraphicsComponent;
class CollisionComponent;
class AttributeStoreComponent;

class KeyValueStore;


class SerDesBase
{
	public: 
		// overloads for all Serializables
		virtual void serDes(BodyComponent&) = 0;
		virtual void serDes(GraphicsComponent&) = 0;
		virtual void serDes(SphereGraphicsComponent&) = 0;
		virtual void serDes(MeshGraphicsComponent&) = 0;
		virtual void serDes(CollisionComponent&) = 0;
		virtual void serDes(AttributeStoreComponent&) = 0;

		virtual void serDes(KeyValueStore&) = 0;
};

class Serializable
{
	public:
		virtual void serDes(SerDesBase& s) = 0;
};

#endif /* SERIALIZABLE_HPP_16_12_02_22_22_57 */
