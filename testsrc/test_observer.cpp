#include <observer.hpp>
#include "gtest/gtest.h"
#include "observerMock.hpp"

typedef ObserverMock_<Msg> ObserverMock;
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
	observable.broadcastMsg(Msg(5));
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
	observable.broadcastMsg(Msg(0));
	observable.removeObserver(observer);
	observable.broadcastMsg(Msg(0));
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
		observable.broadcastMsg(Msg(0));
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
	observable.broadcastMsg(Msg(0)); 
}

TEST(Observabler, create) {
	Observabler<Msg> observabler(Msg(0, true), Msg(0, false, true));
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

TEST(Observabler, ObservablerSendsAddMsg) {
	Observabler<Msg> observabler(Msg(0, true), Msg(0, false, true));
	// observer is declared later, therefore dies before observabler
	ObserverMock observer(MsgSeq{
			Msg(0, true),
			});

	observabler.addObserver(observer);
}

TEST(Observabler, ObservablerSendsRemMsg) {
	ObserverMock observer(MsgSeq{
			Msg(0, true),
			Msg(0, false, true),
			});
	// observabler is declared later, therefore dies before observer
	Observabler<Msg> observabler(Msg(0, true), Msg(0, false, true));

	observabler.addObserver(observer);
}

TEST(Observabler, ObserverReceivesAddMsgsWhenObservablerAdded) {
	Observable<Msg> observable1(Msg(1, true), Msg(1, false, true));
	Observable<Msg> observable2(Msg(2, true), Msg(2, false, true));
	Observabler<Msg> observabler(Msg(0, true), Msg(0, false, true));
	// no remove messages because observer dies before observabler or observables
	ObserverMock observer(MsgSeq{
			Msg(0, true),
			Msg(1, true),
			Msg(2, true),
			});
	observable1.addObserver(observabler);
	observable2.addObserver(observabler);

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
