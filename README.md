# PDARGS
Poorly designed arguments
#### Description:
A small library that allows you to process command line arguments. Pretty simple.

I cannot assure that it is POSIX/GNU compliant.
#### Usage
Just include `include/pd/pdargs.h` somewhere in your project.
#### Example #1
Lets consider simple ping program. It is required to provide hostname to ping. We will name this parameter as `hostname` or `h` as short variant. Two optional parameters are `number/n` - number of bytes to send, and `count/c` - number of replies we need.
```c++
#include <iostream>
#include "pd/pdargs.h"

int main(int argc, char **argv)
{
    pd::pdargs args{ argc, argv };
    
    auto hostname = args.get<std::string>({ "hostname", 'h' });
    if (!hostname)
    {
        std::cerr << "You have to provide hostname to ping!\n";
        return 1;
    }
    auto n = args.get_or<int>({"number", 'n'}, 32);
    auto c = args.get_or<int>({"count", 'c'}, 256);
    std::cout << "Hostname: " << hostname.value()
        << "\nNumber: " << n
        << "\nCount: " << c << '\n';
    return 0;
}

```
Result:
```bash
: ./main
You have to provide hostname to ping!
: ./main --hostname archlinux.org
Hostname: archlinux.org
Number: 32
Count: 256
: ./main -harchlinux.org --count=4
Hostname: archlinux.org
Number: 32
Count: 4
: ./main -harchlinux.org -c=4 -n64
Hostname: archlinux.org
Number: 64
Count: 4
```
#### Example #2
Lets consider another program I am tired to explain. There will be so many bool arguments.
```c++
#include <iostream>
#include "pd/pdargs.h"

int main(int argc, char **argv)
{
    pd::pdargs args{ argc, argv };

    auto print_if_true = [](bool p, std::string str)
        {
            if(p)
                std::cout << str << '\n';
        };

    auto sync = args.get<bool>({"sync", 'S'});
    auto sysupgrade = args.get<bool>({"sysupgrade", 'u'});
    auto refresh = args.get<bool>({"refresh", 'y'});

    auto ambiguous = args.get<std::string>({"stategy", 's'});

    print_if_true(sync, "sync is enabled");
    print_if_true(sysupgrade, "sysupgrade is enabled");
    print_if_true(refresh, "refresh is enabled");
    if (ambiguous)
        std::cout << "Strategy is: " << ambiguous.value() << '\n';
    
    return 0;
}
```
Result:
```bash
: ./main -Syu
sync is enabled
sysupgrade is enabled
refresh is enabled
: ./main --sync --sysupgrade -y
sync is enabled
sysupgrade is enabled
refresh is enabled
: ./main -Syu -syank
terminate called after throwing an instance of 'std::invalid_argument'
  what():  Short options for bool presented in multiple variants.
```
#### API
##### constructor
Constructor receives `argc` and `argv`. Than two storages are constructed: first is a map for long options and second is vector for short options. This is obvious how the map is constructed.

Vector represents each short option as one string. For example: `-Syu` will be stored as `Syu`, `-m=auto` or `-mauto` or `-m auto` will be stored as `mauto`.

##### get
get just takes a pair `{std::string, char}` and tries to find desired option. Returns `std::optional` of desired type.

If it finds one than that option will be removed from storages, so that the next attempt to get same option will be failed.

Note: ambiguity can appear. Consider the following options: `-Syu -syank`. Attempt to get bool option with `y` as short variant will fail because `y` is present in two strings. That's the ambiguity. In order to deal with it you should always pick valued options before bool options.

##### get_or
get_or takes a pair `{std::string, char}` and a default value to return if search is failing. Pretty much the same as `get` but if the search is failed returns second argument instead of `std::optional`. `get_or<bool>` will fail to compile, because such call makes no sense. 
