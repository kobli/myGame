cmake_minimum_required (VERSION 2.8)
project(myGame)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -w -Wall -Wextra -pedantic")


set(${PROJECT_NAME}_VERSION_MAJOR 0)
set(${PROJECT_NAME}_VERSION_MINOR 1)
configure_file(
	"${PROJECT_SOURCE_DIR}/include/config.hpp.in"
	"${PROJECT_BINARY_DIR}/config.hpp"
	)
include_directories("${PROJECT_BINARY_DIR}")


set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -DDEBUG_BUILD")


file(GLOB CPP_SOURCES
	"${PROJECT_SOURCE_DIR}/src/*.cpp"
	)
include_directories(
	${PROJECT_SOURCE_DIR}/include
	/usr/X11R6/include
	${PROJECT_SOURCE_DIR}/extlibs/irrlicht-1.8.4/include
	${PROJECT_SOURCE_DIR}/extlibs/SFML-2.3.2/include
	${PROJECT_SOURCE_DIR}/extlibs/bullet3-2.83.7/include
	${PROJECT_SOURCE_DIR}/extlibs/bullet3-2.83.7/include/bullet
	${PROJECT_SOURCE_DIR}/extlibs/lua-5.3.4/include
	)
set(PLATFORM CACHE STRING "The destination platform.")
set_property(CACHE PLATFORM PROPERTY STRINGS lin_64 lin_32 win_gcc_32)
set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/extlibs")
link_directories(
	/usr/X11R6/lib64
	${PROJECT_SOURCE_DIR}/extlibs/SFML-2.3.2/lib/${PLATFORM}
	${PROJECT_SOURCE_DIR}/extlibs/irrlicht-1.8.4/lib/${PLATFORM}
	${PROJECT_SOURCE_DIR}/extlibs/lua-5.3.4/lib/${PLATFORM}
	${PROJECT_SOURCE_DIR}/extlibs/bullet3-2.83.7/lib/${PLATFORM}
	)


add_executable(${PROJECT_NAME} ${CPP_SOURCES})

set(LINUX_ONLY_LIBS
	GL
	Xxf86vm
	Xext
	X11
	Xcursor
	dl
	)

set(WINDOWS_ONLY_LIBS
	ws2_32
	gcc
	stdc++
	winmm
	gdi32
	opengl32
	winmm
	gdi32
	ws2_32
	)

set(GENERAL_LIBS
	Irrlicht
	sfml-system
	sfml-network
	lua
	BulletDynamics
	BulletCollision
	LinearMath
	)

if(${PLATFORM} MATCHES "lin_64")
	set(SYSTEM_SPECIFIC_LIBS ${LINUX_ONLY_LIBS})
elseif(${PLATFORM} MATCHES "lin_32")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
	set(SYSTEM_SPECIFIC_LIBS ${LINUX_ONLY_LIBS})
elseif(${PLATFORM} MATCHES "win_gcc_32")
	set(SYSTEM_SPECIFIC_LIBS ${WINDOWS_ONLY_LIBS})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_IRR_STATIC_LIB_ -DSFML_STATIC -static")
endif()

target_link_libraries(${PROJECT_NAME}
	${GENERAL_LIBS}
	${SYSTEM_SPECIFIC_LIBS}
	)
