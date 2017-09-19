#ifndef KEYVALUESTORE_HPP_17_06_22_19_18_42
#define KEYVALUESTORE_HPP_17_06_22_19_18_42 
#include <map>
#include <cassert>
#include "serializable.hpp"

class KeyValueStore : public Serializable {
	public:
		template <typename T>
		void addPair(std::string key, T value)
		{
			assert(_store.count(key) == 0);
			_store[key] = static_cast<float>(value);
		}

		bool hasKey(std::string key) const;

		template <typename T>
		T getValue(std::string key) const
		{
			assert(_store.count(key) == 1);
			return _store.find(key)->second;
		}

		template <typename T>
		void setValue(std::string key, T value)
		{
			_store[key] = static_cast<float>(value);
		}

		virtual void serDes(SerDesBase& s);
		template <typename T>
			void doSerDes(T& t)
			{
				t & _store;
				t & _strStore;
			}

	private:
		std::map<std::string, float> _store;
		std::map<std::string, std::string> _strStore;
};

template <>
std::string KeyValueStore::getValue<std::string>(std::string key) const;

template <>
void KeyValueStore::setValue<std::string>(std::string key, std::string value);

template <>
void KeyValueStore::addPair<std::string>(std::string key, std::string value);
#endif /* KEYVALUESTORE_HPP_17_06_22_19_18_42 */
