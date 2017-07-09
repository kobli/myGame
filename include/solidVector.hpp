#ifndef SOLIDVECTOR_HPP_17_04_19_13_14_42
#define SOLIDVECTOR_HPP_17_04_19_13_14_42 
#include <vector>
#include "rental.hpp"
#include <stdexcept>
#include <cassert>

/** A vector-like container which retains indicies after insert/remove.
 * Elements are stored in contignuous memmory.
 */
template<typename T, typename indexT = std::size_t, indexT NULLi = indexT{}-1>
class SolidVector {
	static_assert(!std::is_signed<indexT>(), "indexT must be unsigned");

	public:
		SolidVector(indexT firstFree = indexT{}): _mapSlot{firstFree} {
		}

		/** Constructs an object in place from given arguments.
		 * \return index of the constructed object
		 */
		template <typename... Args>
		indexT emplace(Args&&... args) {
			_v.emplace_back(std::forward<Args>(args)...);
			indexT pi = _v.size()-1;
			return insertLogical(pi);
		}

		/** Inserts a copy of the object into the container.
		 * \return index of the inserted object
		 */
		indexT insert(const T& elem, indexT reqI = NULLi) {
			indexT pi = insertPhysical(elem);
			return insertLogical(pi, reqI);
		}

		/** Insert an object into the container.
		 * \return index of the inserted object
		 */
		indexT insert(T&& elem, indexT reqI = NULLi) {
			indexT pi = insertPhysical(std::move(elem));
			return insertLogical(pi, reqI);
		}

		/** 
		 * \return index of the next inserted object
		 */
		indexT peekNextI() {
			return _mapSlot.peek();
		}

		/** Access specified element with validity check.
		 * Throws std::out_of_range exception if index is not.
		 * \return reference to the object at given index
		 */
		T& at(indexT i) {
			return _v[physicalIndexChecked(i)];
		}

		/** Access specified element with validity check.
		 * Throws std::out_of_range exception if index is not.
		 * \return const reference to the object at given index
		 */
		const T& at(indexT i) const {
			return _v[physicalIndexChecked(i)];
		}

		/** Access specified element.
		 * \return reference to the object at given index
		 */
		T& operator[](indexT i) {
			return _v[_map[i]];
		}

		/** Access specified element.
		 * \return const reference to the object at given index
		 */
		const T& operator[](indexT i) const {
			return _v[_map[i]];
		}

		/** Removes specified element.
		 * Throws std::out_of_range exception if index is not valid.
		 * Uses swap function.
		 */
		void remove(indexT i) {
			// swap elements at pi and last
			indexT pi = physicalIndexChecked(i);
			indexT lastpi = _v.size()-1;
			using std::swap;
			swap(_v.at(pi), _v.at(lastpi));
			// swap pointers in _map - search _map for record pointing to pi (linear)
			for(auto& m : _map)
				if(m == lastpi) {
					m = pi;
					break;
				}
			// remove last element of vector
			_map[i] = NULLi;
			// pop_back calls the elements destructor - make sure the container is in valid state
			_v.pop_back();
			_mapSlot.remit(i);
		}

		typedef typename std::vector<T>::iterator iterator;

		indexT iteratorToIndex(iterator it) {
			indexT pi = it-_v.begin();
			for(indexT li = 0; li < _map.size(); li++)
				if(_map[li] == pi)
					return li;
			return NULLi;
		}

		void remove(iterator it) {
			remove(iteratorToIndex(it));
		}

		/** Removes all elements.
		 */
		void clear() {
			_map.clear();
			_mapSlot.reset();
			_v.clear();
		}

		/**
		 * \return number of elements in the container.
		 */
		indexT size() {
			return _v.size();
		}

		/*
		class iterator {// : public std::forward_iterator_tag { // TODO is the inheritance necessary?
			public:

				typedef T                          value_type;
				typedef indexT										 difference_type;
				typedef std::forward_iterator_tag iterator_category; //TODO change to random_access
				typedef T*                         pointer;
				typedef T&                         reference;

				iterator(): _v{nullptr} {
				}
				
				//TODO make this private
				iterator(SolidVector<T>& v, indexT elemI): _i{elemI}, _v{&v} {
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

				indexT _i;
				SolidVector<T>* _v;
		};

		iterator begin() {
			return iterator{*this, 0};
		}

		iterator end() {
			return iterator{*this, size()};
		}
		*/

		/**
		 * \return an iterator to the beginning.
		 */
		iterator begin() {
			return _v.begin();
		}

		/**
		 * \return an iterator past the last element. (Same as begin when empty)
		 */
		iterator end() {
			return _v.end();
		}

		/** Index validity check
		 * \return true if index is valid, false otherwise.
		 */
		bool indexValid(indexT i) const {
			if(i >= _map.size())
				return false;
			indexT pi = _map[i];
			if(pi == NULLi)
				return false;
			return true;
		}

	private:
		// inserts the element and returns physical index
		// (the physical index may change on remove)
		indexT insertPhysical(T&& elem) {
			_v.push_back(std::move(elem));
			return _v.size()-1;
		}

		// creates a new element and returns its logical index
		indexT insertLogical(indexT pi, indexT logicalPosHint = NULLi) {
			indexT li = logicalPosHint==NULLi ?
				_mapSlot.borrow() :
				_mapSlot.borrow(logicalPosHint);
			assert(logicalPosHint == NULLi || logicalPosHint == li);//TODO remove -- not always true (but here now should always be true)
			_map.resize(std::max<indexT>(li+1, _map.size()), NULLi);
			_map[li] = pi;
			return li;
		}
		
		// returns physical index if the logical index is valid
		// else throws std::out_of_range
		indexT physicalIndexChecked(indexT logicalI) const {
			if(!indexValid(logicalI))
				throw std::out_of_range("Index out of range");
			else
				return _map[logicalI];
		}

		std::vector<indexT> _map;
		Rental<indexT> _mapSlot;
		std::vector<T> _v;
};

#endif /* SOLIDVECTOR_HPP_17_04_19_13_14_42 */
