.SUFFIXES:

PROJECT = myGame
# ! the builddir is deleted on clean
BUILDDIR = build
SRCDIR = src
CPPFLAGS = -std=c++14 -g -w #-Wall -Wextra -pedantic
INCLUDES = -Iinclude -I/usr/X11R6/include -Iextlibs/irrlicht-1.8.4/include -Iextlibs/SFML-2.3.2/include -Iextlibs/bullet3-2.83.7/include -Iextlibs/bullet3-2.83.7/include/bullet -Iextlibs/lua-5.3.4/include

ifeq ($(MAKECMDGOALS), lin64)
	TARGET = $(PROJECT)
	CXX = g++
	LIBS = \
				 -L/usr/X11R6/lib64 \
				 -Lextlibs/SFML-2.3.2/lib/lin_64 \
				 -Lextlibs/irrlicht-1.8.4/lib/lin_64 \
				 -Lextlibs/lua-5.3.4/lib/lin_64 \
				 -Lextlibs/bullet3-2.83.7/lib/lin_64 \
				 -lIrrlicht -lGL -lXxf86vm -lXext -lX11 -lXcursor -lsfml-system -lsfml-network -llua -lBulletDynamics -lBulletCollision -lLinearMath -ldl
	else

	ifeq ($(MAKECMDGOALS), win32-static)
	TARGET = $(PROJECT).exe
	CPPFLAGS += -D_IRR_STATIC_LIB_  -DSFML_STATIC
	CXX = i686-w64-mingw32-g++
	LIBS = \
				 -L/usr/i686-w64-mingw32/lib \
				 -Lextlibs/irrlicht-1.8.4/lib/win_gcc_32 \
				 -Lextlibs/SFML-2.3.2/lib/win_gcc_32 \
				 -Lextlibs/lua-5.3.4/lib/win_gcc_32 \
				 -Lextlibs/bullet3-2.83.7/lib/win_gcc_32 \
				 -lIrrlicht \
				 -lsfml-network-s -lsfml-system-s \
				 -lws2_32 -static -static-libgcc -static-libstdc++ -lwinmm -lgdi32 \
				 -lopengl32 -lwinmm -lgdi32 -lws2_32 \
				 -lBulletDynamics -lBulletCollision -lLinearMath \
				 -llua
	endif
endif

########## end of user settings #########

build-default:
	@$(MAKE) lin64

win32-static lin64: $(TARGET)


test: build-test
	./test

build-test: testsrc/* src/entity.cpp
	$(CXX) $(CPPFLAGS) $(INCLUDES) -o test $^ -lpthread -lgtest -lgtest_main



SRCS = $(shell cd $(SRCDIR)>/dev/null && ls *.cpp && cd - >/dev/null)
OBJS = $(subst .cpp,.o,$(SRCS))

$(TARGET): $(SRCS:%.cpp=$(BUILDDIR)/%.depend) $(SRCS:%.cpp=$(BUILDDIR)/%.o)
	$(CXX) $(CPPFLAGS) $(OBJS:%=$(BUILDDIR)/%) -o $(TARGET) $(LIBS)

$(BUILDDIR)/%.depend: $(SRCDIR)/%.cpp
	@mkdir -p $(BUILDDIR)
	rm -f $@
	printf '$(BUILDDIR)/' >> $@
	$(CXX) $(CPPFLAGS) $(INCLUDES) -MM $^>>$@;
	printf '\t$(CXX) $(CPPFLAGS) $(INCLUDES) -c $< -o $(subst .depend,.o,$@)' >> $@

clean:
	rm -r $(BUILDDIR)

ifneq ($(MAKECMDGOALS),clean)
-include $(SRCS:%.cpp=$(BUILDDIR)/%.depend)
endif
