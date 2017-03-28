
#ifndef __CPP_STUDY_SAMPLE_INTERNAL__H__
#define __CPP_STUDY_SAMPLE_INTERNAL__H__

#include "export/memory_tracker.h"

#include <string>
#include <iostream>

struct VerbosStruct
{
    VerbosStruct(const std::string& str)
    {
        id = str;
        std::cout << "Constuct - " << id.c_str() << std::endl;
    }

    ~VerbosStruct(void)
    {
        std::cout << "Destruct - " << id.c_str() << std::endl;
    }
    std::string id;
};


class MyClass22 : public MemoryTracker<MyClass22>
{
public:
    int k;
};

class TestClass
{
public:
    TestClass(void)
    {
        x = 0;
    }

    inline TestClass(int xx);

private:
    int x;
};

inline TestClass::TestClass(int xx)
{   
    x = xx;
}

#endif /*__CPP_STUDY_SAMPLE_INTERNAL__H__*/ 