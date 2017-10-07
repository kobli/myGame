cmake_minimum_required (VERSION 2.8)
project(myGame)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -w -Wall -Wextra -pedantic")

# set runtime library path to be relatively next to the executable
set(CMAKE_BUILD_WITH_INSTALL_RPATH true) 
set(CMAKE_INSTALL_RPATH "$ORIGIN")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH false)

set(${PROJECT_NAME}_VERSION_MAJOR 0)
set(${PROJECT_NAME}_VERSION_MINOR 3)
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
	${FREETYPE_INCLUDE_DIRS}
	/usr/X11R6/include
	${PROJECT_SOURCE_DIR}/extlibs/irrlicht-1.8.4/include
	${PROJECT_SOURCE_DIR}/extlibs/SFML-2.3.2/include
	${PROJECT_SOURCE_DIR}/extlibs/bullet3-2.83.7/include
	${PROJECT_SOURCE_DIR}/extlibs/bullet3-2.83.7/include/bullet
	${PROJECT_SOURCE_DIR}/extlibs/lua-5.3.4/include
	${PROJECT_SOURCE_DIR}/extlibs/freetype-2.8.1/include
	)
set(PLATFORM CACHE STRING "The destination platform.")
set(PLATFORMS lin_64 lin_32 win_gcc_32)
set_property(CACHE PLATFORM PROPERTY STRINGS ${PLATFORMS})
set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/extlibs")
link_directories(
	/usr/X11R6/lib64
	${PROJECT_SOURCE_DIR}/extlibs/SFML-2.3.2/lib/${PLATFORM}
	${PROJECT_SOURCE_DIR}/extlibs/irrlicht-1.8.4/lib/${PLATFORM}
	${PROJECT_SOURCE_DIR}/extlibs/lua-5.3.4/lib/${PLATFORM}
	${PROJECT_SOURCE_DIR}/extlibs/bullet3-2.83.7/lib/${PLATFORM}
	${PROJECT_SOURCE_DIR}/extlibs/freetype-2.8.1/lib/${PLATFORM}
	)


add_executable(${PROJECT_NAME} ${CPP_SOURCES})

set(LINUX_ONLY_LIBS
	sfml-system
	sfml-network
	GL
	Xxf86vm
	Xext
	X11
	Xcursor
	dl
	)

set(WINDOWS_ONLY_LIBS
	sfml-network-s
	sfml-system-s
	ws2_32
	winmm
	gdi32
	opengl32
	winmm
	gdi32
	ws2_32
	)

set(GENERAL_LIBS
	freetype
	Irrlicht
	lua
	BulletDynamics
	BulletCollision
	LinearMath
	)

if(${PLATFORM} MATCHES "lin_64")
	set(TARGET_OS "Linux")
elseif(${PLATFORM} MATCHES "lin_32")
	set(TARGET_OS "Linux")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
elseif(${PLATFORM} MATCHES "win_gcc_32")
	set(TARGET_OS "Windows")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_IRR_STATIC_LIB_ -DSFML_STATIC -static -static-libgcc -static-libstdc++")
else()
	message(FATAL_ERROR "PLATFORM variable not set. Possible values: ${PLATFORMS}")
endif()

if(${TARGET_OS} MATCHES "Linux")
	set(SYSTEM_SPECIFIC_LIBS ${LINUX_ONLY_LIBS})
	file(GLOB SHARED_LIBS 
		"${PROJECT_SOURCE_DIR}/extlibs/SFML-2.3.2/lib/${PLATFORM}/*.so*"
		)
	file(COPY ${SHARED_LIBS} DESTINATION ${PROJECT_BINARY_DIR})
	set(GENERAL_PACKAGE_FILES
		"libsfml-system.so*"
		"libsfml-network.so*"
		)
elseif(${TARGET_OS} MATCHES "Windows")
	set(SYSTEM_SPECIFIC_LIBS ${WINDOWS_ONLY_LIBS})
	set(CLIENT_ONLY_PACKAGE_FILES "run.bat")
endif()

target_link_libraries(${PROJECT_NAME}
	${GENERAL_LIBS}
	${SYSTEM_SPECIFIC_LIBS}
	)

set(GENERAL_PACKAGE_FILES
	${GENERAL_PACKAGE_FILES}
	"$<TARGET_FILE:${PROJECT_NAME}>"
	"${PROJECT_SOURCE_DIR}/media"
	"${PROJECT_SOURCE_DIR}/controls.lua"
	"${PROJECT_SOURCE_DIR}/keymap.lua" 
	"${PROJECT_SOURCE_DIR}/spellBook.lua"
	)
set(SERVER_ONLY_PACKAGE_FILES
	"${PROJECT_SOURCE_DIR}/lua/spellSystem.lua" 
	"${PROJECT_SOURCE_DIR}/lua/gamemode_dm.lua"
	"${PROJECT_SOURCE_DIR}/lua/list.lua"
	)

file(WRITE "${PROJECT_BINARY_DIR}/run.bat" "myGame.exe -c -a kobl.eu -p 55555")
add_custom_target(package-client COMMAND
	${CMAKE_COMMAND} -E tar "cvf" "${PROJECT_NAME}-client-${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}-${PLATFORM}.zip" --format=zip ${GENERAL_PACKAGE_FILES} ${CLIENT_ONLY_PACKAGE_FILES}
	)
add_custom_target(package-server COMMAND
	${CMAKE_COMMAND} -E tar "cvf" "${PROJECT_NAME}-server-${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}-${PLATFORM}.zip" --format=zip ${GENERAL_PACKAGE_FILES} ${SERVER_ONLY_PACKAGE_FILES}
	)
