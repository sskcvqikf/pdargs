#include <iostream>

#include "pd/pdargs.h"

void* print_testname(const char* name) {
  std::cout << " ----- RUNNING " << name << " ----- \n";
  return nullptr;
}

bool is_failed = false;

#define TEST(name) void name(void* = print_testname(#name))

#define ASSERT(expr, message)                                           \
  if (!(expr)) {                                                        \
    is_failed = true;                                                   \
    std::cerr << "RUNNING: " << #expr << " - FAILED.\n"                 \
              << __FILE__ << ":" << __LINE__ << " " << message << '\n'; \
  }

#define REQUIRE(expr)                                                    \
  if (!(expr)) {                                                         \
    is_failed = true;                                                    \
    std::cerr << "RUNNING: " << #expr << " - FAILED." << __FILE__ << ":" \
              << __LINE__ << '\n';                                       \
  }

#define ASSERT_THROW(expr, exc)                                              \
  try {                                                                      \
    expr;                                                                    \
    is_failed = true;                                                        \
    std::cerr << "RUNNING: " << #expr << " - FAILED."                        \
              << " No exception. " << __FILE__ << ":" << __LINE__ << '\n';   \
  } catch (exc&) {                                                           \
  } catch (const std::exception& e) {                                        \
    is_failed = true;                                                        \
    std::cerr << "RUNNING: " << #expr << " - FAILED."                        \
              << " Other exception. " << __FILE__ << ":" << __LINE__ << '\n' \
              << e.what() << '\n';                                           \
  } catch (...) {                                                            \
    is_failed = true;                                                        \
    std::cerr << "RUNNING: " << #expr << " - FAILED."                        \
              << " Other thing. " << __FILE__ << ":" << __LINE__ << '\n';    \
  }


int main() {
  BaseTest();
  TestNumbers();
  TestBool();
  TestStrings();
  TestGetOr();
  TestEqualSign();
  TestShortOptions();
  if (is_failed) exit(1);

  return 0;
}
