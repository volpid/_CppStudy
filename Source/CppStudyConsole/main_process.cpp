
#include <iostream>

/*import library*/
#pragma comment(lib, "static_library.lib")

extern int _main(int argc, char** argv);
int main(int argc, char** argv)
{
    return _main(argc, argv);
}