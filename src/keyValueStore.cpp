#include "keyValueStore.hpp"

template <>
void KeyValueStore::addPair<std::string>(std::string key, std::string value)
{
	assert(_strStore.count(key) == 0);
	_strStore[key] = value;
}

bool KeyValueStore::hasKey(std::string key) const
{
	return _store.find(key) != _store.end() || _strStore.find(key) != _strStore.end();
}

template <>
std::string KeyValueStore::getValue<std::string>(std::string key) const
{
	assert(_strStore.count(key) == 1);
	return _strStore.find(key)->second;
}

template <>
void KeyValueStore::setValue<std::string>(std::string key, std::string value)
{
	_strStore[key] = value;
}

void KeyValueStore::serDes(SerDesBase& s)
{
	s.serDes(*this);
}
