#ifndef REALLOCABLE_HPP_17_05_14_10_13_14
#define REALLOCABLE_HPP_17_05_14_10_13_14 
#include <memory>

template <typename T>
class Reallocable : public T {
	public:
		template <typename... Args>
		Reallocable(Args... args): T{args...},
			_self{new Reallocable*(this)} {
		}

		Reallocable():
			_self{new Reallocable*(this)} {
		}

		Reallocable(Reallocable& other) noexcept: T{other},
			_self{new Reallocable*(this)} {
		}

		Reallocable& operator=(Reallocable& other) noexcept {
			_self.reset(new Reallocable*(this));
			T::operator=(other);
		}

		Reallocable(Reallocable&& other) noexcept: T{std::move(other)}, 
			_self{new Reallocable*(this)} {
			swapSelf(other._self);
		}
		
		Reallocable& operator=(Reallocable&& other) noexcept {
			T::operator=(std::move(other));
			swapSelf(other._self);
			return *this;
		}

		std::weak_ptr<Reallocable<T>*> getSelf() {
			return _self;
		}

		virtual void swap(Reallocable& other) {
			swapSelf(other._self);
			T::swap(other);
		}

	protected:
		std::shared_ptr<Reallocable<T>*> _self;

		void swapSelf(std::shared_ptr<Reallocable<T>*>& otherSelf) {
			using std::swap;
			swap(_self, otherSelf);
			swap(*_self, *otherSelf);
		}
};

template <typename T>
void swap(Reallocable<T>& lhs, Reallocable<T>& rhs) {
	lhs.swap(rhs);
}

#endif /* REALLOCABLE_HPP_17_05_14_10_13_14 */
