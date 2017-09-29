#include <main.hpp>
#include <SFML/Network.hpp>
#include <world.hpp>

#ifndef NETWORK_HPP_16_11_27_11_45_29
#define NETWORK_HPP_16_11_27_11_45_29 

enum PacketType: u8
{
	PlayerCommand,
	WorldUpdate,
	RegistryUpdate,
	GameRegistryUpdate,
	GameInit,
	ClientHello,
	ServerMessage,
};

/*
template <typename enumT, typename enumW = u8>
sf::Packet& operator <<(sf::Packet& packet, const enumT& m) {
  return packet << static_cast<enumW>(m);
}
template <typename enumT, typename enumW = u8>
sf::Packet& operator >>(sf::Packet& packet, enumT& m) {
	enumW d;
  packet >> d;
	m = static_cast<enumT>(d);
	return packet;
}
*/

sf::Packet& operator <<(sf::Packet& packet, const ComponentType& m);
sf::Packet& operator >>(sf::Packet& packet, ComponentType& m);

template <typename T>
sf::Packet& operator <<(sf::Packet& packet, const vec2<T>& m) {
  return packet << m.X << m.Y;
}
template <typename T>
sf::Packet& operator >>(sf::Packet& packet, vec2<T>& m) {
  return packet >> m.X >> m.Y;
}
template <typename T>
sf::Packet& operator <<(sf::Packet& packet, const vec3<T>& m) {
  return packet << m.X << m.Y << m.Z;
}
template <typename T>
sf::Packet& operator >>(sf::Packet& packet, vec3<T>& m) {
  return packet >> m.X >> m.Y >> m.Z;
}
template sf::Packet& operator << <float>(sf::Packet& packet, const vec2f& m);
template sf::Packet& operator >> <float>(sf::Packet& packet, vec2f& m);
sf::Packet& operator <<(sf::Packet& packet, const quaternion& q);
sf::Packet& operator >>(sf::Packet& packet, quaternion& q);

sf::Packet& operator <<(sf::Packet& packet, const PacketType& m);
sf::Packet& operator >>(sf::Packet& packet, PacketType& m);

sf::Packet& operator <<(sf::Packet& packet, const EntityEvent& m);
sf::Packet& operator >>(sf::Packet& packet, EntityEvent& m);

sf::Packet& operator <<(sf::Packet& packet, const Command& m);
sf::Packet& operator >>(sf::Packet& packet, Command& m);

sf::Packet& operator <<(sf::Packet& packet, const Command::Type& m);
sf::Packet& operator >>(sf::Packet& packet, Command::Type& m);

sf::Packet& operator <<(sf::Packet& packet, const irr::scene::ESCENE_NODE_TYPE& m);
sf::Packet& operator >>(sf::Packet& packet, irr::scene::ESCENE_NODE_TYPE& m);

template <typename T, typename K, typename V>
T& operator <<(T& t, const std::map<K,V>& m) {
	t << static_cast<u32>(m.size());
	for(auto& p : m)
		t << p;
	return t;
}
template <typename T, typename K, typename V>
T& operator >>(T& t, std::map<K,V>& m) {
	m.clear();
	u32 s;
	t >> s;
	std::pair<K,V> p;
	for(u32 i = 0; i < s; i++) {
		t >> p;
		m.insert(p);
	}
	return t;
}

template <typename T, typename K, typename V>
T& operator <<(T& t, const std::pair<K,V>& p) {
	return t << p.first << p.second;
}
template <typename T, typename K, typename V>
T& operator >>(T& t, std::pair<K,V>& p) {
	return t >> p.first >> p.second;
}

template <typename T, typename TT>
T& operator<<(T& t, const std::vector<TT>& v)
{
	t << u32(v.size());
	for(const auto& e : v)
		t << e;
}
template <typename T, typename TT>
T& operator>>(T& t, std::vector<TT>& v)
{
	v.clear();
	u32 size;
	t >> size;
	v.reserve(size);
	for(u32 i = 0; i < size; i++) {
		TT e;
		t >> e;
		v.push_back(e);
	}
}
#endif /* NETWORK_HPP_16_11_27_11_45_29 */
