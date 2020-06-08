// Licensed under the Unlicense <https://unlicense.org/>
// SPDX-License-Identifier: Unlicense
#include <doctest/doctest.h>
#include <small_vector/small_vector.h>

#include <iostream>
#include <vector>

class X {
  int i_;
  int* p_;

public:
  struct special {
    unsigned c;
    unsigned dt;
    unsigned cc;
    unsigned ca;
    unsigned mc;
    unsigned ma;
    constexpr decltype(auto) members() const { return std::tie(c, dt, cc, ca, mc, ma); }
    friend constexpr bool operator==(const special& lhs, const special& rhs) {
      return lhs.members() == rhs.members();
    }
    friend constexpr bool operator!=(const special& lhs, const special& rhs) {
      return lhs.members() != rhs.members();
    }
    friend constexpr special operator+(const special& lhs, const special& rhs) {
        return special{ lhs.c + rhs.c, lhs.dt + rhs.dt, lhs.cc + rhs.cc, lhs.ca + rhs.ca, lhs.mc + rhs.mc, lhs.ma + rhs.ma };
    }
  };
  static inline special sp{};

  X(int i, int* p = nullptr) : i_(i), p_(p) {
    //         std::cout << "X(int i, int* p)\n";
    sp.c++;
  }

  ~X() {
    //         std::cout << "~X()\n";
    sp.dt++;
  }
  X() noexcept : i_(0), p_(nullptr) {
      //         std::cout << "X(const X& x)\n";
      sp.c++;
  }

  X(const X& x) : i_(x.i_), p_(x.p_) {
    //         std::cout << "X(const X& x)\n";
    sp.cc++;
  }

  X& operator=(const X& x) {
    i_ = x.i_;
    p_ = x.p_;
    //         std::cout << "X& operator=(const X& x)\n";
    sp.ca++;
    return *this;
  }

  X(X&& x) noexcept : i_(x.i_), p_(x.p_) {
    //         std::cout << "X(X&& x)\n";
    sp.mc++;
  }

  X& operator=(X&& x) noexcept {
    i_ = x.i_;
    p_ = x.p_;
    //         std::cout << "X& operator=(X&& x)\n";
    sp.ma++;
    return *this;
  }
};

constexpr X::special copyConstruct = {0, 0, 1, 0, 0, 0};
constexpr X::special moveConstruct = { 0, 0, 0, 0, 1, 0 };
constexpr X::special temporary = {1, 1, 0, 0, 0, 0};


std::ostream& operator<<(std::ostream& os, X::special const& sp) {
  os << sp.c <<  ',';
  os << sp.dt << ',';
  os << sp.cc << ',';
  os << sp.ca << ',';
  os << sp.mc << ',';
  os << sp.ma;
  return os;
}

TEST_CASE("default_construct_has_reserved_small_buffer") {
    X::sp = {};
    auto myVec = sbo::small_vector<int, 100>();
    CHECK(myVec.capacity() == 100);
    CHECK(myVec.size() == 0);
    CHECK(X::sp == X::special{});
}


TEST_CASE("test no reallocation") {
  X::sp = {};
  sbo::small_vector<X, 4> v{X(0, nullptr), X(0, nullptr), X(0, nullptr)};
  CHECK(X::sp == X::special{ 3, 3, 3, 0, 0, 0 });
  X x{0, nullptr};
  X::sp = {};
  SUBCASE("insert lvalue no reallocation") {
    v.insert(v.begin(), x);
    CHECK(X::sp == X::special{0, 1, 1, 0, 1, 3});
  }
  SUBCASE("emplace lvalue no reallocation") {
    v.emplace(v.begin(), x);
    CHECK(X::sp == X::special{0, 1, 1, 0, 1, 3});
  }
  SUBCASE("insert xvalue no reallocation") {
    v.insert(v.begin(), std::move(x));
    CHECK(X::sp == X::special{ 0, 1, 0, 0, 2, 3 });
  }
  SUBCASE("emplace xvalue no reallocation") {
    v.emplace(v.begin(), std::move(x));
    CHECK(X::sp == X::special{ 0, 1, 0, 0, 2, 3 });
  }
  SUBCASE("insert rvalue no reallocation") {
    v.insert(v.begin(), X{0, nullptr});
    CHECK(X::sp == X::special{1, 2, 0, 0, 2, 3});
  }
  SUBCASE("emplace rvalue no reallocation") {
    v.emplace(v.begin(), X{0, nullptr});
    CHECK(X::sp == X::special{1, 2, 0, 0, 2, 3});
  }
  SUBCASE("emplace rvalue no reallocation") {
    v.push_back(x);
    CHECK(X::sp == X::special{ 0, 0, 1, 0, 0, 0 });
  }
  SUBCASE("emplace_back lvalue no reallocation") {
    v.emplace_back(x);
    CHECK(X::sp == copyConstruct);
  }
  SUBCASE("push_back xvalue no reallocation") {
    v.push_back(std::move(x));
    CHECK(X::sp == moveConstruct);
  }
  SUBCASE("emplace_back xvalue no reallocation") {
    v.emplace_back(std::move(x));
    CHECK(X::sp == moveConstruct);
  }
  SUBCASE("push_back rvalue no reallocation") {
    v.push_back(X{0, nullptr});
    CHECK(X::sp == temporary + moveConstruct);
  }
  SUBCASE("emplace_back rvalue no reallocation") {
    v.emplace_back(X{0, nullptr});
    CHECK(X::sp == temporary + moveConstruct);
  }
  CHECK(v.capacity() == 4);
  CHECK(v.size() == 4);
}

TEST_CASE("Small vector with reallocation") {
  X::sp = {};
  sbo::small_vector<X, 3> v;
  v.emplace_back(0, nullptr);
  v.emplace_back(0, nullptr);
  v.emplace_back(0, nullptr);
  constexpr auto construct = X::special{ 1 };
  constexpr auto moveAndDestruct3 = X::special{ 0, 3, 0, 0, 3, 0 };
  
  CHECK(X::sp == construct + construct + construct);
  X x{0, nullptr};
  X::sp = {};
  SUBCASE("resize") {
    v.resize(4);
    CHECK(X::sp == moveAndDestruct3 + construct);
  }
  SUBCASE("emplace lvalue reallocation") {
    v.emplace(v.begin(), x);
    CHECK(X::sp == moveAndDestruct3 + copyConstruct);
  }
  SUBCASE("emplace xvalue reallocation") {
    v.emplace(v.begin(), std::move(x));
    CHECK(X::sp == moveAndDestruct3 + moveConstruct);
  }
  SUBCASE("emplace rvalue reallocation") {
    v.emplace(v.begin(), X{0, nullptr});
    CHECK(X::sp == moveAndDestruct3 + moveConstruct + temporary);
  }
  SUBCASE("insert xvalue reallocation") {
    v.insert(v.begin(), std::move(x));
    CHECK(X::sp == moveAndDestruct3 + moveConstruct);
  }
  SUBCASE("push_back lvalue reallocation") {
    v.push_back(x);
    CHECK(X::sp == moveAndDestruct3 + copyConstruct);
  }
  SUBCASE("push_back xvalue reallocation") {
    v.push_back(std::move(x));
    CHECK(X::sp == moveAndDestruct3 + moveConstruct);
  }
  SUBCASE("push_back rvalue reallocation") {
    v.push_back(X{0, nullptr});
    CHECK(X::sp == moveAndDestruct3 + moveConstruct + temporary);
  }
  SUBCASE("emplace_back construct in place") {
      v.emplace_back(0, nullptr);
      CHECK(X::sp == moveAndDestruct3 + construct);
  }
  SUBCASE("emplace_back lvalue reallocation") {
    v.emplace_back(x);
    CHECK(X::sp == moveAndDestruct3 + copyConstruct);
  }
  SUBCASE("emplace_back xvalue reallocation") {
    v.emplace_back(std::move(x));
    CHECK(X::sp == moveAndDestruct3 + moveConstruct);
  }
  SUBCASE("emplace_back rvalue reallocation") {
    v.emplace_back(X{0, nullptr});
    CHECK(X::sp == moveAndDestruct3 + moveConstruct + temporary);
  }
  SUBCASE("insert lvalue reallocation") {
    v.insert(v.begin(), x);
    CHECK(X::sp == moveAndDestruct3  + copyConstruct);
  }
  SUBCASE("insert rvalue reallocation") {
    v.insert(v.begin(), X{0, nullptr});
    CHECK(X::sp == moveAndDestruct3 + moveConstruct + temporary);
  }
}
