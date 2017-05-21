#include <solidVector.hpp>
#include "gtest/gtest.h"
#include <functional>

using namespace std;

TEST(SolidVector, create) {
	SolidVector<int> s;
}

TEST(SolidVector, insertOne) {
	SolidVector<int> s;
	size_t i = s.insert(5);
	ASSERT_EQ(i, 0);
}

TEST(SolidVector, insertAndGet) {
	SolidVector<int> s;
	size_t i = s.insert(5);
	ASSERT_EQ(s.at(i), 5);
}

TEST(SolidVector, slotReuse) {
	SolidVector<int> s;
	size_t i = s.insert(1);
	s.insert(2);
	s.remove(i);
	size_t iii = s.insert(3);
	ASSERT_EQ(i, iii);
	ASSERT_EQ(s.at(iii), 3);
}

TEST(SolidVector, slotReuse2) {
	SolidVector<int> s;
	size_t i = s.insert(4);
	size_t ii = s.insert(5);
	s.remove(i);
	size_t iii = s.insert(6);
	ASSERT_EQ(i, iii);
	ASSERT_EQ(i, 0);
	ASSERT_EQ(ii, 1);
	ASSERT_EQ(iii, 0);
	ASSERT_EQ(s.at(i), 6);
	ASSERT_EQ(s.at(ii), 5);
	ASSERT_EQ(s.at(iii), 6);
}

void expectOutOfRange(std::function<void(void)> f) {
	try {
		f();
		FAIL() << "Expected std::out_of_range";
	}
	catch(std::out_of_range expected) {
	}
	catch(...) {
		FAIL() << "Expected std::out_of_range";
	}
}

TEST(SolidVector, accessRemoved) {
	SolidVector<int> s;
	size_t i = s.insert(4);
	s.remove(i);
	expectOutOfRange([&s, i]{ s.at(i); });
}

TEST(SolidVector, accessNonExisting) {
	SolidVector<int> s;
	expectOutOfRange([&s]{ s.at(42); });
}

TEST(SolidVector, removeNonExisting) {
	SolidVector<int> s;
	expectOutOfRange([&s]{ s.remove(42); });
}

TEST(SolidVector, removeRemoved) {
	SolidVector<int> s;
	size_t i = s.insert(4);
	expectOutOfRange([i, &s]{ s.remove(i); s.remove(i); });
}

TEST(SolidVector, iterate) {
	SolidVector<int> s;
	vector<int> v{4,5,6};
	s.insert(4);
	s.insert(5);
	s.insert(6);
	int i = 0;
	for(auto& e : s) {
		ASSERT_EQ(v[i], e);
		i++;
	}
}

TEST(SolidVector, indexValidFalse) {
	SolidVector<int> s;
	ASSERT_EQ(false, s.indexValid(0));
	ASSERT_EQ(false, s.indexValid(364));
	ASSERT_EQ(false, s.indexValid(32));
}

TEST(SolidVector, indexValidTrue) {
	SolidVector<int> s;
	s.insert(34);
	s.insert(4);
	s.insert(3);
	ASSERT_EQ(true, s.indexValid(0));
	ASSERT_EQ(true, s.indexValid(1));
	ASSERT_EQ(true, s.indexValid(2));
}
