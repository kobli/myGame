#include <observer.hpp>
#include "gtest/gtest.h"
#include <queue>
using namespace std;
class Msg
{
	public:
		Msg(int data = 0, bool add=false, bool rem=false): _data{data}, _add{add}, _rem{rem}
		{}

		bool operator==(const Msg& other) const {
			return _data == other._data && _add == other._add && _rem == other._rem;
		}

		int _data;
		bool _add;
		bool _rem;
};

ostream& operator<<(ostream& o, const Msg& m) {
	o << m._data << " " << m._add << " " << m._rem << endl;
	return o;
}

class ObserverMock: public Observer<Msg>
{
	public:
		typedef std::queue<Msg> MessageSequence;
		typedef MessageSequence::container_type MsgSeqCont;
		ObserverMock(MsgSeqCont s = MsgSeqCont()) : _s{s} {
		}

		ObserverMock(ObserverMock&) = delete;
		ObserverMock(ObserverMock&&) = delete; //unnecessary
		ObserverMock& operator=(ObserverMock&& other) {
			Observer<Msg>::operator=(std::move(other));
			std::swap(_s, other._s);
			return *this;
		}

		~ObserverMock() {
			EXPECT_EQ(_s.empty(), true);
		}

		MessageSequence _s;
	private:

		virtual void onObservableAdd(Observable_<Msg>& caller, const Msg& m) {
			onMsg(caller, m);
		}

		virtual void onObservableUpdate(Observable_<Msg>& caller, const Msg& m) {
			onMsg(caller, m);
		}

		virtual void onObservableRemove(Observable_<Msg>& caller, const Msg& m) {
			onMsg(caller, m);
		}

		void onMsg(Observable_<Msg>& caller, const Msg& m) {
			//cout << m;// << " (caller: " << &caller << ")\n";
			ASSERT_EQ(_s.empty(), false);
			ASSERT_EQ(m, _s.front());
			_s.pop();
		}
};

typedef ObserverMock::MsgSeqCont MsgSeq;

TEST(Observer, ObservableAdded) {
	Observable<Msg> observable(Msg(0, true), Msg(0, false, true));
	ObserverMock observer(MsgSeq{Msg(0, true)});

	observable.addObserver(observer);
}

TEST(Observer, ObservableUpdated) {
	Observable<Msg> observable(Msg(0, true), Msg(0, false, true));
	ObserverMock observer(MsgSeq{
			Msg(0, true),
			Msg(5),
			});

	observable.addObserver(observer);
	observable.notifyObservers(Msg(5));
}

TEST(Observer, ObservableDeath) {
	ObserverMock observer(MsgSeq{
			Msg(0, true),
			Msg(0, false, true),
			});
	{
		Observable<Msg> observable(Msg(0, true), Msg(0, false, true));
		observable.addObserver(observer);
	}
}

TEST(Observer, ObservableRemove) {
	ObserverMock observer(MsgSeq{
			Msg(0, true),
			Msg(0),
			Msg(0, false, true),
			});
	Observable<Msg> observable(Msg(0, true), Msg(0, false, true));
	observable.addObserver(observer);
	observable.notifyObservers(Msg(0));
	observable.removeObserver(observer);
	observable.notifyObservers(Msg(0));
}

TEST(Observer, ObserverMove) {
	// make sure observer dies AFTER observable, so it receives the onRemove message
	ObserverMock observer2;
	{
		Observable<Msg> observable(Msg(0, true), Msg(0, false, true));
		{
			ObserverMock observer(MsgSeq{
					Msg(0, true),
					});
			observable.addObserver(observer);
			observer2 = std::move(observer);
		}
		observer2._s.push(Msg(0));
		observer2._s.push(Msg(0, false, true));
		observable.notifyObservers(Msg(0));
	}
}

TEST(Observer, ObservableUpdateAfterObserverDead) {
	Observable<Msg> observable(Msg(0, true), Msg(0, false, true));
	{
		ObserverMock observer(MsgSeq{
				Msg(0, true),
				});
		observable.addObserver(observer);
	}
	// this shouldn't crash the program
	observable.notifyObservers(Msg(0)); 
}

TEST(Observabler, create) {
	Observabler<Msg> observabler(Msg(0, true), Msg(0, false, true));
}

TEST(Observabler, ObserverReceivesAddMsgsWhenObservablerAdded) {
	Observable<Msg> observable1(Msg(1, true), Msg(1, false, true));
	Observable<Msg> observable2(Msg(2, true), Msg(2, false, true));
	Observabler<Msg> observabler(Msg(0, true), Msg(0, false, true));
	// no remove messages because observer dies before observabler or observables
	ObserverMock observer(MsgSeq{
			Msg(1, true),
			Msg(2, true),
			});
	observable1.addObserver(observabler);
	observable2.addObserver(observabler);

	observabler.addObserver(observer);
}

TEST(Observabler, DeadObservedIgnored) { // ignored ...  since we cannot test if the dead observer was removed
	ObserverMock observer(MsgSeq{
			});
	Observabler<Msg> observabler;

	{
		Observable<Msg> observable(Msg(1, true), Msg(1, false, true));
		observable.addObserver(observabler);
	}

	observabler.addObserver(observer);
}

TEST(Observabler, ObservablerSendsNoAddMsg) {
	Observabler<Msg> observabler(Msg(0, true), Msg(0, false, true));
	// observer is declared later, therefore dies before observabler
	ObserverMock observer(MsgSeq{
			});

	observabler.addObserver(observer);
}

TEST(Observabler, ObservablerSendsNoRemMsg) {
	ObserverMock observer(MsgSeq{
			});
	// observabler is declared later, therefore dies before observer
	Observabler<Msg> observabler;

	observabler.addObserver(observer);
}

TEST(Observabler, ObserverReceivesRmMsgsWhenObservablerRemoved) {
	Observable<Msg> observable1(Msg(1, true), Msg(1, false, true));
	Observable<Msg> observable2(Msg(2, true), Msg(2, false, true));
	ObserverMock observer(MsgSeq{
			Msg(1, true),
			Msg(2, true),
			Msg(1, false, true),
			Msg(2, false, true),
			});

	{
		Observabler<Msg> observabler;
		observable1.addObserver(observabler);
		observable2.addObserver(observabler);

		observabler.addObserver(observer);
	}
}

TEST(Observabler, MoveConstruction) {
	Observabler<Msg> observabler1;
	Observable<Msg> observable1(Msg(1, true), Msg(1, false, true));
	Observable<Msg> observable2(Msg(2, true), Msg(2, false, true));
	ObserverMock observer(MsgSeq{
			Msg(1, true),
			Msg(2, true),
			Msg(1, false, true),
			Msg(2, false, true),
			});
	observable1.addObserver(observabler1);
	observable2.addObserver(observabler1);
	{
		Observabler<Msg> observabler2(std::move(observabler1));
		observabler2.addObserver(observer);
	}
}

TEST(Observabler, MoveAssignment) {
	ObserverMock observer(MsgSeq{
			Msg(1, true),
			Msg(2, true),
			Msg(1, false, true),
			Msg(2, false, true),
			});
	Observabler<Msg> observabler1;
	Observable<Msg> observable1(Msg(1, true), Msg(1, false, true));
	Observable<Msg> observable2(Msg(2, true), Msg(2, false, true));
	observable1.addObserver(observabler1);
	observable2.addObserver(observabler1);
	{
		Observabler<Msg> observabler2 = std::move(observabler1);
		observabler2.addObserver(observer);
	}
}

TEST(Observabler, ObservableDeath) {
	Observabler<Msg> observabler;
	Observable<Msg> observable(Msg(1, true), Msg(1, false, true));
	observable.addObserver(observabler);
}

TEST(Observabler, CopyConstruction) {
	ObserverMock observer(MsgSeq{
			Msg(1, true),
			Msg(2, true),

			Msg(1, true),
			Msg(2, true),

			Msg(1, false, true),
			Msg(2, false, true),

			Msg(1, false, true),
			Msg(2, false, true),
			});
	Observable<Msg> observable1(Msg(1, true), Msg(1, false, true));
	Observable<Msg> observable2(Msg(2, true), Msg(2, false, true));
	Observabler<Msg> observabler1;
	observable1.addObserver(observabler1);
	observable2.addObserver(observabler1);
	observabler1.addObserver(observer);
	{
		Observabler<Msg> observabler2(observabler1);
		observabler2.addObserver(observer);
	}
}

TEST(Observabler, CopyAssignment) {
	ObserverMock observer(MsgSeq{
			Msg(1, true),
			Msg(2, true),

			Msg(1, true),
			Msg(2, true),

			Msg(1, false, true),
			Msg(2, false, true),

			Msg(1, false, true),
			Msg(2, false, true),
			});
	Observable<Msg> observable1(Msg(1, true), Msg(1, false, true));
	Observable<Msg> observable2(Msg(2, true), Msg(2, false, true));
	Observabler<Msg> observabler1;
	observable1.addObserver(observabler1);
	observable2.addObserver(observabler1);
	observabler1.addObserver(observer);
	{
		Observabler<Msg> observabler2 = observabler1;
		observabler2.addObserver(observer);
	}
}
