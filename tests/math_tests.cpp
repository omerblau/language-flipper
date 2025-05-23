//
// Created by omerb on 22/05/2025.
//
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

int add(int a, int b) { return a + b; }

TEST_CASE("simple add") {
    CHECK(add(2, 3) == 5);
}
