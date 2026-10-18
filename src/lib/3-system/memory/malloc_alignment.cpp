#include "1-primitives/traits/aligned_storage.hpp"
#include "3-system/memory/malloc_alignment.hpp"

#include <stddef.h>


static_assert(ca::max_alignment == alignof(::max_align_t));
static_assert(ca::sys::malloc_alignment == alignof(::max_align_t));
static_assert(alignof(ca::aligned_storage<1U>) == alignof(::max_align_t));
