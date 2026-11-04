#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/atomic/memory_order.hpp"
#include "1-primitives/debug/trap.hpp"
#include "1-primitives/memory/copy_bytes.hpp"
#include "1-primitives/memory/fill_bytes.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/thread/thread_key.hpp"
#include "3-system/thread/thread_key_create.hpp"
#include "3-system/thread/thread_key_set.hpp"
#include "4-lifecycle/memory/allocation.hpp"
#include "4-lifecycle/memory/heap.hpp"
#include "6-resources/dl/impl/tls.hpp"


namespace {

	/* A registered TLS image. _generation changes at each registration and release, so a block
	   a thread made for an earlier user of the id is never taken for the current one; it is
	   published last (release), the fields before it. */
	struct module_entry final {

			using self = module_entry;

			ca::atomic<ca::u64> _generation;
			const ca::byte* _image;
			ca::usz _file_size;
			ca::usz _size;
			ca::usz _align;
			bool _used;

	}; // struct module_entry

	/* A thread's block for one module, and the generation it was made for. */
	struct slot final {

			using self = slot;

			void* _block;
			ca::usz _size;
			ca::usz _align;
			ca::u64 _generation;

	}; // struct slot

	/* A thread's blocks, indexed by module id: its dynamic thread vector. */
	struct thread_vector final {

			using self = thread_vector;

			slot* _slots;
			ca::usz _capacity;
			bool _keyed;

	}; // struct thread_vector

	constinit module_entry modules[ca::dl::impl::tls_modules] = {};
	constinit ca::u64 next_generation = 1U;
	constinit ca::sys::thread_key exit_key{0U};
	constinit bool keyed = false;
	constinit thread_local thread_vector current{nullptr, 0U, false};

	auto release_block(slot& s) noexcept -> void {

		if (s._block != nullptr)
			ca::heap{}.deallocate(ca::allocation{s._block, s._size}, s._align);
		s = slot{nullptr, 0U, 0U, 0U};
	}

	auto release_vector(void* const value) noexcept -> void {

		thread_vector& v = *static_cast<thread_vector*>(value);
		for (ca::usz i = 0U; i < v._capacity; ++i)
			release_block(v._slots[i]);
		if (v._slots != nullptr)
			ca::heap{}.deallocate(ca::allocation{v._slots, v._capacity * sizeof(slot)}, alignof(slot));
		v = thread_vector{nullptr, 0U, false};
	}

	[[noreturn, gnu::cold, gnu::noinline]] auto tls_out_of_memory(void) noexcept -> void {
		ca::trap("__tls_get_addr: out of memory");
	}

	[[noreturn, gnu::cold, gnu::noinline]] auto tls_bad_module(void) noexcept -> void {
		ca::trap("__tls_get_addr: no such module");
	}

	[[gnu::noinline]] auto grow(const ca::usz needed) noexcept -> void {

		ca::usz capacity = current._capacity == 0U ? 16U : current._capacity;
		while (capacity <= needed)
			capacity *= 2U;
		auto made = ca::heap{}.allocate(capacity * sizeof(slot), alignof(slot));
		if (!made)
			tls_out_of_memory();
		slot* const slots = static_cast<slot*>(made->data());
		for (ca::usz i = 0U; i < capacity; ++i)
			slots[i] = i < current._capacity ? current._slots[i] : slot{nullptr, 0U, 0U, 0U};
		if (current._slots != nullptr)
			ca::heap{}.deallocate(ca::allocation{current._slots, current._capacity * sizeof(slot)}, alignof(slot));
		   current._slots = slots;
		current._capacity = capacity;
	}

	[[gnu::noinline]] auto refill(slot& s, const module_entry& m, const ca::u64 generation) noexcept -> void {

		release_block(s);
		const ca::usz size = m._size != 0U ? m._size : 1U;
		auto made = ca::heap{}.allocate(size, m._align);
		if (!made)
			tls_out_of_memory();
		ca::copy_bytes(made->data(), m._image, m._file_size);
		ca::fill_bytes(static_cast<ca::byte*>(made->data()) + m._file_size, 0U, size - m._file_size);
		s = slot{made->data(), made->size(), m._align, generation};
		if (!current._keyed && keyed) {
			static_cast<void>(ca::sys::thread_key_set(exit_key, &current));
			current._keyed = true;
		}
	}

} // namespace


[[nodiscard]] auto ca::dl::impl::tls_register(const ca::byte* const image, const ca::usz file_size, const ca::usz size, const ca::usz align) noexcept -> ca::result<ca::u64> {

	if (!keyed) {
		auto key = ca::sys::thread_key_create(release_vector);
		if (!key)
			return key.propagate();
		exit_key = *key;
		   keyed = true;
	}
	for (ca::u64 id = 1U; id < ca::dl::impl::tls_modules; ++id) {
		module_entry& m = modules[id];
		if (m._used)
			continue;
		    m._image = image;
		m._file_size = file_size;
		     m._size = size;
		    m._align = align != 0U ? align : 1U;
		     m._used = true;
		m._generation.store<ca::memory_order::release>(next_generation++);
		return id;
	}
	return ca::failure{ca::errc::no_space};
}

auto ca::dl::impl::tls_unregister(const ca::u64 module) noexcept -> void {

	if (module == 0U || module >= ca::dl::impl::tls_modules)
		return;
	modules[module]._used = false;
	modules[module]._generation.store<ca::memory_order::release>(next_generation++);
	if (module < current._capacity)
		release_block(current._slots[module]);
}

auto ca::dl::impl::tls_get_addr(const ca::dl::impl::tls_index* const index) noexcept -> void* {

	const ca::u64 id = index->_module;
	if (id == 0U || id >= ca::dl::impl::tls_modules) [[unlikely]]
		tls_bad_module();
	const module_entry& m = modules[id];
	const ca::u64 generation = m._generation.load<ca::memory_order::acquire>();
	if (id >= current._capacity) [[unlikely]]
		grow(id);
	slot& s = current._slots[id];
	if (s._block == nullptr || s._generation != generation) [[unlikely]]
		refill(s, m, generation);
	return static_cast<ca::byte*>(s._block) + index->_offset;
}
