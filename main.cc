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

TEST(BaseTest) {
  char* argv[] = {"./main", "--port", "8080", "-Syu", "--lines", "blank"};
  int argc = 6;
  pd::pdargs args{argc, argv};
  REQUIRE(args.get<int>({"port", 'p'}) == 8080);
  REQUIRE(args.get<bool>({"sus", 'S'}));
  REQUIRE(args.get<bool>({"yuko", 'y'}));
  REQUIRE(args.get<bool>({"update", 'u'}));
  REQUIRE(!args.get<bool>({"increase", 'i'}));
  REQUIRE(args.get<std::string>({"lines", 'l'}) == "blank");
  ASSERT_THROW((args.get<int>({"port", 'p'}).value()),
               std::bad_optional_access);
}

TEST(TestNumbers) {
  char* argv[] = {"./main",   "--port", "8080",      "-f",    "12.25",
                  "--negate", "-402",   "--portion", "-20.24", "-m", "-20.479", "-z-297"};
  int argc = 12;
  pd::pdargs args{argc, argv};
  REQUIRE(args.get<int>({"port", 'p'}) == 8080);
  REQUIRE(args.get<float>({"float", 'f'}) == 12.25);
  REQUIRE(args.get<long>({"negate", 'n'}) == -402);
  REQUIRE(args.get<double>({"portion", 'P'}) == -20.24);
  REQUIRE(args.get<double>({"maxxi", 'm'}) == -20.479);
  REQUIRE(args.get<int>({"zooba", 'z'}) == -297);

}

TEST(TestBool) {
  char* argv[] = {"./main", "-Syu", "-iSr", "--root"};
  int argc = 4;
  pd::pdargs args{argc, argv};
  REQUIRE(args.get<bool>({"yummy", 'y'}));
  REQUIRE(args.get<bool>({"update", 'u'}));
  REQUIRE(args.get<bool>({"install", 'i'}));

  REQUIRE(!args.get<bool>({"update", 'u'}));
  REQUIRE(!args.get<bool>({"install", 'i'}));

  ASSERT_THROW(args.get<bool>({"root", 'r'}), std::runtime_error);
  ASSERT_THROW(args.get<bool>({"Sus", 'S'}), std::invalid_argument);
}

TEST(TestStrings) {
  char* argv[] = {"./main", "--selection", "primary", "--mode", "active",
                  "-f",     "soft",        "--force", "super"};
  int argc = 9;
  pd::pdargs args{argc, argv};
  REQUIRE(args.get<std::string>({"selection", 's'}) == "primary");
  REQUIRE(args.get<std::string>({"mode", 'm'}) == "active");

  ASSERT_THROW(args.get<std::string>({"mode", 'm'}).value(),
               std::bad_optional_access);
  ASSERT_THROW(args.get<std::string>({"selection", 's'}).value(),
               std::bad_optional_access);

  ASSERT_THROW(args.get<std::string>({"force", 'f'}), std::runtime_error);
}

TEST(TestGetOr) {
  char* argv[] = {"./main", "--selection", "primary", "-f",  "soft",
                  "-c",     "55",          "-p",      "0.5", "--install"};
  int argc{10};
  pd::pdargs args{argc, argv};

  REQUIRE(args.get_or<std::string>({"selection", 's'}, "secondary") ==
          "primary");
  REQUIRE(args.get_or<std::string>({"model", 'm'}, "new") == "new");

  REQUIRE(args.get_or<int>({"count", 'c'}, 100) == 55);
  REQUIRE(args.get_or<int>({"number", 'n'}, 128) == 128);

  REQUIRE(args.get_or<double>({"portion", 'p'}, 0.25) == 0.5);
  REQUIRE(args.get_or<double>({"fraction", 'F'}, 0.75) == 0.75);

  REQUIRE(args.get<bool>({"install", 'i'}));
  REQUIRE(!args.get<bool>({"update", 'u'}));
}

TEST(TestEqualSign) {
  char* argv[] = {"./main", "--selection=primary", "-f=soft", "-c=55",
                  "--portion=0.5"};
  int argc = 5;
  pd::pdargs args{argc, argv};

  REQUIRE(args.get<std::string>({"selection", 's'}) == "primary");
  REQUIRE(args.get<std::string>({"force", 'f'}) == "soft");
  REQUIRE(args.get<int>({"count", 'c'}) == 55);
  REQUIRE(args.get<double>({"portion", 'p'}) == 0.5);
}

TEST(TestShortOptions) {
  char* argv[] = {"./main", "-sprimary", "-Wall", "-c155", "-f12.532", "-Xabc"};
  int argc = 6;
  pd::pdargs args{argc, argv};

  REQUIRE(args.get<std::string>({"selection", 's'}) == "primary");
  REQUIRE(args.get<std::string>({"warnings", 'W'}) == "all");
  REQUIRE(args.get<unsigned long>({"count", 'c'}) == 155);
  REQUIRE(args.get<double>({"fraction", 'f'}) == 12.532);
  REQUIRE(args.get<bool>({"XXXXX", 'X'}));
  REQUIRE(args.get<bool>({"aaaaaa", 'a'}));
  REQUIRE(args.get<bool>({"cccccc", 'c'}));
  REQUIRE(args.get<bool>({"bbbbbb", 'b'}));
  REQUIRE(!args.get<bool>({"XXXXX", 'X'}));
  REQUIRE(!args.get<bool>({"aaaaaa", 'a'}));
  REQUIRE(!args.get<bool>({"cccccc", 'c'}));
  REQUIRE(!args.get<bool>({"bbbbbb", 'b'}));
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
