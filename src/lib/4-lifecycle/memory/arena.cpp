#include "1-primitives/debug/trap.hpp"
#include "1-primitives/memory/construct_at.hpp"
#include "1-primitives/numeric/add_overflow.hpp"
#include "1-primitives/numeric/align_up.hpp"
#include "1-primitives/numeric/max.hpp"
#include "2-vocabulary/failure.hpp"
#include "3-system/error/errc.hpp"
#include "4-lifecycle/memory/arena.hpp"
#include "4-lifecycle/memory/page_allocator.hpp"


static constexpr ca::usz chunk_alignment = ca::page_allocator::ALIGNMENT;

static auto unmap(ca::impl::arena_chunk* const chunk) noexcept -> void {
	ca::page_allocator{}.deallocate(ca::allocation{chunk, chunk->_size}, chunk_alignment);
}

[[noreturn, gnu::noinline, gnu::cold]] auto ca::impl::allocator_of_an_empty_arena(void) noexcept -> void {
	ca::trap();
}

auto ca::impl::arena_release(ca::impl::arena_state* const state) noexcept -> void {

	if (state == nullptr)
		return;
	ca::impl::arena_chunk* chunk = state->_chunk;
	while (chunk != nullptr) {
		ca::impl::arena_chunk* const previous = chunk->_previous;
		unmap(chunk);
		chunk = previous;
	}
}

[[nodiscard, gnu::noinline]] auto ca::impl::arena_grow(ca::impl::arena_state* const state, const ca::usz size, const ca::usz alignment) noexcept -> ca::result<ca::allocation> {

	ca::usz bytes{0U};
	if (ca::add_overflow(bytes, size, alignment, sizeof(ca::impl::arena_chunk))) [[unlikely]]
		return ca::failure{ca::errc::out_of_memory};
	auto mapped = ca::page_allocator{}.allocate(ca::max(bytes, state->_chunk_size), chunk_alignment);
	if (!mapped) [[unlikely]]
		return mapped.propagate();
	ca::impl::arena_chunk* const chunk = ca::construct_at(static_cast<ca::impl::arena_chunk*>(mapped->data()), ca::impl::arena_chunk{state->_chunk, mapped->size()});
	ca::usz start = reinterpret_cast<ca::usz>(chunk + 1);
	static_cast<void>(ca::align_up(start, alignment));
	state->_chunk = chunk;
	  state->_top = reinterpret_cast<char*>(start + size);
	  state->_end = reinterpret_cast<char*>(chunk) + mapped->size();
	return ca::allocation{reinterpret_cast<void*>(start), size};
}

[[nodiscard]] auto ca::arena::make(const ca::usz chunk_size) noexcept -> ca::result<self> {

	constexpr ca::usz header = sizeof(ca::impl::arena_chunk) + sizeof(ca::impl::arena_state);
	auto mapped = ca::page_allocator{}.allocate(ca::max(chunk_size, header + 1U), chunk_alignment);
	if (!mapped) [[unlikely]]
		return mapped.propagate();
	char* const base = static_cast<char*>(mapped->data());
	ca::impl::arena_chunk* const chunk = ca::construct_at(reinterpret_cast<ca::impl::arena_chunk*>(base), ca::impl::arena_chunk{nullptr, mapped->size()});
	ca::impl::arena_state* const state = ca::construct_at(reinterpret_cast<ca::impl::arena_state*>(base + sizeof(ca::impl::arena_chunk)),
		ca::impl::arena_state{base + header, base + mapped->size(), chunk, chunk, mapped->size()});
	self built{};
	built._state = state;
	return built;
}

auto ca::arena::reset(void) noexcept -> void {

	if (_state == nullptr)
		return;
	ca::impl::arena_chunk* chunk = _state->_chunk;
	while (chunk != _state->_first) {
		ca::impl::arena_chunk* const previous = chunk->_previous;
		unmap(chunk);
		chunk = previous;
	}
	char* const base = reinterpret_cast<char*>(chunk);
	_state->_chunk = chunk;
	  _state->_top = base + sizeof(ca::impl::arena_chunk) + sizeof(ca::impl::arena_state);
	  _state->_end = base + chunk->_size;
}
