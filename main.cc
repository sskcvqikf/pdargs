#include "pd/pdargs.hh"


int main(int argc, char** argv)
{
    pd::pdargs pdarg(argc, argv);
    pdarg.print();
    std::cout << pdarg.get<int>(std::make_pair("port", 'p')).value();
    return 0;
}
