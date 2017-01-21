#include <irrlicht.h>
#include <cstdint>
#include <iostream>
#include <lua5.3/lua.hpp>

#ifndef MAIN_HPP_16_11_18_13_20_24
#define MAIN_HPP_16_11_18_13_20_24 

using namespace std;
using namespace irr;

template <typename T>
using vec2 = core::vector2d<T>;
template <typename T>
using vec3 = core::vector3d<T>;
using vec2f = core::vector2df;
using vec3f = core::vector3df;
using vec2i = core::vector2di;
using vec3i = core::vector3di;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
 
ostream& operator<<(ostream& os, vec2f v);
ostream& operator<<(ostream& os, vec3f v);
ostream& operator<<(ostream& os, vec2i v);
ostream& operator<<(ostream& os, vec3i v);

#endif /* MAIN_HPP_16_11_18_13_20_24 */
