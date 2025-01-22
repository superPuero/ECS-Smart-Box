#pragma once
#include <unordered_map>
#include <unordered_set>
#include <typeindex>
#include <iostream>
#include <algorithm>
#include <execution>

#define DEBUG

#ifdef DEBUG
	#define ctx_assert(expr, msg) if (!expr){std::cout << msg << " on line " << __LINE__, __debugbreak();}
#else 
	#define ctx_assert(expr, msg)
#endif DEBUG

namespace ctx {
	class SmartBox {
	public:
		using deleter_func_ty = void(*)(void*);

		SmartBox(const SmartBox&) = delete;
		SmartBox& operator=(const SmartBox&) = delete;

		SmartBox() : data(nullptr), deleter(nullptr){}

		template<typename T, typename...Args>
		inline static SmartBox create(Args&&...args) {
			void* ptr = new T(std::forward<Args>(args)...);

			deleter_func_ty deleter_func = [](void* data_ptr) {
				delete reinterpret_cast<T*>(data_ptr);
			};
			return SmartBox(ptr, deleter_func);
		}

		inline SmartBox(SmartBox&& other) noexcept : data(other.data), deleter(other.deleter) {
			other.data = nullptr;
		}


		inline SmartBox& operator=(SmartBox&& other) noexcept {
			if (this != &other) {
				try_destroy();
				data = other.data;
				deleter = other.deleter;
				other.data = nullptr;
			}
			return *this;
		}

		inline ~SmartBox() noexcept {
			try_destroy();
			deleter = nullptr;
			data = nullptr;
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
		inline SmartBox(void* data_ptr, deleter_func_ty deleter_func) : data(data_ptr), deleter(deleter_func) {}

	private:
		void* data;
		deleter_func_ty deleter;
	};

	using EntityId = uint64_t;
	using ComponentId = std::type_index;
	using ComponentData = SmartBox;

	class Registry {
	public:
		static Registry create()
		{
			return Registry();
		}
		
		~Registry() noexcept {}

		EntityId entity() noexcept
		{
			EntityId new_id = m_counter;
			m_counter++;

			m_pool[new_id];
			m_data[new_id];

			return new_id;
		}

		template<typename T>
		inline T& add(EntityId entity_id, T&& arg)
		{
			ComponentId component_id = typeid(T);

			ctx_assert(valid(entity_id), "invalid entity");

			m_data[entity_id].emplace(component_id, SmartBox::create<T>(std::forward<T>(arg)));
			m_pool[entity_id].insert(component_id);
			m_valids[component_id].insert(entity_id);
			
			return *reinterpret_cast<T*>(m_data[entity_id][component_id].get());
		}

		template<typename T, typename...Args>
		inline T& emplace(EntityId entity_id, Args&&...args)
		{
			ComponentId component_id = typeid(T);

			ctx_assert(valid(entity_id), "invalid entity");

			m_data[entity_id].emplace(component_id, SmartBox::create<T>(std::forward<Args>(args)...));
			m_pool[entity_id].insert(component_id);
			m_valids[component_id].insert(entity_id);

			return *reinterpret_cast<T*>(m_data[entity_id][component_id].get());
		}

		template<typename T>
		inline T& get(EntityId entity_id) noexcept
		{
			ComponentId component_id = typeid(T);

			ctx_assert(valid(entity_id), "invalid entity");
			ctx_assert(valid(entity_id, component_id), "invalid component");

			return *reinterpret_cast<T*>(m_data[entity_id][component_id].get());
		}

		template<typename T>
		inline void remove(EntityId entity_id)
		{
			ComponentId component_id = typeid(T);

			ctx_assert(valid(entity_id), "invalid entity");
			ctx_assert(valid(entity_id, component_id), "invalid component");

			m_valids[component_id].erase(entity_id);
		}

		inline void destroy(EntityId entity_id) 
		{
			ctx_assert(valid(entity_id), "invalid entity");
			m_data.erase(entity_id);
			for (ComponentId component_id : m_pool[entity_id] ) {
				m_valids[component_id].erase(entity_id);
			}
			m_pool.erase(entity_id);
		}

		template<typename...Args>
		inline std::unordered_set<EntityId> group() noexcept
		{
			constexpr size_t components_count = sizeof...(Args);

			ComponentId component_ids[components_count] = { typeid(Args)... };

			std::unordered_set<EntityId> current_group = m_valids[component_ids[0]];
			std::unordered_set<EntityId> compare_group;

			for (ComponentId component_id : component_ids) {
				for (EntityId entity_id : m_valids[component_id]) {
					if (current_group.contains(entity_id)) {
						compare_group.insert(entity_id);
					}
				}
				std::swap(current_group, compare_group);
				if (current_group.empty()) {
					break;
				}
				compare_group.clear();
			}

			return current_group;
		}

		template<typename...Args>
		inline std::vector<std::tuple<Args&...>> query()
		{
			auto commons = group<Args...>();

			std::vector<std::tuple<Args&...>>  query;

			for (EntityId entity_id : commons) {
				query.push_back({ get<Args>(entity_id)... });
			}

			return query;
		}

		template<typename...Args, typename Fn>
		void transform(Fn task)
		{
			static_assert(std::is_convertible_v<Fn, void(*)(std::tuple<Args&...>&)>,
				"Only non-capturing lambdas are allowed");

			std::vector<std::tuple<Args&...>> view = query<Args...>();
			std::for_each(std::execution::par_unseq, view.begin(), view.end(), task);
		}

		template<typename...Args, typename Fn>
		void task(Fn task)
		{
			std::unordered_set<EntityId> view = group<Args...>();
			std::for_each(std::execution::unseq, view.begin(), view.end(), task);
		}

		inline uint64_t count() {
			return m_pool.size();
		}

	private:	 

		Registry() : m_counter(1){}

		inline bool valid(EntityId entity_id) 
		{
			return m_pool.contains(entity_id);
		}
		
		inline bool valid(EntityId entity_id, ComponentId component_id) 
		{
			return m_pool.contains(entity_id) && m_valids[component_id].contains(entity_id);
		}
		
	private:
		std::unordered_map<EntityId, std::unordered_map<ComponentId, ComponentData>> m_data;
		std::unordered_map<EntityId, std::unordered_set<ComponentId>> m_pool;
		std::unordered_map<ComponentId, std::unordered_set<EntityId>> m_valids;
		

	private:
		EntityId m_counter;
	};
}
