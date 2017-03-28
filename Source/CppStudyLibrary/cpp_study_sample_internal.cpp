
#include "cpp_study_sample_internal.h"
#include "export/cpp_study_sample.h"

#include <iostream>

using std::cout;
using std::endl;

const int testConst = 2;
static VerbosStruct obj2("global obj");

void CppStudyLibTest(void)
{
    TestClass nanpid;
    TestClass nanpid2(1);

    cout << "test function" << endl;

    MyClass22 a1;
    cout << MyClass22::BytesUsed() << endl;
    MyClass22 a2;
    cout << MyClass22::BytesUsed() << endl;

    cout << "test function end" << endl;
}
