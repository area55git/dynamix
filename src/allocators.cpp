// DynaMix
// Copyright (c) 2013-2017 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "internal.hpp"

#include <dynamix/allocators.hpp>

#if DYNAMIX_DEBUG
#include <dynamix/object.hpp>
#endif

namespace dynamix
{

static_assert(domain_allocator::mixin_data_size == sizeof(internal::mixin_data_in_object), "Error in domain_allocator::mixin_data_size definition");

// ceil(a/b)*b with integers
// scales a so an exact number of b will fit in it
static size_t ceil_scale(size_t a, size_t b)
{
    size_t result = (a + b - 1) / b; // division rounding up
    result *= b; // scale

    return result;
}

void mixin_allocator::construct_mixin(const basic_mixin_type_info& info, void* ptr)
{
    info.constructor(ptr);
}

bool mixin_allocator::copy_construct_mixin(const basic_mixin_type_info& info, void* ptr, const void* source)
{
    if (!info.copy_constructor) return false;
    info.copy_constructor(ptr, source);
    return true;
}

void mixin_allocator::destroy_mixin(const basic_mixin_type_info& info, void* ptr) noexcept
{
    info.destructor(ptr);
}

void object_allocator::on_set_to_object(object&)
{}

void object_allocator::release(object&) noexcept
{}

object_allocator* object_allocator::on_copy_construct(object& target, const object& source)
{
    DYNAMIX_ASSERT(source.allocator() == this);
    return nullptr;
}

object_allocator* object_allocator::on_move(object& target, object& source) noexcept
{
    DYNAMIX_ASSERT(source.allocator() == this);
    return this;
}

namespace internal
{

char* default_allocator::alloc_mixin_data(size_t count, const object*)
{
#if DYNAMIX_DEBUG
    _has_allocated = true;
#endif
    return new char[sizeof(internal::mixin_data_in_object) * count];
}

void default_allocator::dealloc_mixin_data(char* ptr, size_t, const object*)
{
#if DYNAMIX_DEBUG
    DYNAMIX_ASSERT(_has_allocated); // what? deallocate without ever allocating?
#endif
    delete[] ptr;
}

std::pair<char*, size_t> default_allocator::alloc_mixin(const basic_mixin_type_info& info, const object*)
{
#if DYNAMIX_DEBUG
    _has_allocated = true;
#endif

    size_t mem_size = mem_size_for_mixin(info.size, info.alignment);

    auto buffer = new char[mem_size];
    auto offset = mixin_offset(buffer, info.alignment);

    DYNAMIX_ASSERT(offset + info.size <= mem_size); // we should have room for the mixin

    return std::make_pair(buffer, offset);
}

void default_allocator::dealloc_mixin(char* ptr, size_t, const basic_mixin_type_info&, const object*)
{
#if DYNAMIX_DEBUG
    DYNAMIX_ASSERT(_has_allocated); // what? deallocate without ever allocating?
#endif
    delete[] ptr;
}

} // namespace internal

} // namespace dynamix
