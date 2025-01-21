#include "Registry.h"		
#include <iostream>
#include <variant>
#include <array>

struct Transform {
	float x;
	float y;
};

class Foo {
public:
	using deleter_func_ty = void(*)(void* );

	Foo(const Foo&) = delete;
	Foo& operator=(const Foo&) = delete;
							
	template<typename T, typename...Args>
	inline static Foo create(Args...args) {
		void* ptr = new T(std::forward<Args>(args)...);

		deleter_func_ty deleter_func = [](void* data_ptr) {
			delete reinterpret_cast<T*>(data_ptr); 
		};

		return Foo(ptr, deleter_func);
	}

	inline Foo(Foo&& other) noexcept : data(other.data), deleter(other.deleter){
		data = other.data;
		deleter = other.deleter;
		other.data = nullptr;
	}


	inline Foo& operator=(Foo&& other) noexcept{
		if (this != &other) {
			try_destroy();
			data = other.data;
			deleter = other.deleter;
			other.try_destroy();
			other.data = nullptr;
		}
		return *this;
	}

	inline ~Foo() noexcept{
		try_destroy();
	}

public:
	void* get() {
		return data;
	}

	deleter_func_ty get_deleter() {
		return deleter;
	}

	inline void try_destroy() noexcept {
		if (data) {
			deleter(data);
			data = nullptr;
		}
	}

private:
	inline Foo(void* data_ptr, deleter_func_ty deleter_func) : data(data_ptr) , deleter(deleter_func){}

private:
	void* data;
	deleter_func_ty deleter;
};

int main() {												   
	using namespace ctx;

}