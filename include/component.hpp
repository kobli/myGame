#ifndef COMPONENT_HPP_17_04_24_17_41_47
#define COMPONENT_HPP_17_04_24_17_41_47 

namespace Component {
		typedef unsigned ID;
}

enum ComponentType {
	t1,
	t2,
};

class ComponentBase {

};

class Component1 : public ComponentBase {

};

class Component2 : public ComponentBase {

};
#endif /* COMPONENT_HPP_17_04_24_17_41_47 */
