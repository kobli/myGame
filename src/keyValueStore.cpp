#include "keyValueStore.hpp"
#include <cassert>

void KeyValueStore::addPair(std::string key, float value)
{
	assert(_store.count(key) == 0);
	_store[key] = value;
}

bool KeyValueStore::hasKey(std::string key) const
{
	return _store.find(key) != _store.end();
}

float KeyValueStore::getValue(std::string key) const
{
	assert(_store.count(key) == 1);
	return _store.find(key)->second;
}

void KeyValueStore::setValue(std::string key, float value)
{
	_store[key] = value;
}

void KeyValueStore::serDes(SerDesBase& s)
{
	s.serDes(*this);
}
