#pragma once

#include <exception>
#include <iostream>
#include <vector>

namespace testfw {

struct TestCase {
    const char* name;
    void (*function)();
};

inline std::vector<TestCase>& allTests() {
    static std::vector<TestCase> tests;
    return tests;
}

inline int& failedChecks() {
    static int count = 0;
    return count;
}

struct Registrar {
    Registrar(const char* name, void (*function)()) {
        allTests().push_back({name, function});
    }
};

inline int runAllTests() {
    int failedTests = 0;
    for (const TestCase& test : allTests()) {
        const int before = failedChecks();
        try {
            test.function();
        } catch (const std::exception& e) {
            std::cout << "  unexpected exception: " << e.what() << "\n";
            ++failedChecks();
        } catch (...) {
            std::cout << "  unexpected unknown exception\n";
            ++failedChecks();
        }
        const bool passed = (failedChecks() == before);
        std::cout << (passed ? "[PASS] " : "[FAIL] ") << test.name << "\n";
        if (!passed) {
            ++failedTests;
        }
    }
    const int total = static_cast<int>(allTests().size());
    std::cout << "\n" << (total - failedTests) << "/" << total << " tests passed\n";
    return failedTests == 0 ? 0 : 1;
}

}

#define TEST_CASE(name)                                                   \
    static void name();                                                   \
    static const testfw::Registrar name##_registrar(#name, &name);        \
    static void name()

#define CHECK(condition)                                                  \
    do {                                                                  \
        if (!(condition)) {                                               \
            std::cout << "  CHECK failed: " #condition " (" << __FILE__   \
                      << ":" << __LINE__ << ")\n";                        \
            ++testfw::failedChecks();                                     \
        }                                                                 \
    } while (false)

#define CHECK_EQ(actual, expected)                                        \
    do {                                                                  \
        const auto& actual_ = (actual);                                   \
        const auto& expected_ = (expected);                               \
        if (!(actual_ == expected_)) {                                    \
            std::cout << "  CHECK_EQ failed: " #actual " == " #expected   \
                      << " (" << __FILE__ << ":" << __LINE__ << ")\n"     \
                      << "    got:      " << actual_ << "\n"              \
                      << "    expected: " << expected_ << "\n";           \
            ++testfw::failedChecks();                                     \
        }                                                                 \
    } while (false)
