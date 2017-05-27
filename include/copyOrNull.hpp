#ifndef COPYORNULL_HPP_17_05_27_10_27_25
#define COPYORNULL_HPP_17_05_27_10_27_25 
#include <memory>
#include <cassert>

template <typename T>
class CopyOrNull {
	public:
		CopyOrNull() {
		}

		CopyOrNull(T& obj): _obj{new T(obj)} {
		}

		CopyOrNull(CopyOrNull<T>& other) {
			if(!other.isNull())
				_obj.reset(new T(other.get()));
		}

		CopyOrNull(CopyOrNull<T>&& other) {
			std::swap(_obj, other._obj);
		}

		CopyOrNull& operator=(CopyOrNull<T>& other) {
			if(!other.isNull())
				_obj.reset(new T(other.get()));
		}

		CopyOrNull& operator=(CopyOrNull<T>&& other) {
			std::swap(_obj, other._obj);
		}

		bool isNull() {
			return _obj == nullptr;
		}

		T& get() {
			assert(!isNull());
			return *_obj;
		}

		bool operator==(const CopyOrNull& other) const {
			return (_obj.isNull() && other.isNull())
				|| *_obj == *_obj;
		}

	private:
		std::unique_ptr<T> _obj;
};
#endif /* COPYORNULL_HPP_17_05_27_10_27_25 */
