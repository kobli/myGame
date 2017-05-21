#include "gtest/gtest.h"
#include "reallocable.hpp"
#include <iostream>

class ObjMock {
	public:
		ObjMock() {
			_ID = _nextID;
			_nextID++;
		}

		ObjMock(ObjMock& other) {
			_ID = _nextID;
			_nextID++;
			_class = other._class;
		}

		ObjMock& operator=(ObjMock& other) {
			_class = other._class;
			return *this;
		}

		ObjMock(ObjMock&& other) noexcept {
			swap(other);
		}

		ObjMock& operator=(ObjMock&& other) {
			swap(other);
			return *this;
		}

		~ObjMock() {
		}

		virtual void swap(ObjMock& o) {
			using std::swap;
			swap(_ID, o._ID);
			swap(_class, o._class);
		}


		int getID() {
			return _ID;
		}

		int getClass() {
			return _class;
		}

	private:
		static int _nextID;
		int _ID;
		static int _nextClass;
		int _class;
};

void swap(ObjMock& lhs, ObjMock& rhs) {
	lhs.swap(rhs);
}

int ObjMock::_nextID = 0;
int ObjMock::_nextClass = 0;




TEST(Reallocable, construction) {
	Reallocable<ObjMock> o;
}

TEST(Reallocable, swap) {
	Reallocable<ObjMock> o1;
	Reallocable<ObjMock> o2;
	auto o1Ref = o1.getSelf();
	int o1ID = o1.getID();

	swap(o1, o2);

	ASSERT_EQ((*o1Ref.lock())->getID(), o1ID);
}

TEST(Reallocable, moveConstruction) {
	Reallocable<ObjMock> o1;
	auto o1Ref = o1.getSelf();
	int o1ID = o1.getID();

	Reallocable<ObjMock> o2(std::move(o1));

	ASSERT_EQ((*o1Ref.lock())->getID(), o1ID);
}

TEST(Reallocable, moveAssignment) {
	Reallocable<ObjMock> o1;
	auto o1Ref = o1.getSelf();
	int o1ID = o1.getID();
	Reallocable<ObjMock> o2;

	o2 = std::move(o1);

	ASSERT_EQ((*o1Ref.lock())->getID(), o1ID);
}

TEST(Reallocable, copyConstruction) {
	Reallocable<ObjMock> o1;
	auto o1Ref = o1.getSelf();
	int o1Class = o1.getClass();

	Reallocable<ObjMock> o2(o1);

	ASSERT_EQ((*o1Ref.lock())->getClass(), o1Class);
	ASSERT_NE((*o2.getSelf().lock())->getID(),
						(*o1.getSelf().lock())->getID());
}

TEST(Reallocable, copyAssignment) {
	Reallocable<ObjMock> o1;
	auto o1Ref = o1.getSelf();
	int o1Class = o1.getClass();
	Reallocable<ObjMock> o2;

	o2 = o1;

	ASSERT_EQ((*o1Ref.lock())->getClass(), o1Class);
	ASSERT_NE((*o2.getSelf().lock())->getID(),
						(*o1.getSelf().lock())->getID());
}
