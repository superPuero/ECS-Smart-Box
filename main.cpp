#include "Registry.h"		
#include <iostream>

struct Transform {
	int x;
	int y;

	inline ~Transform(){
		std::cout << "Transform destructed" << '\n';
	}
};

int main() {												   
	using namespace ctx;

	Registry reg = Registry::create();

	auto e1 = reg.entity();
	auto e2 = reg.entity();

	auto& tr1 = reg.emplace<Transform>(e1, 13, 54 );

	auto& tr2 = reg.add(e2, Transform{ 98, 34 });

	std::cout << "foo\n";

	reg.destroy(e1);

	std::cout << "foo\n";

	std::cout << tr1.x << " " << tr1.y << '\n'; //Undefined
	std::cout << tr2.x << " " << tr2.y << '\n';
	std::cout << reg.count() << '\n';

}