#ifndef OBSERVER_HPP_16_11_12_10_26_41
#define OBSERVER_HPP_16_11_12_10_26_41 
#include <vector>
#include <list>
#include <memory>
#include <algorithm>
#include "reallocable.hpp"
#include "copyOrNull.hpp"
#include <iostream>

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
	friend Observable_<messageT>;
	protected:
		virtual void onMsg(const messageT& m) = 0;
		virtual void onDirectObservableAdd(Observable_<messageT>& o) {
		}
	public:
		virtual ~Observer_()
		{}

		virtual void swap(Observer_<messageT>&) {
		}
};

template <typename messageT>
class Observable_ {
	public:
		Observable_(messageT obsHelloMsg, messageT obsByeMsg): _obsHelloMsg{obsHelloMsg}, _obsByeMsg{obsByeMsg}
		{}

		Observable_()
		{}

		Observable_(Observable_& other) noexcept:
			_observers{other._observers},
			_obsHelloMsg{other._obsHelloMsg},
			_obsByeMsg{other._obsByeMsg} {
		}

		Observable_(Observable_&& other) noexcept {
			swap(other);
		}

		Observable_& operator=(Observable_ other) noexcept {
			swap(other);
			return *this;
		}

		virtual ~Observable_() {
			for(auto& o : _observers) {
				if(auto spt = o.lock())
					sendByeMsgTo(**spt);
			}
		}

		void swap(Observable_& other) noexcept { //TODO when is this really noexcept?
			using std::swap;
			swap(_observers, other._observers);
			swap(_obsHelloMsg, other._obsHelloMsg);
			swap(_obsByeMsg, other._obsByeMsg);
		}

		virtual void addObserver(Observer<messageT>& obs) {
			_observers.push_back(obs.getSelf());
			obs.onDirectObservableAdd(*this);
			sendHelloMsgTo(obs);
		}

		void removeObserver(Observer<messageT>& obs) {
			for(auto it = _observers.begin(); it != _observers.end(); it++) {
				if(auto spt = it->lock()) {
					if(*spt.get() == &obs) {
						sendByeMsgTo(**spt);
						_observers.erase(it);
						break;
					}
				}
			}
		}

		void removeAllObservers() {
			_observers.clear();
		}

		virtual void sendHelloMsgTo(Observer<messageT>& observer) {
			if(!_obsHelloMsg.isNull())
				observer.onMsg(_obsHelloMsg.get());
		}

		virtual void sendByeMsgTo(Observer<messageT>& observer) {
			if(!_obsByeMsg.isNull()) {
				observer.onMsg(_obsByeMsg.get());
			}
		}
		
		virtual void broadcastMsg(const messageT& m) {
			for(int i = 0; i < _observers.size(); i++) {
				auto& observer = _observers[i];
				auto ospt = observer.lock();
				if(ospt && *ospt)
					(*ospt)->onMsg(m);
				else {
					_observers.erase(_observers.begin()+i);
					i--;
				}
			}
		}

	private:
		std::vector<std::weak_ptr<Observer<messageT>*>> _observers;
		CopyOrNull<messageT> _obsHelloMsg;
		CopyOrNull<messageT> _obsByeMsg;
};

template <typename messageT>
void swap(Observable_<messageT>& lhs, Observable_<messageT>& rhs) {
	lhs.swap(rhs);
}

// an observable observer
// it creates no messages - only forwards whatever it receives up the tree
// after calling observabler::addObserver the observer should receive HelloMsgs from all observed objs
// also when observabler dies, the observer should receive ByeMsg from objects observed by observabler
template <typename messageT>
class Observabler: public Observable<messageT>, public Observer<messageT> {
	public:
		Observabler() = default;

		Observabler(messageT obsHelloMsg, messageT obsByeMsg)
			: Observable<messageT>(obsHelloMsg, obsByeMsg) {
		}

		Observabler(Observabler& other, bool dropObservers = true)
			: Observable<messageT>(static_cast<Observable<messageT>&>(other))
				, Observer<messageT>(static_cast<Observer<messageT>&>(other)) {
			_observed = other._observed;
			if(dropObservers)
				this->removeAllObservers();
		}

		Observabler& operator=(Observabler& other) = default;

		Observabler(Observabler&& other) = default;

		Observabler& operator=(Observabler&& other) {
			Observable<messageT>::operator=(std::move(other));
			Observer<messageT>::operator=(std::move(other));
			swapSelf(other);
			return *this;
		}

		void swap(Observabler& other) {
			swapSelf(other);
			Observable<messageT>::swap(other);
			Observer<messageT>::swap(other);
		}

		virtual ~Observabler() {
			while(!_observed.empty()) {
				auto& o = _observed.front();
				if(auto spt = o.lock())
					(*spt)->sendByeMsgTo(*this);
				_observed.pop_front();
			}
		}

		// when new observer starts observing, send him addMessages from all observed objects and mine
		virtual void sendHelloMsgTo(Observer<messageT>& observer) {
			Observable<messageT>::sendHelloMsgTo(observer);
			for(auto& o : _observed)
				if(auto spt = o.lock())
					(*spt)->sendHelloMsgTo(observer);
		}

		virtual void sendByeMsgTo(Observer<messageT>& observer) {
			Observable<messageT>::sendByeMsgTo(observer);
			for(auto& o : _observed)
				if(auto spt = o.lock())
					(*spt)->sendByeMsgTo(observer);
		}

		virtual void onMessage(const messageT& m) {
			this->broadcastMsg(m);
		}

	private:
		void swapSelf(Observabler& other) {
			using std::swap;
			swap(_observed, other._observed);
		}

		virtual void onMsg(const messageT& m) final override {
			_observed.remove_if([](auto& v) -> bool {
				if(v.expired())
					return true;
				else
				return false;
			});
			onMessage(m);
		}

		virtual void onDirectObservableAdd(Observable_<messageT>& o) final override {
			auto& ro = dynamic_cast<Observable<messageT>&>(o);
			_observed.push_back(ro.getSelf());
		}

		std::list<std::weak_ptr<Observable<messageT>*>> _observed;
};
#endif /* OBSERVER_HPP_16_11_12_10_26_41 */
