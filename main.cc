#include "pd/pdargs.hh"


int main(int argc, char** argv)
{
    pd::pdargs pdarg(argc, argv);
    std::cout << pdarg.get<int>(std::make_pair("port", 'p')).value() << '\n';
    std::cout << pdarg.get<bool>(std::make_pair("serial", 's')).value() << '\n';
    std::cout << pdarg.get<bool>(std::make_pair("foo", 'f')).value() << '\n';
    std::cout << pdarg.get<bool>(std::make_pair("bar", 'b')).value() << '\n';
    std::cout << pdarg.get<bool>(std::make_pair("sel", 'S')).value() << '\n';
    std::cout << pdarg.get<double>(std::make_pair("part", 'P')).value() << '\n';
    std::cout << pdarg.get<unsigned>(std::make_pair("number", 'n')).value() << '\n';
    pdarg.print();
    return 0;
}
