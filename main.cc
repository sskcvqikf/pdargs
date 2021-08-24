#include "pd/pdargs.hh"

void* print_testname(const char* name)
{
    std::cout << " ----- RUNNING " << name
        << " ----- \n";
    return nullptr;
}

#define TEST(name)                                      \
    void name(void* = print_testname(#name))            \


#define ASSERT(expr, message)                           \
    std::cout << "RUNNING: " << #expr;                  \
    if (!(expr))                                        \
        std::cerr << " - FAILED.\n" << __FILE__ << ":"  \
            << __LINE__ << " " << message << '\n';      \
    else std::cout << " - PASSED.\n";

#define ASSERT_THROW(expr, exc, message)                \
    std::cout << "RUNNING: " << #expr;                  \
    try {                                               \
        expr;                                           \
        std::cerr << " - FAILED." << " No exception.\n" \
        << __FILE__ << ":" << __LINE__ << " "           \
        << message << '\n';                             \
    } catch (exc&) {std::cout << " - PASSED.\n";}       \
      catch (...) {std::cerr << " - FAILED."            \
          << " Other exception.\n" << __FILE__ << ":"   \
          << __LINE__ << " " << message << '\n';};      \


TEST(BaseTest)
{
    char* argv[] = {"./main", "--port", "8080", "-Syu", "--lines",  "blank"};
    int argc = 6;
    pd::pdargs args{argc, argv};
    ASSERT(args.get<int>({"port", 'p'}) == 8080, "port option must be equal to 8080");
    ASSERT(args.get<bool>({"sus", 'S'}), "sus/S must be true");
    ASSERT(args.get<bool>({"yuko", 'y'}), "yuko/y must be true");
    ASSERT(args.get<bool>({"update", 'u'}), "update/u must be true");
    ASSERT(!args.get<bool>({"increase", 'i'}), "increase/i must be false");
    ASSERT(args.get<std::string_view>({"lines", 'l'}) == "blank", "lines option must be equal to \"blank\"");
    ASSERT_THROW(args.get<int>({"port", 'p'}).value(), std::bad_optional_access, "option can be obtained only once.");
}

TEST(TestNumbers)
{
    char* argv[] = {"./main", "--port", "8080", "-f", "12.25", "--negate", "-402", "--portion", "-20.24"};
    int argc = 9;
    pd::pdargs args{argc, argv};
    ASSERT(args.get<int>({"port", 'p'}) == 8080, "port must be equal to 8080");
    ASSERT(args.get<float>({"float", 'f'}) == 12.25, "float must be equal to 12.25");
    ASSERT(args.get<long>({"negate", 'n'}) == -402, "negate must be equal to -402");
    ASSERT(args.get<double>({"portion", 'P'}) == -20.24, "portion must be equal to -20.24");
}

int main()
{
    BaseTest();
    TestNumbers();
    
    return 0;
}
