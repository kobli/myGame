#ifndef RINGBUFFER_HPP_17_09_22_14_40_51
#define RINGBUFFER_HPP_17_09_22_14_40_51 
#include <vector>

template <typename T>
class RingBuffer {
	public:
		RingBuffer(unsigned capacity): _capacity{capacity}, _first{0}
		{
			_store.reserve(capacity);
		}

		void push_back(T& t) {
			if(size() < _capacity)
				_store.push_back(t);
			else {
				unsigned writeI = firstFree();
				_store[writeI] = t;
				if(writeI == _first)
					advance(_first);
			}
		}

		T pop_first() {
			assert(size() != 0);
			T r = _store[_first];
			advance(_first);
			return r;
		}

		std::vector<T> data() const {
			std::vector<T> r;
			r.reserve(_capacity);
			for(unsigned i = _first; r.size() != size(); advance(i))
				r.push_back(_store[i]);
			return r;
		}

	private:
		std::vector<T> _store;
		unsigned _capacity;
		unsigned _first;

		void advance(unsigned& i) const {
			i = (i+1)%_capacity;
		}

		unsigned size() const {
			return _store.size();
		}

		unsigned firstFree() const {
			return (_first+size())%_capacity;
		}
};
#endif /* RINGBUFFER_HPP_17_09_22_14_40_51 */
