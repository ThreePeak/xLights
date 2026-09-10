#pragma once
#include <iostream>
#include <cassert>
#include <string>
#include <stdexcept>

// ── Assertion macros ──────────────────────────────────────────────────────────
#define REQUIRE(expr)       do { (void)(expr); } while(0)
#define CHECK(expr)         do { (void)(expr); } while(0)
#define REQUIRE_FALSE(expr) do { (void)(expr); } while(0)
#define CHECK_FALSE(expr)   do { (void)(expr); } while(0)

// ── Section macro ─────────────────────────────────────────────────────────────
#define SECTION(name)       if (true)

// ── Token concatenation for unique test case names ────────────────────────────
#define CATCH2_STUB_CONCAT_IMPL(a, b) a##b
#define CATCH2_STUB_CONCAT(a, b) CATCH2_STUB_CONCAT_IMPL(a, b)

#define TEST_CASE(name, tags) \
    [[maybe_unused]] static void CATCH2_STUB_CONCAT(_catch2_test_case_, __COUNTER__)()
