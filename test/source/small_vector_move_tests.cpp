// Licensed under the Unlicense <https://unlicense.org/>
// SPDX-License-Identifier: Unlicense
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <small_vector/small_vector.h>

#include <iostream>
#include <numeric>
#include <vector>
#include <memory>

#include <chrono>
#include <cstring>

struct ThrowMoveT{
    ThrowMoveT(ThrowMoveT&&) noexcept(false) {}
};

static_assert(std::is_nothrow_move_constructible_v<sbo::small_vector<int, 100>>);
static_assert(!std::is_nothrow_move_constructible_v<sbo::small_vector<ThrowMoveT, 100>>);


TEST_CASE("test_for_crash_access_used_moved_from_trivial_copyable_type") {
    auto myVec = std::make_unique<sbo::small_vector<int, 100>>(100);
    for (unsigned i = 0; i < 50; ++i)
        (*myVec)[i] = static_cast<int>(i);

    sbo::small_vector<int, 100> myVector2(std::move(*myVec));

    myVec = nullptr;

    for (unsigned i = 0; i < 50; ++i)
        CHECK(myVector2[i] == i);
}

static_assert(std::is_nothrow_move_constructible_v<sbo::small_vector<std::unique_ptr<int>, 100>>);

TEST_CASE("test_for_crash_access_used_moved_from_move_only_type") {
    auto myVec = std::make_unique<sbo::small_vector<std::unique_ptr<int>, 100>>(50);
    for (unsigned i = 0; i < 50; ++i)
        (*myVec)[i] = std::make_unique<int>(i);

    sbo::small_vector<std::unique_ptr<int>, 100> myVector2(std::move(*myVec));

    myVec = nullptr;

    myVector2 = sbo::small_vector<std::unique_ptr<int>, 100>();
    myVector2.resize(50);
    for (unsigned i = 0; i < 50; ++i)
        CHECK(myVector2[i] == nullptr);
}
TEST_CASE("swap_test_small") {
    sbo::small_vector<int, 10> ints1, ints2;
    ints1.push_back(1);
    ints2.push_back(0);
    std::swap(ints1, ints2);
    CHECK(ints1.front() == 0);
    CHECK(ints2.front() == 1);
}

TEST_CASE("swap_test_big") {
    sbo::small_vector<int, 10> ints1(20), ints2(15);
    ints1[5] = 1;
    ints2[6] = 2;
    std::swap(ints1, ints2);
    CHECK(ints1[6]== 2);
    CHECK(ints2[5] == 1);
}


