#ifndef SERIALIZABLE_HPP_16_12_02_22_22_57
#define SERIALIZABLE_HPP_16_12_02_22_22_57 

class Serializable;

class BodyComponent;
class GraphicsComponent;
class SphereGraphicsComponent;
class CollisionComponent;
class InputComponent;

class SerializerBase
{
	public: 
		// overloads for various Serializables
		virtual void serialize(BodyComponent&) = 0;
		virtual void serialize(GraphicsComponent&) = 0;
		virtual void serialize(SphereGraphicsComponent&) = 0;
		virtual void serialize(CollisionComponent&) = 0;
		virtual void serialize(InputComponent&) = 0;
};

////////////////////////////////////////////////////////////

class DeserializerBase
{
	public: 
		// overloads for various Serializables
		virtual void deserialize(BodyComponent&) = 0;
		virtual void deserialize(GraphicsComponent&) = 0;
		virtual void deserialize(SphereGraphicsComponent&) = 0;
		virtual void deserialize(CollisionComponent&) = 0;
		virtual void deserialize(InputComponent&) = 0;
};

////////////////////////////////////////////////////////////

class Serializable
{
	public:
		virtual void serialize(SerializerBase& s) = 0; // s.serialize(*this);
		virtual void deserialize(DeserializerBase& s) = 0; // s.deserialize(*this);
};

#endif /* SERIALIZABLE_HPP_16_12_02_22_22_57 */
