#pragma once

#include <concepts>
#include <memory>
#include <vector>

namespace nv {
	class Arena {
	private:
		std::unique_ptr<std::byte[]> m_buff;
		void* m_nextObjectBegin;
		size_t m_space;
	public:
		Arena(size_t size);

		template<typename T, typename... Args>
		T* emplace(Args&&... args) noexcept requires(std::is_nothrow_constructible_v<T, Args...>) {
			if (!std::align(alignof(T), sizeof(T), m_nextObjectBegin, m_space)) { //align ptr so we can construct a properly aligned object
				return nullptr;
			}
			T* obj = new (m_nextObjectBegin) T(std::forward<Args...>(args)...);
			m_nextObjectBegin = static_cast<std::byte*>(m_nextObjectBegin) + sizeof(T);
			return obj;
		}

		void* rawAlloc(size_t bytes) noexcept;

		void clear() noexcept;
	};

	template<typename T>
	class ArenaAllocator {
	private:
		std::reference_wrapper<Arena> m_arena;
	public:
		ArenaAllocator(Arena& arena) noexcept
			: m_arena{ arena }
		{
		}

		template<typename U>
		friend class ArenaAllocator;

		template<typename U> //rebinding constructor
		ArenaAllocator(const ArenaAllocator<U>& other) 
			: m_arena{ other.m_arena }
		{
		};

		//allocator boilerplate
		using size_type = size_t;
		using value_type = T;
		using difference_type = std::ptrdiff_t;
		template <typename U>
		struct rebind {
			using other = ArenaAllocator<U>;
		};

		T* allocate(size_t n) noexcept {
			auto ptr = static_cast<T*>(m_arena.get().rawAlloc(n * sizeof(T)));
			assert(ptr != nullptr);
			return ptr;
		}
		void deallocate(T*, size_t) noexcept {} //no-op
	};

	/*class ArenaManager {
	private:
		Arena* m_globalArena = nullptr;

	public:
		ArenaManager(size_t size);
	};*/
}