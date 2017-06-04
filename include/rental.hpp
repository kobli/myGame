#ifndef RENTAL_HPP_17_04_19_16_16_53
#define RENTAL_HPP_17_04_19_16_16_53 

#include <set>

template<typename T>
class Rental {
	public:
		Rental(T generator = T{}): _gen{generator} {
		}

		T borrow() {
			if(_store.empty())
				return _gen++;
			else {
				T r = *_store.begin();
				_store.erase(r);
				return r;
			}
		}

		T borrow(T demand) {
			auto r = _store.end();
			if((r = _store.find(demand)) != _store.end()) {
				auto v = *r;
				_store.erase(r);
				return v;
			}
			else {
				while(!(_gen == T{} && !_store.empty())) { // overflow
					if(_gen == demand)
						return _gen++;
					else
						_store.insert(_gen++);
				}
				return borrow();
			}
		}

		T peek() {
			if(_store.empty())
				return _gen;
			else {
				return *_store.begin();
			}
		}

		void remit(T i) {
			_store.insert(i);
		}
		
		void reset() {
			_gen = {};
			_store.clear();
		}

	private:
		T _gen;
		std::set<T> _store;
};
#endif /* RENTAL_HPP_17_04_19_16_16_53 */
