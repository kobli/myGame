#include <observer.hpp>
#include "gtest/gtest.h"
using namespace std;
/*
TODO automatic tests
class Msg
{
	public:
		Msg(bool add=false, bool rem=false): _add{add}, _rem{rem}
		{}

		bool _add;
		bool _rem;
};

class ObserverImpl: public Observer<Msg>
{
		void onObservableAdd(Msg& m)
		{
			cout << "OnOAdd\n";
			EXPECT_EQ(true, m._add);
		}

		void onObservableUpdate(Msg& m)
		{
			cout << "OnOUpd\n";
		}

		void onObservableRemove(Msg& m)
		{
			cout << "OnORem\n";
			EXPECT_EQ(true, m._rem);
		}
};

TEST(Observer, AddAndUpdate) {
	Observable<Msg> oa(Msg(true), Msg(false, true));
	ObserverImpl o;
	Msg m;

	o.observe(oa);
	oa.notifyObservers(m);
}

TEST(Observer, UpdateAfterDestr) {
	Observable<Msg> oa(Msg(true), Msg(false, true));
	Msg m;
	{
		ObserverImpl o;

		o.observe(oa);
	}
	oa.notifyObservers(m);
}

TEST(Observer, ObservableDies) {
	ObserverImpl o;
	Observable<Msg> oa1(Msg(true), Msg(false, true));
	o.observe(oa1);
	Msg m;
	{
		cout << "start of block\n";
		Observable<Msg> oa2(Msg(true), Msg(false, true));
		o.observe(oa2);
		cout << "end of block\n";
	}
	cout << "end of func\n";
}

TEST(Observer, ObserverDies) {
	Observable<Msg> oa1(Msg(true), Msg(false, true));
	ObserverImpl o;
	o.observe(oa1);
	Msg m;
	{
		cout << "start of block\n";
		ObserverImpl o;
		o.observe(oa1);
		cout << "oa - two os updated\n";
		oa1.notifyObservers(m);
		cout << "end of block\n";
	}
	cout << "oa - only one o updated\n";
	oa1.notifyObservers(m);
	cout << "end of func\n";
}
*/
