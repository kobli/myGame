#include <main.hpp>

ostream& operator<<(ostream& os, i8 i)
{
	return os << int(i);
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
