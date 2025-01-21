#pragma once
#include <unordered_map>
#include <unordered_set>
#include <typeindex>
#include <iostream>
#include <new>
#define DEBUG

#ifdef DEBUG
	#define ctx_assert(expr, msg) if (!expr){std::cout << msg, __debugbreak();}
#elif // DEBUG
	#define ctx_assert(expr, msg)
#endif DEBUG

namespace ctx {

	using EntityId = uint64_t;
	using ComponentId = std::type_index;
	using ComponentData = void*;

	class Registry {
	public:
		static Registry create()
		{
			return Registry();
		}
		
		EntityId entity() 
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

			m_data[entity_id][component_id] = new T(std::forward<T>(arg));
			m_valids[component_id].insert(entity_id);
			
			return *reinterpret_cast<T*>(m_data[entity_id][component_id]);
		}

		template<typename T, typename...Args>
		inline T& emplace(EntityId entity_id, Args&&...args)
		{
			ComponentId component_id = typeid(T);

			ctx_assert(valid(entity_id), "invalid entity");

			m_data[entity_id][component_id] = new T(std::forward<Args>(args)...);
			m_valids[component_id].insert(entity_id);

			return *reinterpret_cast<T*>(m_data[entity_id][component_id]);
		}

		template<typename T>
		inline T& get(EntityId entity_id) 
		{
			ComponentId component_id = typeid(T);

			ctx_assert(valid(entity_id), "invalid entity");
			ctx_assert(valid(entity_id, component_id), "invalid component");

			return *reinterpret_cast<T*>(m_data[entity_id][component_id]);
		}

	private:
		Registry()
		{
			m_counter = 1;			
		}

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
