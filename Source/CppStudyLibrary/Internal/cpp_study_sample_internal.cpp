
#include "cpp_study_sample_internal.h"

#include <iostream>

const int testConst = 2;
static VerbosStruct obj2("global obj");

void CppStudyLibTest(void)
{
    TestClass test1;
    TestClass test2(1);

    std::cout << "test function" << std::endl;

    MyClass22 a1;
    std::cout << MyClass22::BytesUsed() << std::endl;
    MyClass22 a2;
    std::cout << MyClass22::BytesUsed() << std::endl;

    std::cout << "test function end" << std::endl;
}
