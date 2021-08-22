#include "pd/pdargs.hh"

#define ASSERT(expr, message)                           \
    std::cout << "RUNNING: " << #expr;                  \
    if (!(expr))                                          \
        std::cerr << "- FAILED\n" << __FILE__ << ":"    \
            << __LINE__ << " " << message << '\n';      \
    std::cout << " - PASSED\n";

void test1()
{
    std::cout << " ----- test1 START ----- \n";
    char* argv[] = {"./main", "--port", "8080", "-Syu"};
    int argc = 4;
    pd::pdargs args{argc, argv};
    ASSERT(args.get<int>({"port", 'p'}).value() == 8080, "port option must be equal to 8080");
    ASSERT(args.get<bool>({"sus", 'S'}), "port option must be equal to 8080");
    ASSERT(args.get<bool>({"yuko", 'y'}), "port option must be equal to 8080");
    ASSERT(args.get<bool>({"update", 'u'}), "port option must be equal to 8080");
    ASSERT(!args.get<bool>({"increase", 'i'}), "port option must be equal to 8080");
    ASSERT(!args.get<bool>({"sus", 's'}), "port option must be equal to 8080");
    std::cout << " ----- test1 PASSED ----- \n";
}

int main()
{
    test1();
    
    return 0;
}
