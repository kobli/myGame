#include <main.hpp>
#include <SFML/Network.hpp>
#include <world.hpp>

#ifndef NETWORK_HPP_16_11_27_11_45_29
#define NETWORK_HPP_16_11_27_11_45_29 

enum PacketType: u8
{
	PlayerCommand,
	WorldUpdate,
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
#endif /* NETWORK_HPP_16_11_27_11_45_29 */
