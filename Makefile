BUILDDIR = build
SRCDIR = src
TARGET = myGame
CPPFLAGS = -std=c++11 -g -w #-Wall -Wextra -pedantic
LIBS = -L/usr/X11R6/lib64 -L../../irrlicht-1.8.3/lib/Linux -lIrrlicht -lGL -lXxf86vm -lXext -lX11 -lXcursor -lsfml-system -lsfml-network -llua5.3 -lBulletDynamics -lBulletCollision -lLinearMath
INCLUDES = -Iinclude -I../../irrlicht-1.8.3/include -I/usr/X11R6/include -I/usr/include/bullet

SRCS = $(shell cd $(SRCDIR)>/dev/null && ls *.cpp && cd - >/dev/null)
OBJS = $(subst .cpp,.o,$(SRCS))

build: $(TARGET)

quicktest: FORCE
	lua5.3 ./lua/sandbox.lua

FORCE:


test: tests
	./test

tests: testsrc/*
	g++ $(CPPFLAGS) $(INCLUDES) -o test testsrc/* -lpthread -lgtest -lgtest_main



$(TARGET): $(SRCS:%.cpp=$(BUILDDIR)/%.depend) $(SRCS:%.cpp=$(BUILDDIR)/%.o) 
	g++ $(CPPFLAGS) $(OBJS:%=$(BUILDDIR)/%) -o $@ $(LIBS)


$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp $(BUILDDIR)/%.depend
	g++ $(CPPFLAGS) $(INCLUDES) $^ -c -o $@

%.depend: $(BUILDDIR)/%.depend 

$(BUILDDIR)/%.depend: $(SRCDIR)/%.cpp
	rm -f $@
	printf '$(BUILDDIR)/' >> $@
	g++ $(CPPFLAGS) $(INCLUDES) -MM $^>>$@;
	printf '\tg++ $(CPPFLAGS) $(INCLUDES) -c $< -o $(subst .depend,.o,$@)' >> $@

clean:
	rm $(BUILDDIR)/* $(TARGET)

ifneq ($(MAKECMDGOALS),clean)
include $(SRCS:%.cpp=$(BUILDDIR)/%.depend)
endif
