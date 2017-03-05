BUILDDIR = build
SRCDIR = src
CPPFLAGS = -std=c++11 #-g -w #-Wall -Wextra -pedantic
INCLUDES = -Iinclude -I../../irrlicht-1.8.4/include -I/usr/X11R6/include -I../../SFML-2.3.2/include -I../../SFML-2.3.2/bin/include -I../../bullet3-2.83.7/src -I../../bullet3-2.83.7 -I../../lua-5.3.4

TARGET = myGame
CXX = g++
LIBS = -L/usr/X11R6/lib64 -L../../SFML-2.3.2/lib -L../../irrlicht-1.8.4/lib/Linux -L../../lua-5.3.4/lib-lin -L../../bullet3-2.83.7/lib-lin -lIrrlicht -lGL -lXxf86vm -lXext -lX11 -lXcursor -lsfml-system -lsfml-network -llua -lBulletDynamics -lBulletCollision -lLinearMath -ldl

#TARGET = myGame.exe
#CXX = i686-w64-mingw32-g++
#LIBS = \
			 -L../../irrlicht-1.8.4/lib/Win32-gcc -lIrrlicht \
			 -l:../../SFML-2.3.2/extlibs/libs-mingw/x64/libjpeg.a \
			 -L../../SFML-2.3.2/lib-win -L../../SFML-2.3.2/extlibs/libs-mingw/x64 -lsfml-network-s -lsfml-system-s \
			 -L/usr/i686-w64-mingw32/lib -lws2_32 -static -static-libgcc -static-libstdc++ -lwinmm -lgdi32 \
			 -lopengl32 -lwinmm -lgdi32 -lws2_32 \
			 -L../../bullet3-2.83.7/lib-win -lBulletDynamics -lBulletCollision -lLinearMath \
			 -L../../lua-5.3.4/lib-win -llua
#CPPFLAGS += -D_IRR_STATIC_LIB_  -DSFML_STATIC



SRCS = $(shell cd $(SRCDIR)>/dev/null && ls *.cpp && cd - >/dev/null)
OBJS = $(subst .cpp,.o,$(SRCS))

build: $(TARGET)

quicktest: FORCE
	lua5.3 ./lua/sandbox.lua

FORCE:


test: tests
	./test

tests: testsrc/*
	$(CXX) $(CPPFLAGS) $(INCLUDES) -o test testsrc/* -lpthread -lgtest -lgtest_main



$(TARGET): $(SRCS:%.cpp=$(BUILDDIR)/%.depend) $(SRCS:%.cpp=$(BUILDDIR)/%.o) 
	$(CXX) $(CPPFLAGS) $(OBJS:%=$(BUILDDIR)/%) -o $@ $(LIBS)


$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp $(BUILDDIR)/%.depend
	$(CXX) $(CPPFLAGS) $(INCLUDES) $^ -c -o $@

%.depend: $(BUILDDIR)/%.depend 

$(BUILDDIR)/%.depend: $(SRCDIR)/%.cpp
	rm -f $@
	printf '$(BUILDDIR)/' >> $@
	$(CXX) $(CPPFLAGS) $(INCLUDES) -MM $^>>$@;
	printf '\t$(CXX) $(CPPFLAGS) $(INCLUDES) -c $< -o $(subst .depend,.o,$@)' >> $@

clean:
	rm $(BUILDDIR)/* $(TARGET)

ifneq ($(MAKECMDGOALS),clean)
include $(SRCS:%.cpp=$(BUILDDIR)/%.depend)
endif
