#ifndef UNITY_H
#define UNITY_H

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <string>

inline int g_test_count = 0;
inline int g_fail_count = 0;

#define UNITY_BEGIN() (g_test_count = 0, g_fail_count = 0)
#define UNITY_END() (std::cout << "\n-----------------------\n" << g_test_count << " Tests " << g_fail_count << " Failures 0 Ignored\n" << (g_fail_count == 0 ? "OK\n" : "FAIL\n"), g_fail_count)

#define RUN_TEST(func) do { \
    g_test_count++; \
    std::cout << __FILE__ << ":" << __LINE__ << ":" << #func << ":"; \
    func(); \
    std::cout << "PASS\n"; \
} while(0)

#define TEST_ASSERT_TRUE(condition) if (!(condition)) { std::cout << "FAIL (line " << __LINE__ << ")\n"; g_fail_count++; return; }
#define TEST_ASSERT_FALSE(condition) if (condition) { std::cout << "FAIL (line " << __LINE__ << ")\n"; g_fail_count++; return; }
#define TEST_ASSERT_EQUAL_INT(expected, actual) if ((expected) != (actual)) { std::cout << "FAIL (expected " << (expected) << " got " << (actual) << " at line " << __LINE__ << ")\n"; g_fail_count++; return; }
#define TEST_ASSERT_EQUAL_UINT8(expected, actual) if ((uint8_t)(expected) != (uint8_t)(actual)) { std::cout << "FAIL (expected " << (int)(expected) << " got " << (int)(actual) << " at line " << __LINE__ << ")\n"; g_fail_count++; return; }
#define TEST_ASSERT_EQUAL_UINT32(expected, actual) if ((uint32_t)(expected) != (uint32_t)(actual)) { std::cout << "FAIL (expected " << (uint32_t)(expected) << " got " << (uint32_t)(actual) << " at line " << __LINE__ << ")\n"; g_fail_count++; return; }
#define TEST_ASSERT_EQUAL_STRING(expected, actual) if (std::string(expected) != std::string(actual)) { std::cout << "FAIL (expected " << (expected) << " got " << (actual) << " at line " << __LINE__ << ")\n"; g_fail_count++; return; }
#define TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual) if (std::abs((expected) - (actual)) > (delta)) { std::cout << "FAIL at line " << __LINE__ << "\n"; g_fail_count++; return; }

#endif // UNITY_H
