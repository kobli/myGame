#include <world.hpp>

#ifndef SERDES_HPP_16_12_02_11_12_45
#define SERDES_HPP_16_12_02_11_12_45 
template <typename T>
class Serializer: public SerDesBase
{
	public:
		Serializer(Serializable& s): _s{s}
		{}
		Serializer<T>& operator >>(T& t)
		{
			_t = &t;
			_s.serDes(*this);
			return *this;
		}
		template <typename DT>
			void operator&(DT& d)
			{
				*_t << d;// << " | ";
			}
		virtual void serDes(BodyComponent& s) {
			s.doSerDes(*this);
		}
		virtual void serDes(GraphicsComponent& s) {
			s.doSerDes(*this);
		}
		virtual void serDes(SphereGraphicsComponent& s) {
			s.doSerDes(*this);
		}
		virtual void serDes(MeshGraphicsComponent& s) {
			s.doSerDes(*this);
		}
		virtual void serDes(ParticleSystemGraphicsComponent& s) {
			s.doSerDes(*this);
		}
		virtual void serDes(CollisionComponent& s) {
			s.doSerDes(*this);
		}
		virtual void serDes(AttributeStoreComponent& s) {
			s.doSerDes(*this);
		}

		virtual void serDes(KeyValueStore& s) {
			s.doSerDes(*this);
		}

	private:
		Serializable& _s;
		T* _t;
};

////////////////////////////////////////////////////////////

template <typename T>
class Deserializer: public SerDesBase
{
	public:
		Deserializer(Serializable& s): _s{s}
		{}
		Deserializer<T>& operator <<(T& t)
		{
			_t = &t;
			_s.serDes(*this);
			return *this;
		}
		template <typename DT>
			void operator&(DT& d)
			{
				*_t >> d;
			}
		virtual void serDes(BodyComponent& s) {
			s.doSerDes(*this);
		}
		virtual void serDes(GraphicsComponent& s) {
			s.doSerDes(*this);
		}
		virtual void serDes(SphereGraphicsComponent& s) {
			s.doSerDes(*this);
		}
		virtual void serDes(MeshGraphicsComponent& s) {
			s.doSerDes(*this);
		}
		virtual void serDes(ParticleSystemGraphicsComponent& s) {
			s.doSerDes(*this);
		}
		virtual void serDes(CollisionComponent& s) {
			s.doSerDes(*this);
		}
		virtual void serDes(AttributeStoreComponent& s) {
			s.doSerDes(*this);
		}

		virtual void serDes(KeyValueStore& s) {
			s.doSerDes(*this);
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

template <> template <typename DT>
void Serializer<std::ostream>::operator&(DT& d)
{
	*_t << d << " | ";
}
#endif /* SERDES_HPP_16_12_02_11_12_45 */
