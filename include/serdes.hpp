#include <world.hpp>

#ifndef SERDES_HPP_16_12_02_11_12_45
#define SERDES_HPP_16_12_02_11_12_45 
template <typename T>
class Serializer: public SerializerBase
{
	public:
		Serializer(Serializable& s): _s{s}
		{}
		SerializerBase& operator >>(T& t)
		{
			_t = &t;
			_s.serialize(*this);
			return *this;
		}
		virtual void serialize(BodyComponent& b)
		{
			if(!_t)
				return;	
			(*_t) << b.getStrafeDir() << b.getRotDir() << b.getPosition() << b.getRotation() << b.getVelocity();
		}
		virtual void serialize(GraphicsComponent& g)
		{
			using namespace irr::scene;
			if(!_t)
				return;
			(*_t) << g.getPosOffset() << g.getRotOffset();
			/*
			ESCENE_NODE_TYPE snt = g.getSceneNode()->getType();
			(*_t) << snt;
			switch(snt)
			{
				case ESCENE_NODE_TYPE::ESNT_MESH:
					break;
				case ESCENE_NODE_TYPE::ESNT_SPHERE:
					g.getSceneNode()->
					static_cast<irr::ISphereSceneNode> g.getSceneNode()->radius
					break;
				default:
					break;
			}
			*/
		}
		virtual void serialize(SphereGraphicsComponent& g)
		{
			if(!_t)
				return;
			(*_t) << g.getRadius();
		}
		virtual void serialize(CollisionComponent& c)
		{
			if(!_t)
				return;
			(*_t) << c.getColliderRadius();
		}
		virtual void serialize(InputComponent&)
		{

		}

	private:
		Serializable& _s;
		T* _t;
};

////////////////////////////////////////////////////////////

template <typename T>
class Deserializer: public DeserializerBase
{
	public:
		Deserializer(Serializable& s): _s{s}
		{}
		DeserializerBase& operator <<(T& t)
		{
			_t = &t;
			_s.deserialize(*this);
			return *this;
		}
		virtual void deserialize(BodyComponent& b)
		{
			if(!_t)
				return;
			vec3f v3f;
			vec2f v2f;
			i32 ii32;
			(*_t) >> v2f;
			b.setStrafeDir(v2f);
			(*_t) >> ii32;
			b.setRotDir(ii32);	
			(*_t) >> v3f;
			b.setPosition(v3f);
			(*_t) >> v3f;
			b.setRotation(v3f);
			(*_t) >> v3f;
			b.setVelocity(v3f);
		}
		virtual void deserialize(GraphicsComponent& g)
		{
			if(!_t)
				return;
			vec3f v3f;
			(*_t) >> v3f;
			g.setPosOffset(v3f);
			(*_t) >> v3f;
		 	g.setRotOffset(v3f);
			//irr::scene::ESCENE_NODE_TYPE snt;
			//(*_t) >> snt;
			//std::cout << "deserialized snt: " << snt<< std::endl;
		}
		virtual void deserialize(SphereGraphicsComponent& g)
		{
			if(!_t)
				return;
			float f;
			(*_t) >> f;
			g.setRadius(f);
		}
		virtual void deserialize(CollisionComponent& c)
		{
			if(!_t)
				return;
			vec3f v3f;
			(*_t) >> v3f;
			c.setColliderRadius(v3f);
		}
		virtual void deserialize(InputComponent&)
		{

		}

	private:
		Serializable& _s;
		T* _t;
};

template <typename T>
T& operator <<(T& t, Serializer<T>& m)
{
	m >> t;
	return t;
}
template <typename T>
T& operator >>(T& t, Deserializer<T>& m)
{
	m << t;
	return t;
}

template <typename T>
T& operator <<(T& t, Serializer<T>&& m)
{
	return operator<<(t,m);
}
template <typename T>
T& operator >>(T& t, Deserializer<T>&& m)
{
	return operator>>(t,m);
}

#endif /* SERDES_HPP_16_12_02_11_12_45 */
