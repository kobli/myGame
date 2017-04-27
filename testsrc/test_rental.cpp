#include <rental.hpp>
#include "gtest/gtest.h"

using namespace std;

TEST(Rental, create) {
	Rental<size_t> r;
}

TEST(Rental, borrow) {
	Rental<size_t> r;
	size_t i = r.borrow();
	ASSERT_EQ(i, 0);
}

TEST(Rental, borrowTwice) {
	Rental<size_t> r;
	size_t i = r.borrow();
	ASSERT_EQ(i, 0);
	i = r.borrow();
	ASSERT_EQ(i, 1);
}

TEST(Rental, remit) {
	Rental<size_t> r;
	size_t first = r.borrow();
	r.remit(first);
	size_t second = r.borrow();
	ASSERT_EQ(first, second);
}
