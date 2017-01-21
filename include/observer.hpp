#include <vector>
#include <list>
#include <memory>
#include <algorithm>
#include <iostream>

#ifndef OBSERVER_HPP_16_11_12_10_26_41
#define OBSERVER_HPP_16_11_12_10_26_41 

template <typename messageT>
class Observer;

template <typename messageT>
class Observable;

template <typename messageT>
class Observer {
	friend Observable<messageT>;
	public:
		Observer(): _self{this, [](Observer<messageT>*){}} {
		}
		virtual void observe(Observable<messageT>& o) {
			o.addObserver(*this);
		}
		// called when the observer is starting to observe an object
		virtual void onObservableAdd(messageT& m) = 0;
		// called when the observable is changed
		virtual void onObservableUpdate(messageT& m) = 0;
		// called when the observable stops observing an object
		// - either because the observable was destroyed or 
		// because Observable::removeObserver was called
		virtual void onObservableRemove(messageT& m) = 0;
		virtual void onObservableRemove2(Observable<messageT>*, messageT& m) { onObservableRemove(m); }
		// i guess observable.remove(self) should not be called in dtor
		virtual ~Observer()
		{}
	private:
		std::shared_ptr<Observer<messageT>> _self;
};

template <typename messageT>
class Observable {
	friend Observer<messageT>;
	public:
		Observable(messageT obsAddMsg, messageT obsRemMsg): _obsAddMsg{obsAddMsg}, _obsRemMsg{obsRemMsg}
		{}

	protected:
		virtual void addObserver(Observer<messageT>& obs)
		{
			_observers.push_back(obs._self);
			sendAddMsg(obs);
		}

		//TODO make removeObserver private - same as addObserver
	public:
		virtual void sendAddMsg(Observer<messageT>& obs)
		{
			obs.onObservableAdd(_obsAddMsg);
		}

		void removeObserver(Observer<messageT>& obs)
		{
			auto it = _observers.begin();
			for(; it != _observers.end(); it++)
				if(it->get() == &obs)
					break;
			if(it != _observers.end())
			{
				if(auto spt = it->lock())
					spt->onObservableRemove2(this, _obsRemMsg);
				_observers.erase(it);
			}
		}

		void notifyObservers(messageT& m)
		{
			for(auto& o : _observers)
			{
				if(auto spt = o.lock())
					spt->onObservableUpdate(m);
			}
		}

		void removeDeadObservers()
		{
			_observers.erase(std::remove_if(_observers.begin(), 
						_observers.end(),
						[](std::weak_ptr<Observer<messageT>> p){return !p.lock();}),
					_observers.end());
		}

		virtual ~Observable()
		{
			std::cout << "dest observable\n";
			for(auto& o : _observers)
			{
				if(auto spt = o.lock())
					spt->onObservableRemove2(this, _obsRemMsg);
			}
		}

	private:
		std::vector<std::weak_ptr<Observer<messageT>>> _observers;
		messageT _obsAddMsg;
		messageT _obsRemMsg;
};

// an observable observer
template <typename messageT>
class Observabler: public Observable<messageT>, protected Observer<messageT> {
	public:
		Observabler(messageT obsAddMsg, messageT obsRemMsg): Observable<messageT>(obsAddMsg, obsRemMsg)
		{}
		virtual ~Observabler()
		{
			std::cout << "dest observabler\n";
		}
		virtual void observe(Observable<messageT>& o) {
			Observer<messageT>::observe(o);
			_observed.push_back(&o);
		}
		//TODO
		/*
		virtual void stop_observe(Observable<messageT>& o) {
			o.addObserver(*this);
		}
		*/

		virtual void sendAddMsg(Observer<messageT>& obs)
		{
			for(auto o : _observed)
				o->sendAddMsg(obs);
		}


	private:
		virtual void onObservableAdd(messageT& m)
		{
			this->notifyObservers(m);
		}

		virtual void onObservableUpdate(messageT& m)
		{
			this->notifyObservers(m);
		}

		virtual void onObservableRemove(messageT& m)
		{
			this->notifyObservers(m);
		}

		virtual void onObservableRemove2(Observable<messageT>* o, messageT& m)
		{
			onObservableRemove(m);
			_observed.remove(o);
		}
		std::list<Observable<messageT>*> _observed; // TODO are all observed elements always valid? do they stay alive? -- now probably yes
};
#endif /* OBSERVER_HPP_16_11_12_10_26_41 */
