#include "Registry.h"		
#include <iostream>
#include <variant>
#include <array>

struct Transform {
	int x;
	int y;
	inline ~Transform(){
		std::cout << "Transform died" << '\n';
	}
};


int main() {												   
	using namespace ctx;

	Registry reg = Registry::create();

	auto e = reg.entity();

	auto& tr = reg.add(e, Transform{ 13, 54 });

	std::cout << tr.x << "\n" << tr.y << '\n';
}