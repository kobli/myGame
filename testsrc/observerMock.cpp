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

