#include <main.hpp>

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

