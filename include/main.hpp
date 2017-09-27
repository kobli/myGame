#include <irrlicht.h>
#include <bullet/btBulletCollisionCommon.h>
#include <cstdint>
#include <iostream>
#include <lua5.3/lua.hpp>
#include <vector>
#include <functional>
#include "config.hpp"
#include <map>

#ifndef MAIN_HPP_16_11_18_13_20_24
#define MAIN_HPP_16_11_18_13_20_24 

using namespace std;
using namespace irr;

constexpr float PI = 3.1415926535;
constexpr float PI_2 = PI/2;

template <typename T>
using vec2 = core::vector2d<T>;
template <typename T>
using vec3 = core::vector3d<T>;
using vec2f = core::vector2df;
using vec2u = core::vector2d<u32>;
using vec3f = core::vector3df;
using vec2i = core::vector2di;
using vec3i = core::vector3di;
using quaternion = core::quaternion;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
 
ostream& operator<<(ostream& os, i8 i);
ostream& operator<<(ostream& os, vec2u v);
ostream& operator<<(ostream& os, vec2f v);
ostream& operator<<(ostream& os, vec3f v);
ostream& operator<<(ostream& os, vec2i v);
ostream& operator<<(ostream& os, vec3i v);
ostream& operator<<(ostream& os, quaternion q);
ostream& operator<<(ostream& os, btVector3 v);

vec3f btV3f2V3f(btVector3 v);
btVector3 V3f2btV3f(vec3f v);
quaternion btQ2Q(btQuaternion q);
btQuaternion Q2btQ(quaternion q);

typedef std::vector<std::pair<std::string,std::string>> Table;
std::ostream& operator<<(std::ostream&, const Table&);

std::string lua_valueAsStr(lua_State* L, int index);
Table lua_loadTable(lua_State* l, int index);

class ImageDumper {
	public:
		typedef std::function<unsigned(unsigned x, unsigned y)> PixValGetter;
		inline ImageDumper(irr::video::IVideoDriver* driver): _driver{driver}
		{}

		inline bool operator()(std::string fileName, PixValGetter pixVal, irr::core::vector2d<unsigned> imSize) const
		{
			using namespace irr;
			auto im = _driver->createImage(video::ECF_R8G8B8, core::dimension2du(imSize));
			for(unsigned y = 0; y < imSize.Y; ++y)
				for(unsigned x = 0; x < imSize.X; ++x) {
					unsigned h = pixVal(imSize.X-x-1,y);
					im->setPixel(x,y, irr::video::SColor(255, h, h, h));
				}
			return _driver->writeImageToFile(im, fileName.c_str());
		}

	private:
		irr::video::IVideoDriver* _driver;
};

template <typename T>
std::ostream& operator<<(std::ostream& o, std::vector<T> v) {
	for(auto& e : v)
		o << e << ", ";
	o << std::endl;
}

template <typename K, typename V>
std::ostream& operator <<(std::ostream& t, const std::map<K,V>& m) {
	t << static_cast<u32>(m.size());
	t << " ";
	for(auto& p : m)
		t << p;
	return t;
}
template <typename K, typename V>
std::ostream& operator >>(std::ostream& /*t*/, std::map<K,V>& /*m*/) {
	assert(false); //TODO
}

template <typename K, typename V>
std::ostream& operator <<(std::ostream& o, const std::pair<K,V>& p) {
	return o << p.first << ": " << p.second << "; ";
}
template <typename K, typename V>
std::ostream& operator >>(std::ostream& /*o*/, std::pair<K,V>& /*p*/) {
	assert(false); //TODO
}


extern ImageDumper SAVEIMAGE;
#endif /* MAIN_HPP_16_11_18_13_20_24 */
