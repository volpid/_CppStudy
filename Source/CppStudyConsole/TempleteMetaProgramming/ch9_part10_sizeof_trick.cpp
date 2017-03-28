
#include <iostream>
#include <vector>

using namespace std;
namespace boost {namespace mpl{}}
namespace mpl = boost::mpl;

template <typename _T>
struct iis_class_or_union
{
    typedef char yes;
    typedef char (&no)[2];

    template <typename _U>
    static yes checker(int _U::* arg);

    template <typename _U>
    static no checker(...);

    /*
        @Note 
        - check wether class or union
        - use sizeof trick
        !! sizeof   
            works in compile time 
            check return value of function
    */
    static const bool value = sizeof(iis_class_or_union::checker<_T>(0)) == sizeof(yes);
};

union A
{
    int a;
    char b;
    int* c;
};

struct B 
{
    int a;
    char b;
    int* c;
};

template <typename _T>
bool HelpCheck(_T x)
{
    return iis_class_or_union<_T>::value;
}

static int _main(int argc, char** argv)
{
    cout << iis_class_or_union<A>::value << endl;
    cout << iis_class_or_union<B>::value << endl;
    cout << iis_class_or_union<int>::value << endl;

    A a;
    B b;
    a.a = 1;
    b.a = 1;
    int i = 1;

    /*helper function*/
    cout << endl;
    cout << HelpCheck(a) << endl;
    cout << HelpCheck(b) << endl;
    cout << HelpCheck(i) << endl;
    
    return 0;
}