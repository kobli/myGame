#ifndef KEYVALUESTORE_HPP_17_06_22_19_18_42
#define KEYVALUESTORE_HPP_17_06_22_19_18_42 
#include <map>
#include "serializable.hpp"

class KeyValueStore : public Serializable {
	public:
		void addPair(std::string key, float value);
		bool hasKey(std::string key) const;
		float getValue(std::string key) const;
		void setValue(std::string key, float value);

		virtual void serDes(SerDesBase& s);
		template <typename T>
			void doSerDes(T& t)
			{
				t & _store;
			}

	private:
		std::map<std::string, float> _store;
};

#endif /* KEYVALUESTORE_HPP_17_06_22_19_18_42 */
