#ifndef OBSERVABLEKEYVALUESTORE_HPP_17_09_29_15_25_34
#define OBSERVABLEKEYVALUESTORE_HPP_17_09_29_15_25_34 
#include "keyValueStore.hpp"
#include "observer.hpp"

template <typename IDType>
struct KeyValueStoreChange {
	KeyValueStoreChange(IDType id): typeID{id}
	{}
	
	const IDType typeID;	
};

template <typename KeyValueStoreIDType, KeyValueStoreIDType typeID>
class ObservableKeyValueStore: public KeyValueStore, public Observable<KeyValueStoreChange<KeyValueStoreIDType>>
{
	public:
		using MessageT = KeyValueStoreChange<KeyValueStoreIDType>;
		virtual void addPair(std::string key, float value) override;
		virtual void addPair(std::string key, std::string value) override;
		virtual void setValue(std::string key, float value) override;
		virtual void setValue(std::string key, std::string value) override;

	private:
		void notify();
};

template <typename KeyValueStoreIDType, KeyValueStoreIDType typeID>
void ObservableKeyValueStore<KeyValueStoreIDType,typeID>::addPair(std::string key, float value)
{
	KeyValueStore::addPair(key, value);
	notify();
}	

template <typename KeyValueStoreIDType, KeyValueStoreIDType typeID>
void ObservableKeyValueStore<KeyValueStoreIDType,typeID>::addPair(std::string key, std::string value)
{
	KeyValueStore::addPair(key, value);
	notify();
}

template <typename KeyValueStoreIDType, KeyValueStoreIDType typeID>
void ObservableKeyValueStore<KeyValueStoreIDType,typeID>::setValue(std::string key, float value)
{
	KeyValueStore::setValue(key, value);
	notify();
}

template <typename KeyValueStoreIDType, KeyValueStoreIDType typeID>
void ObservableKeyValueStore<KeyValueStoreIDType,typeID>::setValue(std::string key, std::string value)
{
	KeyValueStore::setValue(key, value);
	notify();
}

template <typename KeyValueStoreIDType, KeyValueStoreIDType typeID>
void ObservableKeyValueStore<KeyValueStoreIDType,typeID>::notify()
{
	this->broadcastMsg(KeyValueStoreChange<KeyValueStoreIDType>(typeID));
}

#endif /* OBSERVABLEKEYVALUESTORE_HPP_17_09_29_15_25_34 */
