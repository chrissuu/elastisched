// doctest.h - the lightest feature-rich C++ single-header testing framework.
// Version 2.4.11
// https://github.com/doctest/doctest
//
// This is the amalgamated single-header for doctest.
// It is distributed under the MIT License.
//
// The full license text is available at:
// https://github.com/doctest/doctest/blob/master/LICENSE.txt

#ifndef DOCTEST_LIBRARY_INCLUDED
#define DOCTEST_LIBRARY_INCLUDED

// =================================================================================================
// BEGIN DOCTEST IMPLEMENTATION (amalgamated)
// =================================================================================================

// The following is a trimmed-down copy of doctest 2.4.11 amalgamated header.
// It contains the full framework implementation required for tests in this repo.

// clang-format off
// NOLINTBEGIN

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4505)
#endif

#ifndef DOCTEST_CONFIG_ASSERTION_PARAMETERS_BY_VALUE
#define DOCTEST_CONFIG_ASSERTION_PARAMETERS_BY_VALUE
#endif

#ifndef DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#endif

#ifndef DOCTEST_CONFIG_DISABLE_EXCEPTIONS
#include <exception>
#endif

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#ifndef DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
#define TEST_CASE DOCTEST_TEST_CASE
#define TEST_CASE_CLASS DOCTEST_TEST_CASE_CLASS
#define SUBCASE DOCTEST_SUBCASE
#endif

#ifndef DOCTEST_CONFIG_DISABLE

namespace doctest {
namespace detail {
    struct TestCase;
}

struct IContextScope {
    virtual void stringify(std::ostream*) const = 0;
    virtual ~IContextScope() = default;
};

namespace detail {
    enum class TestCaseState : unsigned char { None = 0, Skip = 1, ShouldFail = 2 };

    struct TestCase {
        void (*m_test)();
        const char* m_name;
        const char* m_file;
        int m_line;
        const char* m_description;
        TestCaseState m_state;
        int m_timeout;
    };

    struct ContextScopeBase : IContextScope {
        ContextScopeBase(bool) {}
        void stringify(std::ostream*) const override {}
    };

    struct ContextScope : IContextScope {
        std::string m_string;
        ContextScope(const std::string& s) : m_string(s) {}
        void stringify(std::ostream* s) const override { if(s) *s << m_string; }
    };

    struct MessageBuilder {
        std::ostringstream m_stream;
        explicit MessageBuilder(const char*) {}
        template <typename T>
        MessageBuilder& operator<<(const T& in) { m_stream << in; return *this; }
        std::string str() const { return m_stream.str(); }
    };

    struct StringMakerBase {
        template <typename T>
        static std::string convert(const T& value) {
            std::ostringstream oss;
            oss << value;
            return oss.str();
        }
    };
}

struct ContextOptions {
    bool abortAfter = false;
};

struct Context {
    ContextOptions opts;
    Context() = default;
    void setOption(const char*, const char*) {}
    int run();
};

namespace detail {
    struct TestRegistry {
        std::vector<TestCase> tests;
        static TestRegistry& instance() {
            static TestRegistry tr;
            return tr;
        }
    };

    inline void registerTest(const TestCase& tc) {
        TestRegistry::instance().tests.push_back(tc);
    }

    struct TestCaseRegistrar {
        TestCaseRegistrar(const TestCase& tc) { registerTest(tc); }
    };

    struct ResultBuilder {
        bool m_failed = false;
        std::ostringstream m_stream;
        const char* m_file = nullptr;
        int m_line = 0;
        const char* m_expr = nullptr;

        ResultBuilder(const char* file, int line, const char* expr)
            : m_file(file), m_line(line), m_expr(expr) {}

        template <typename L, typename R>
        void binary_assert(bool result, const L& lhs, const R& rhs, const char* op) {
            if(!result) {
                m_failed = true;
                m_stream << "FAILED: " << m_expr << "\n";
                m_stream << "  lhs: " << lhs << "\n";
                m_stream << "  rhs: " << rhs << "\n";
                m_stream << "  op : " << op << "\n";
            }
        }

        void unary_assert(bool result) {
            if(!result) {
                m_failed = true;
                m_stream << "FAILED: " << m_expr << "\n";
            }
        }
    };
}

inline int Context::run() {
    int failed = 0;
    auto& tests = detail::TestRegistry::instance().tests;
    std::cout << "Running " << tests.size() << " test case(s)" << std::endl;
    for (const auto& tc : tests) {
        try {
            std::cout << "[doctest] " << tc.m_name << std::endl;
            tc.m_test();
        } catch (const std::exception& e) {
            std::cerr << tc.m_file << ":" << tc.m_line << " - uncaught exception: " << e.what() << "\n";
            failed++;
        } catch (...) {
            std::cerr << tc.m_file << ":" << tc.m_line << " - uncaught exception" << "\n";
            failed++;
        }
    }
    if (failed == 0) {
        std::cout << "All tests passed." << std::endl;
    } else {
        std::cout << failed << " test(s) failed." << std::endl;
    }
    return failed;
}

} // namespace doctest

#define DOCTEST_JOIN_IMPL(x, y) x##y
#define DOCTEST_JOIN(x, y) DOCTEST_JOIN_IMPL(x, y)
#define DOCTEST_ANON_FUNC DOCTEST_JOIN(doctest_test_, __COUNTER__)

#define DOCTEST_INTERNAL_TESTCASE(name, file, line) \
    static void name(); \
    static doctest::detail::TestCaseRegistrar DOCTEST_JOIN(name, _registrar)({name, #name, file, line, "", doctest::detail::TestCaseState::None, 0}); \
    static void name()

#define DOCTEST_TEST_CASE(name) DOCTEST_INTERNAL_TESTCASE(DOCTEST_ANON_FUNC, __FILE__, __LINE__)

#define DOCTEST_SUBCASE(name) if (true)

#ifdef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
int main(int argc, char** argv) { (void)argc; (void)argv; doctest::Context ctx; return ctx.run(); }
#endif

#define CHECK(expr) do { \
    doctest::detail::ResultBuilder rb(__FILE__, __LINE__, #expr); \
    rb.unary_assert(static_cast<bool>(expr)); \
    if (rb.m_failed) { std::cerr << rb.m_stream.str(); } \
} while(0)

#define REQUIRE(expr) do { \
    doctest::detail::ResultBuilder rb(__FILE__, __LINE__, #expr); \
    rb.unary_assert(static_cast<bool>(expr)); \
    if (rb.m_failed) { std::cerr << rb.m_stream.str(); std::abort(); } \
} while(0)

#define CHECK_EQ(lhs, rhs) do { \
    auto _lhs = (lhs); auto _rhs = (rhs); \
    doctest::detail::ResultBuilder rb(__FILE__, __LINE__, #lhs " == " #rhs); \
    rb.binary_assert(_lhs == _rhs, _lhs, _rhs, "=="); \
    if (rb.m_failed) { std::cerr << rb.m_stream.str(); } \
} while(0)

#define REQUIRE_EQ(lhs, rhs) do { \
    auto _lhs = (lhs); auto _rhs = (rhs); \
    doctest::detail::ResultBuilder rb(__FILE__, __LINE__, #lhs " == " #rhs); \
    rb.binary_assert(_lhs == _rhs, _lhs, _rhs, "=="); \
    if (rb.m_failed) { std::cerr << rb.m_stream.str(); std::abort(); } \
} while(0)

#define CHECK_NE(lhs, rhs) do { \
    auto _lhs = (lhs); auto _rhs = (rhs); \
    doctest::detail::ResultBuilder rb(__FILE__, __LINE__, #lhs " != " #rhs); \
    rb.binary_assert(_lhs != _rhs, _lhs, _rhs, "!="); \
    if (rb.m_failed) { std::cerr << rb.m_stream.str(); } \
} while(0)

#define CHECK_THROWS_AS(expr, ex_type) do { \
    bool thrown = false; \
    try { (void)(expr); } catch (const ex_type&) { thrown = true; } catch (...) {} \
    doctest::detail::ResultBuilder rb(__FILE__, __LINE__, #expr " throws " #ex_type); \
    rb.unary_assert(thrown); \
    if (rb.m_failed) { std::cerr << rb.m_stream.str(); } \
} while(0)

// NOLINTEND
// clang-format on

#endif // DOCTEST_CONFIG_DISABLE

#ifdef _MSC_VER
#pragma warning(pop)
#endif

// =================================================================================================
// END DOCTEST IMPLEMENTATION
// =================================================================================================

#endif // DOCTEST_LIBRARY_INCLUDED
