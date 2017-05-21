#ifndef OBSERVER_HPP_16_11_12_10_26_41
#define OBSERVER_HPP_16_11_12_10_26_41 
#include <vector>
#include <list>
#include <memory>
#include <algorithm>
#include <iostream>
#include "reallocable.hpp"
#include <exception>

#include <typeinfo>


template <typename messageT>
class Observer_;

template <typename msgT>
using Observer = Reallocable<Observer_<msgT>>;

template <typename messageT>
class Observable_;

template <typename msgT>
using Observable = Reallocable<Observable_<msgT>>;


template <typename messageT>
class Observer_ {
	//TODO try to hide onObservable... methods .. in Observabler too
	//friend Observable_;
	//protected:
	public:
		// called when the observer is starting to observe an object
		virtual void onObservableAdd(Observable_<messageT>& o, const messageT& m) = 0;
		// called when the observable is changed
		virtual void onObservableUpdate(Observable_<messageT>& o, const messageT& m) = 0;
		// called when the observable stops observing an object
		// - either because the observable was destroyed or 
		// because Observable::removeObserver was called
		virtual void onObservableRemove(Observable_<messageT>& o, const messageT& m) = 0;
		// i guess observable.remove(self) should not be called in dtor
		virtual ~Observer_()
		{}

		virtual void swap(Observer_<messageT>&) {
		}
};

template <typename messageT>
class Observable_ {
	public:
		Observable_(messageT obsAddMsg, messageT obsRemMsg): _obsAddMsg{obsAddMsg}, _obsRemMsg{obsRemMsg}, _sendAddRemMsg(true)
		{}

		Observable_(): _sendAddRemMsg(false)
		{}

		Observable_(Observable_& other) noexcept:
			_observers{other._observers},
			_obsAddMsg{other._obsAddMsg},
			_obsRemMsg{other._obsRemMsg},
		 _sendAddRemMsg{other._sendAddRemMsg}	{
		}

		Observable_(Observable_&& other) noexcept {
			swap(*this, other);
		}

		Observable_& operator=(Observable_ other) noexcept {
			swap(*this, other);
			return *this;
		}

		void swap(Observable_& other) noexcept { //TODO when is this really noexcept?
			using std::swap;
			swap(_observers, other._observers);
			swap(_obsAddMsg, other._obsAddMsg);
			swap(_obsRemMsg, other._obsRemMsg);
			swap(_sendAddRemMsg, other._sendAddRemMsg);
		}

		void notifyObservers(const messageT& m) {
			for(int i = 0; i < _observers.size(); i++) {
				auto& observer = _observers[i];
				if(auto ospt = observer.lock())
					(*ospt)->onObservableUpdate(*this, m);
				else {
					_observers.erase(_observers.begin()+i);
					i--;
				}
			}
		}

		virtual ~Observable_() {
			std::cout << "observable dtor\n";
			for(auto& o : _observers) {
				if(auto spt = o.lock())
					sendRemMsg(**spt);
			}
		}

		virtual void addObserver(Observer<messageT>& obs) {
			_observers.push_back(obs.getSelf());
			sendAddMsg(obs);
		}

		void removeObserver(Observer<messageT>& obs) {
			for(auto it = _observers.begin(); it != _observers.end(); it++) {
				if(auto spt = it->lock()) {
					if(*spt.get() == &obs) {
						sendRemMsg(**spt);
						_observers.erase(it);
						break;
					}
				}
			}
		}

		virtual void sendAddMsg(Observer<messageT>& observer) {
			if(_sendAddRemMsg)
				observer.onObservableAdd(*this, _obsAddMsg);
		}

		virtual void sendRemMsg(Observer<messageT>& observer) {
			std::cout << "observable sendRemMsg\n";
			if(_sendAddRemMsg)
				observer.onObservableRemove(*this, _obsRemMsg);
		}


	private:
		std::vector<std::weak_ptr<Observer<messageT>*>> _observers;
		messageT _obsAddMsg;
		messageT _obsRemMsg;
		bool _sendAddRemMsg;
};

template <typename messageT>
void swap(Observable_<messageT>& lhs, Observable_<messageT>& rhs) {
	lhs.swap(rhs);
}

// an observable observer
// it creates no messages - only forwards whatever it receives up the tree
//TODO revise: after calling observabler::addObserver the observer should receive addMsgs from all observed objs
template <typename messageT>
class Observabler: public Observable<messageT>, public Observer<messageT> {
	public:
		Observabler(messageT obsAddMsg, messageT obsRemMsg): Observable<messageT>(obsAddMsg, obsRemMsg) {
		}

		Observabler() {
		}

		Observabler(Observabler&& other) {
			swap(other);
		}

		void swap(Observabler& other) {
			using std::swap;
			swap(_observed, other._observed);
			Observable<messageT>::swap(other);
			Observer<messageT>::swap(other);
		}

		// when observabler dies, send remMsgs ... but when it reallocates?
		virtual ~Observabler() {
			while(!_observed.empty()) {
				auto& o = _observed.front();
				if(auto spt = o.lock())
					(*spt)->sendRemMsg(*this);
			}
		}

		// when new observer starts observing, send him addMessages from all observed objects
		virtual void sendAddMsg(Observer<messageT>& observer) {
			for(auto& o : _observed)
				if(auto spt = o.lock())
					(*spt)->sendAddMsg(observer);
		}

		virtual void sendRemMsg(Observer<messageT>& observer) {
			for(auto& o : _observed)
				if(auto spt = o.lock())
					(*spt)->sendRemMsg(observer);
		}

	private:
		virtual void onObservableAdd(Observable_<messageT>& o, const messageT& m) {
			// this method is called when the observabler starts observing an observable
			this->notifyObservers(m);
			auto& ro = dynamic_cast<Observable<messageT>&>(o);
			_observed.push_back(ro.getSelf());
		}

		virtual void onObservableUpdate(Observable_<messageT>&, const messageT& m) {
			this->notifyObservers(m);
		}

		//TODO when this is called from Observable_ dtor, the derived part does not exist
		//anymore and the cast will fail ...
		//how about if Realocable::self returned ptr to base type?
		virtual void onObservableRemove(Observable_<messageT>& o, const messageT& m) {
			// this method is called when the observabler stops observing an observable
			std::cout << "observabler::onObservableRemove\n";
			this->notifyObservers(m);
			std::cout << typeid(o).name() << std::endl;
			auto& ro = dynamic_cast<Observable<messageT>&>(o);
			std::cout << "observabler::onObservableRemove casting passed\n";
			_observed.remove_if([&ro](auto& v) -> bool {
				if(v.expired())
					return true;
				auto vsp = v.lock();
				return *vsp == &ro;
			});
		}

		std::list<std::weak_ptr<Observable<messageT>*>> _observed;
};
#endif /* OBSERVER_HPP_16_11_12_10_26_41 */
