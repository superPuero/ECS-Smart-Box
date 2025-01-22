#include "Registry.h"		
#include <iostream>


//basic component exmple
struct Transform {
	int x;
	int y;

	// requires atleast default constructor
	Transform(int o, int t) noexcept {
		x = o;
		y = t;
	}

	// requires atleast default move constructor
	Transform(Transform&& other) noexcept {
		x = other.x;
		y = other.y;
	}

	// does not reqire copy constructor
	Transform(const Transform& other) = delete;

	friend std::ostream& operator<<(std::ostream& os, const Transform& tr) {
		os << "x: " << tr.x << " y: " << tr.y;
		return os;
	}

	// destructor is not required, but if there is any, it will be called on component removing or entity destructng properly
	// with that said, component may have internal heap allocations, such as malloc(), just make sure to write proper deallocation logic in component destructor
	inline ~Transform() {
		std::cout << "Transform destructed" << '\n';
	}
};

int main() {
	using namespace ctx;  // ctx is namespace that contains all of registry functionality

	Registry reg = Registry::create(); // static factory function for registry creation

	auto e1 = reg.entity(); // entity() method to create an entity, which in context of registry is just a 64-bit unsingned integer
	auto e2 = reg.entity();
	auto e3 = reg.entity();


	auto& tr1 = reg.emplace<Transform>(e1, 13, 54); // inplace constructing directly inside registry, better for performance
	auto& tr2 = reg.add(e2, Transform{ 98, 34 }); // inplace construct inside function and tham move of constructed object, slightly worse for performance
	auto& num = reg.emplace<int>(e1, 98);
	auto& num2 = reg.emplace<int>(e2, 8);
	auto& fl = reg.emplace<float>(e1, 123.4f);


	reg.remove<int>(e2);  // removing single component

	reg.destroy(e2); // destroying whole e2 entity with every assosiated component

	std::cout << tr1 << '\n';
	std::cout << "UNDEFINED: " << tr2 << '\n';	// undefined
	std::cout << reg.count() << '\n'; // count of valid entities inside registry

	// group method yileds vector if entity ids, that have every specified component
	for (EntityId id : reg.group<Transform, int, float>()) {
		std::cout << "id: " << id << "\n";
	}

	// query method returns iterable set of tuples of specified component refrences  
	// each tuple corresponds to one entity, yields empty iterable if none 
	for (auto& [transform, inum] : reg.query<Transform, int>()) {
		transform.x += 4;
		transform.y *= 2;
	}

	// task method takes function, that will be applied to each enitity id that has specified components(uses group method)
	// lambda and must take exactly one argument of EntityId type(or auto)
	// execution is not parallel
	reg.task<Transform, int>(
		[&](auto id) 
		{
			reg.emplace<char>(id, 'f');
		});

	// transform method takes funcion, that will be applied to each tuple of specified component references(uses query method)
	// lambda must be non-capturing and take exactly one argument of std::tuple<Args&...> type(or auto&)
	// prefer transform over manual iterator with query method, because transform executes in parallel
	reg.transform<Transform, int, char>(
		[](auto& tuple)
		{
			// to get access to each individual component in tuple, use structured bindings
			auto& [transform, i, ch] = tuple;

			transform.x *= -1;

			std::cout << "int: " << i << " char: " << ch << '\n';

		});
}