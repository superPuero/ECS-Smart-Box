#include "Registry.h"		
#include <iostream>


//basic component exmple
struct Transform {
	int x;
	int y;

	//Requires atleast default constructor
	Transform(int o, int t) noexcept {
		x = o;
		y = t;
	}

	//Requires atleast default move constructor
	Transform(Transform&& other) noexcept{
		x = other.x;
		y = other.y;
	}
	
	//Does not reqire copy constructor
	Transform(const Transform& other) = delete;

	// destructor is not required, but if there is any, it will be called on component removing or entity destructng properly
	// with that said, component may have internal heap allocations, such as malloc(), just make sure to write proper deallocation logic in component destructor
	inline ~Transform(){
		std::cout << "Transform destructed" << '\n';
	}
};

int main() {												   
	using namespace ctx;  // ctx is namespace that contains all of registry functionality

	Registry reg = Registry::create(); //static factory function for registry creation

	auto e1 = reg.entity(); //entity() method to create an entity, which in context of registry is just a 64-bit unsingned integer
	auto e2 = reg.entity();

	auto& tr1 = reg.emplace<Transform>(e1, 13, 54 ); //inplace constructing directly inside registry, better for performance
	auto& tr2 = reg.add(e2, Transform{ 98, 34 }); //inplace construct inside function and tham move of constructed object, slightly worse for performance
	auto& num = reg.emplace<int>(e2, 98);

	reg.remove<int>(e2);  //removing single component

	reg.destroy(e2); //removeing every component assosiated with e1 

	std::cout << tr1.x << " " << tr1.y << '\n'; //Undefined
	std::cout << tr2.x << " " << tr2.y << '\n';
	std::cout << reg.count() << '\n'; // count of valid entities inside registry

}