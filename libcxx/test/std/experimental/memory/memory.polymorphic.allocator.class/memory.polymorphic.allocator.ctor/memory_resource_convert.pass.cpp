//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, libcpp-no-rtti

// test_memory_resource requires RTTI for dynamic_cast
// UNSUPPORTED: no-rtti

// <experimental/memory_resource>

// template <class T> class polymorphic_allocator

// polymorphic_allocator<T>::polymorphic_allocator(memory_resource *)

// ADDITIONAL_COMPILE_FLAGS: -D_LIBCPP_DISABLE_DEPRECATION_WARNINGS

#include <experimental/memory_resource>
#include <type_traits>
#include <cassert>

#include "test_memory_resource.h"

#include "test_macros.h"

namespace ex = std::experimental::pmr;

int main(int, char**)
{
    {
        typedef ex::polymorphic_allocator<void> A;
        static_assert(
            std::is_convertible<decltype(nullptr), A>::value
          , "Must be convertible"
          );
        static_assert(
            std::is_convertible<ex::memory_resource *, A>::value
          , "Must be convertible"
          );
    }
    {
        typedef ex::polymorphic_allocator<void> A;
        TestResource R;
        A const a(&R);
        assert(a.resource() == &R);
    }

  return 0;
}
