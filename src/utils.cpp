#include "main.hpp"

ostream& operator<<(ostream& os, i8 i)
{
	return os << int(i);
}

ostream& operator<<(ostream& os, vec2u v) {
	return os << v.X << " " << v.Y;
}

ostream& operator<<(ostream& os, vec2f v) {
	return os << v.X << " " << v.Y;
}

ostream& operator<<(ostream& os, vec3f v) {
	return os << v.X << " " << v.Y << " " << v.Z;
}

ostream& operator<<(ostream& os, vec2i v) {
	return os << v.X << " " << v.Y;
}

ostream& operator<<(ostream& os, vec3i v) {
	return os << v.X << " " << v.Y << " " << v.Z;
}

ostream& operator<<(ostream& os, quaternion q) {
	return os << q.X << " " << q.Y << " " << q.Z << " " << q.W;
}

ostream& operator<<(ostream& os, btVector3 v) {
	return os << v.x() << " " << v.y() << " " << v.z();
}

vec3f btV3f2V3f(btVector3 v)
{
	return vec3f(v.x(), v.y(), v.z());
}
btVector3 V3f2btV3f(vec3f v)
{
	return btVector3(v.X, v.Y, v.Z);
}

quaternion btQ2Q(btQuaternion q)
{
	return quaternion(q.x(), q.y(), q.z(), q.w());
}
btQuaternion Q2btQ(quaternion q)
{
	return btQuaternion(q.X, q.Y, q.Z, q.W);
}

std::ostream& operator<<(std::ostream&, const Table& t)
{
	for(const auto& r : t)
		std::cout << r.first << "\t\t" << r.second << std::endl;
}

std::string lua_valueAsStr(lua_State* L, int index)
{
	if(lua_isinteger(L, index))
		return std::to_string(lua_tointeger(L, index));
	else if(lua_isnumber(L, index))
		return std::to_string(lua_tonumber(L, index));
	else if(lua_isstring(L, index))
		return lua_tostring(L, index);
	else
		return "";
}

Table lua_loadTable(lua_State* l, int index)
{
	Table t;
	lua_pushnil(l);
	if(index < 0)
		index--;
	while(lua_next(l, index) != 0)
	{
		std::string key = lua_valueAsStr(l, -2),
			value = lua_valueAsStr(l, -1);
		t.push_back(std::make_pair(key, value));
		lua_pop(l, 1);
	}
	return t;
}
