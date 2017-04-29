#ifndef SOLIDVECTOR_HPP_17_04_19_13_14_42
#define SOLIDVECTOR_HPP_17_04_19_13_14_42 
#include <vector>
#include "rental.hpp"
#include <stdexcept>

template<typename T>
class SolidVector {
	typedef std::size_t size_t;
	static const size_t NONE = 0-1;
	public:
		size_t emplace() {
			return insert(T{});
		}

		size_t insert(const T& elem) {
			size_t pi = insertPhysical(elem);
			return insertLogical(pi);
		}

		size_t insert(T&& elem) {
			size_t pi = insertPhysical(std::move(elem));
			return insertLogical(pi);
		}

		// throws when i out of range
		T& at(size_t i) {
			return _v[physicalIndexChecked(i)];
		}

		const T& at(size_t i) const {
			return _v[physicalIndexChecked(i)];
		}

		T& operator[](size_t i) {
			return _v[_map[i]];
		}

		const T& operator[](size_t i) const {
			return _v[_map[i]];
		}

		void remove(size_t i) {
			// swap elements at pi and last
			size_t pi = physicalIndexChecked(i);
			size_t lasti = _v.size()-1;
			using std::swap;
			swap(_v.at(pi), _v.at(lasti));
			// swap pointers in _map - search _map for record pointing to pi (linear)
			for(auto& m : _map)
				if(m == lasti) {
					m = pi;
					break;
				}
			// remove last element of vector
			_v.pop_back();
			// mark slot as free
			_mapSlot.remit(i);
			_map[i] = NONE;
		}

		size_t size() {
			return _v.size();
		}

		typedef typename std::vector<T>::iterator iterator;
		/*
		class iterator {// : public std::forward_iterator_tag { // TODO is the inheritance necessary?
			public:

				typedef T                          value_type;
				typedef size_t										 difference_type;
				typedef std::forward_iterator_tag iterator_category; //TODO change to random_access
				typedef T*                         pointer;
				typedef T&                         reference;

				iterator(): _v{nullptr} {
				}
				
				//TODO make this private
				iterator(SolidVector<T>& v, size_t elemI): _i{elemI}, _v{&v} {
				}

				iterator(const iterator& it) : _i{it._i}, _v{it._v} {
				}

				iterator& operator=(iterator& it) {
					_i = it._i;
					_v = it._v;
				}
				 
				iterator& operator++() { // ++it
					_i++;
				 	return *this;
				}
					        
				iterator operator++(int) { // it++
					iterator tmp(*this);
				 	++(*this);
				 	return tmp;
				}
				
				bool operator==(iterator& it) {
					return _i == it._i && _v == it._v;
				}

				bool operator!=(iterator& it) {
					return !(*this == it);
				}

				T& operator*() {
					return (*_v)[_i];
				}

				T* operator->() {
					return &(*_v)[_i];
				}


			private:

				size_t _i;
				SolidVector<T>* _v;
		};

		iterator begin() {
			return iterator{*this, 0};
		}

		iterator end() {
			return iterator{*this, size()};
		}
		*/
		iterator begin() {
			return _v.begin();
		}

		iterator end() {
			return _v.end();
		}

		bool indexValid(size_t i) const {
			if(i >= _map.size())
				return false;
			size_t pi = _map[i];
			if(pi == NONE)
				return false;
			return true;
		}

	private:
		size_t insertPhysical(T&& elem) {
			_v.push_back(std::move(elem));
			return _v.size()-1;
		}

		size_t insertLogical(size_t pi) {
			size_t li = _mapSlot.borrow();
			_map.resize(std::max(li+1, _map.size()));
			_map[li] = pi;
			return li;
		}
		
		size_t physicalIndexChecked(size_t logicalI) const {
			if(!indexValid(logicalI))
				throw std::out_of_range("Index out of range");
			else
				return _map[logicalI];
		}

		std::vector<size_t> _map;
		Rental<size_t> _mapSlot;
		std::vector<T> _v;
};

#endif /* SOLIDVECTOR_HPP_17_04_19_13_14_42 */
