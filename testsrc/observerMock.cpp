#include "gtest/gtest.h"
#include "observerMock.hpp"

Msg::Msg(int data, bool add, bool rem): _data{data}, _add{add}, _rem{rem}
{}

bool Msg::operator==(const Msg& other) const {
	return _data == other._data && _add == other._add && _rem == other._rem;
}

std::ostream& operator<<(std::ostream& o, const Msg& m) {
	o << m._data << " " << m._add << " " << m._rem << std::endl;
	return o;
}


ObserverMock::ObserverMock(MsgSeqCont s) : _s{s} {
}

ObserverMock& ObserverMock::operator=(ObserverMock&& other) {
	Observer<Msg>::operator=(std::move(other));
	std::swap(_s, other._s);
	return *this;
}

ObserverMock::~ObserverMock() {
	EXPECT_EQ(_s.empty(), true);
}

void ObserverMock::onObservableAdd(Observable_<Msg>& caller, const Msg& m) {
	EXPECT_EQ(m._add, true);
	EXPECT_NE(m._rem, true);
	onMsg(caller, m);
}

void ObserverMock::onObservableUpdate(Observable_<Msg>& caller, const Msg& m) {
	EXPECT_NE(m._add, true);
	EXPECT_NE(m._rem, true);
	onMsg(caller, m);
}

void ObserverMock::onObservableRemove(Observable_<Msg>& caller, const Msg& m) {
	EXPECT_NE(m._add, true);
	EXPECT_EQ(m._rem, true);
	onMsg(caller, m);
}

void ObserverMock::onMsg(Observable_<Msg>&, const Msg& m) {
	EXPECT_EQ(_s.empty(), false);
	EXPECT_EQ(m, _s.front());
	_s.pop();
}
