#ifndef OBSERVABLEKEYVALUESTORE_HPP_17_09_29_15_25_34
#define OBSERVABLEKEYVALUESTORE_HPP_17_09_29_15_25_34 
#include "keyValueStore.hpp"
#include "observer.hpp"

template <typename IDType>
struct KeyValueStoreChange {
	KeyValueStoreChange(IDType id, KeyValueStore* store): typeID{id}, store{store}
	{}
	
	const IDType typeID;	
	KeyValueStore* const store;
};

template <typename KeyValueStoreIDType, KeyValueStoreIDType typeID>
class ObservableKeyValueStore: public KeyValueStore, public Observable<KeyValueStoreChange<KeyValueStoreIDType>>
{
	public:
		ObservableKeyValueStore(): Observable<KeyValueStoreChange<KeyValueStoreIDType>>{KeyValueStoreChange<KeyValueStoreIDType>(typeID, this)}
		{}

		using MessageT = KeyValueStoreChange<KeyValueStoreIDType>;
		virtual void addPair(std::string key, float value) override
		{
			KeyValueStore::addPair(key, value);
			notify();
		}	

		virtual void addPair(std::string key, std::string value) override
		{
			KeyValueStore::addPair(key, value);
			notify();
		}

		virtual void setValue(std::string key, float value) override
		{
			KeyValueStore::setValue(key, value);
			notify();
		}

		virtual void setValue(std::string key, std::string value) override
		{
			KeyValueStore::setValue(key, value);
			notify();
		}

		void swap(ObservableKeyValueStore& other)
		{
			using std::swap;
			swap(static_cast<KeyValueStore&>(*this), static_cast<KeyValueStore&>(other));
			using ObservableT = Observable<KeyValueStoreChange<KeyValueStoreIDType>>;
			swap(static_cast<ObservableT&>(*this), static_cast<ObservableT&>(other));
		}

	private:
		void notify()
		{
			this->broadcastMsg(KeyValueStoreChange<KeyValueStoreIDType>(typeID, this));
		}
};


template <typename KeyValueStoreIDType, KeyValueStoreIDType typeID>
void swap(ObservableKeyValueStore<KeyValueStoreIDType, typeID>& lhs, ObservableKeyValueStore<KeyValueStoreIDType, typeID>& rhs)
{
	lhs.swap(rhs);
}

#endif /* OBSERVABLEKEYVALUESTORE_HPP_17_09_29_15_25_34 */
