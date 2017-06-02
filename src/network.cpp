#include <network.hpp>

sf::Packet& operator <<(sf::Packet& packet, const quaternion& q)
{
	return packet << q.X << q.Y << q.Z << q.W;
}
sf::Packet& operator >>(sf::Packet& packet, quaternion& q)
{
	return packet >> q.X >> q.Y >> q.Z >> q.W;
}

sf::Packet& operator <<(sf::Packet& packet, const ComponentType& m) {
  return packet << static_cast<u8>(m);
}
sf::Packet& operator >>(sf::Packet& packet, ComponentType& m) {
	u8 d;
  packet >> d;
	m = static_cast<ComponentType>(d);
	return packet;
}

sf::Packet& operator <<(sf::Packet& packet, const PacketType& m) {
  return packet << static_cast<u8>(m);
}
sf::Packet& operator >>(sf::Packet& packet, PacketType& m) {
	u8 d;
  packet >> d;
	m = static_cast<PacketType>(d);
	return packet;
}

sf::Packet& operator <<(sf::Packet& packet, const EntityEvent& m) {
  return packet << m.entityID << m.componentT;//TODO send create/destroy events  << m._created << m._destroyed;
}
sf::Packet& operator >>(sf::Packet& packet, EntityEvent& m) {
  return packet >> m.entityID >> m.componentT; // >> m._created >> m._destroyed;
}

sf::Packet& operator <<(sf::Packet& packet, const Command& m) {
	packet << m._type;
	switch(m._type)
	{
		case Command::Type::Null:
			break;
		case Command::Type::STRAFE_DIR_SET:
			packet << m._vec2f;
			break;
		case Command::Type::ROT_DIR_SET:
			packet << m._i32;
			break;
		case Command::Type::STR:
			packet << m._str;
			break;
		default:
			break;
	}
  return packet;
}
sf::Packet& operator>>(sf::Packet& packet, Command& m) {
	packet >> m._type;
	switch(m._type)
	{
		case Command::Type::Null:
			break;
		case Command::Type::STRAFE_DIR_SET:
			packet >> m._vec2f;
			break;
		case Command::Type::ROT_DIR_SET:
			packet >> m._i32;
			break;
		case Command::Type::STR:
			packet >> m._str;
			break;
		default:
			break;
	}
	return packet;
}

sf::Packet& operator <<(sf::Packet& packet, const Command::Type& m)
{
  return packet << static_cast<u16>(m);
}
sf::Packet& operator >>(sf::Packet& packet, Command::Type& m)
{
	u16 d;
  packet >> d;
	m = static_cast<Command::Type>(d);
	return packet;
}

sf::Packet& operator <<(sf::Packet& packet, const irr::scene::ESCENE_NODE_TYPE& m)
{
	return packet << u32(m);
}
sf::Packet& operator >>(sf::Packet& packet, irr::scene::ESCENE_NODE_TYPE& m)
{
	u32 d;
	packet >> d;
	m = (irr::scene::ESCENE_NODE_TYPE)d;
	return packet;
}
