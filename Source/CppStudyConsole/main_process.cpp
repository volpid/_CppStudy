
#include <iostream>

/*import library*/
#pragma comment(lib, "CppStudyStaticLibrary.lib")

extern int _main(int argc, char** argv);
int main(int argc, char** argv)
{
    return _main(argc, argv);
}