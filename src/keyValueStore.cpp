#include "keyValueStore.hpp"

KeyValueStore::~KeyValueStore()
{}

void KeyValueStore::addPair(std::string key, float value)
{
	assert(_store.count(key) == 0);
	_store[key] = static_cast<float>(value);
}

void KeyValueStore::addPair(std::string key, std::string value)
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
	if(_strStore.count(key) == 1)
		return _strStore.find(key)->second;
	else if(_store.count(key) == 1) {
		float val = _store.find(key)->second;

		if(val == int(val))
			return std::to_string(int(val));
		else
			return std::to_string(val);
	}
	assert(false);
}

void KeyValueStore::setValue(std::string key, float value)
{
	_store[key] = static_cast<float>(value);
}

void KeyValueStore::setValue(std::string key, std::string value)
{
	_strStore[key] = value;
}

void KeyValueStore::serDes(SerDesBase& s)
{
	s.serDes(*this);
}
