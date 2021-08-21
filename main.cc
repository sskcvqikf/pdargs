#include "pd/pdargs.hh"


int main(int argc, char** argv)
{
    pd::pdargs pdarg(argc, argv);
    pdarg.print();
    return 0;
}
