#include <gtest/gtest.h>

#include <pd/pdargs.h>

TEST(PdargsTest, JustWorks) {
  char* argv[] = {"./main", "--port", "8080", "-Syu", "--lines", "blank"};
  int argc = 6;
  pd::pdargs args(argc, argv);
  
  EXPECT_EQ(args.get<int>({"port", 'p'}), 8080);
  EXPECT_TRUE(args.get<bool>({"sus", 'S'}));
  EXPECT_TRUE(args.get<bool>({"yuko", 'y'}));
  EXPECT_TRUE(args.get<bool>({"update", 'u'}));
  EXPECT_FALSE(args.get<bool>({"increase", 'i'}));
  EXPECT_EQ(args.get<std::string>({"lines", 'l'}), "blank");
  EXPECT_THROW(args.get<int>({"port", 'p'}).value(), std::bad_optional_access);
}

TEST(PdargsTest, Numbers) {
  char* argv[] = {"./main", "--port",    "8080",   "-f", "12.25",   "--negate",
                  "-402",   "--portion", "-20.24", "-m", "-20.479", "-z-297"};
  int argc = 12;
  pd::pdargs args(argc, argv);
  
  EXPECT_EQ(args.get<int>({"port", 'p'}), 8080);
  EXPECT_EQ(args.get<float>({"float", 'f'}), 12.25);
  EXPECT_EQ(args.get<long>({"negate", 'n'}), -402);
  EXPECT_EQ(args.get<double>({"portion", 'P'}), -20.24);
  EXPECT_EQ(args.get<double>({"maxxi", 'm'}), -20.479);
  EXPECT_EQ(args.get<int>({"zooba", 'z'}), -297);
}

TEST(PdargsTest, Bools) {
  char* argv[] = {"./main", "-Syu", "-iSr", "--root"};
  int argc = 4;
  pd::pdargs args(argc, argv);
  
  EXPECT_TRUE(args.get<bool>({"yummy", 'y'}));
  EXPECT_TRUE(args.get<bool>({"update", 'u'}));
  EXPECT_TRUE(args.get<bool>({"install", 'i'}));
  EXPECT_TRUE(!args.get<bool>({"update", 'u'}));
  EXPECT_TRUE(!args.get<bool>({"install", 'i'}));

  EXPECT_THROW(args.get<bool>({"root", 'r'}), std::runtime_error);
  EXPECT_THROW(args.get<bool>({"Sus", 'S'}), std::invalid_argument);
}

TEST(PdargsTest, Strings) {
  char* argv[] = {"./main", "--selection", "primary", "--mode", "active", "-f", "soft", "--force", "super"};
  int argc = 9;
  pd::pdargs args(argc, argv);
  
  EXPECT_EQ(args.get<std::string>({"selection", 's'}), "primary");
  EXPECT_EQ(args.get<std::string>({"mode", 'm'}), "active");

  ASSERT_THROW(args.get<std::string>({"mode", 'm'}).value(), std::bad_optional_access);
  ASSERT_THROW(args.get<std::string>({"selection", 's'}).value(), std::bad_optional_access);

  ASSERT_THROW(args.get<std::string>({"force", 'f'}), std::runtime_error);
}

TEST(PdargsTest, GetOr) {
  char* argv[] = {"./main", "--selection", "primary", "-f", "soft", "-c", "55", "-p", "0.5", "--install"};
  int argc{10};
  pd::pdargs args(argc, argv);

  EXPECT_EQ(args.get_or<std::string>({"selection", 's'}, "secondary"), "primary");
  EXPECT_EQ(args.get_or<std::string>({"model", 'm'}, "new"), "new");

  EXPECT_EQ(args.get_or<int>({"count", 'c'}, 100), 55);
  EXPECT_EQ(args.get_or<int>({"number", 'n'}, 128), 128);

  EXPECT_EQ(args.get_or<double>({"portion", 'p'}, 0.25), 0.5);
  EXPECT_EQ(args.get_or<double>({"fraction", 'F'}, 0.75), 0.75);

  EXPECT_TRUE(args.get<bool>({"install", 'i'}));
  EXPECT_TRUE(!args.get<bool>({"update", 'u'}));
}

TEST(PdargsTest, EqualSign) {
  char* argv[] = {"./main", "--selection=primary", "-f=soft", "-c=55", "--portion=0.5"};
  int argc = 5;
  pd::pdargs args(argc, argv);

  EXPECT_EQ(args.get<std::string>({"selection", 's'}), "primary");
  EXPECT_EQ(args.get<std::string>({"force", 'f'}), "soft");
  EXPECT_EQ(args.get<int>({"count", 'c'}), 55);
  EXPECT_EQ(args.get<double>({"portion", 'p'}), 0.5);
}

TEST(PdargsTest, ShortOpts) {
  char* argv[] = {"./main", "-sprimary", "-Wall", "-c155", "-f12.532", "-Xabc"};
  int argc = 6;
  pd::pdargs args(argc, argv);

  EXPECT_EQ(args.get<std::string>({"selection", 's'}), "primary");
  EXPECT_EQ(args.get<std::string>({"warnings", 'W'}), "all");
  EXPECT_EQ(args.get<unsigned long>({"count", 'c'}), 155);
  EXPECT_EQ(args.get<double>({"fraction", 'f'}), 12.532);
  EXPECT_TRUE(args.get<bool>({"XXXXX", 'X'}));
  EXPECT_TRUE(args.get<bool>({"aaaaaa", 'a'}));
  EXPECT_TRUE(args.get<bool>({"cccccc", 'c'}));
  EXPECT_TRUE(args.get<bool>({"bbbbbb", 'b'}));
  EXPECT_TRUE(!args.get<bool>({"XXXXX", 'X'}));
  EXPECT_TRUE(!args.get<bool>({"aaaaaa", 'a'}));
  EXPECT_TRUE(!args.get<bool>({"cccccc", 'c'}));
  EXPECT_TRUE(!args.get<bool>({"bbbbbb", 'b'}));
}

TEST(PdargsTest, TrickyOne)
{
  char* argv[] = { "./main", "-f", "hoo.txt"};
  int argc = 3;
  pd::pdargs args(argc, argv);

  EXPECT_FALSE(args.get<bool>({"help", 'h'}));
  EXPECT_EQ(args.get<std::string>({"file", 'f'}), "hoo.txt");
}

struct person_t {
  std::string name;
  int age;
};

namespace pd {

template<>
person_t string_to_T<person_t>(const std::string& str) {
  auto dot_pos = str.find('.');
  std::string name_str(str, 0, dot_pos);
  std::string age_str(str, dot_pos + 1 );

  person_t ret = { std::move(name_str), string_to_T<int>(age_str) };
  return ret;
}

} // namespace pd

TEST(PdargsTest, CustomStringToT) {
  char* argv[] = { "./main", "-p", "Rel.19" };
  int argc = 3;

  pd::pdargs args(argc, argv);
  auto person = args.get<person_t>({"person", 'p'});
  EXPECT_TRUE(person.has_value());
  EXPECT_EQ(person->name, "Rel");
  EXPECT_EQ(person->age, 19);
}
