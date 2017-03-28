
#include <assert.h>
#include <exception>
#include <iostream>
#include <stdio.h>

using namespace std;

//------------------------------------------------------------------------------------------
/*compile time error*/
template <bool> 
struct CompileTimeError;

template <>
struct CompileTimeError<true>
{};

template <bool>
struct CompileTimeChecker
{
    CompileTimeChecker(...)
    {}
};

template <>
struct CompileTimeChecker<false>
{
    CompileTimeChecker(...);
};

template <typename _To, typename _From>
_To safe_reinterper_cast(_From from)
{
    assert(sizeof(_From) <= sizeof(_To));
    static_assert(sizeof(_From) <= sizeof(_To), "note ok");
    //static_assert(sizeof(_From) > sizeof(_To), "note error");

#if 0
    #define STATIC_CHECK(expr) (CompileTimeError<(expr) != 0>())
#elif 0
    #define STATIC_CHECK(expr) {char unnamed[(expr) ? (1) : (0)];}
#else 
    #define STATIC_CHECK(expr, msg) \
        {\
            class Error##msg {}; \
            (void) sizeof(CompileTimeChecker<(expr) != 0> ((Error##msg()))); \
        }
#endif /**/

    STATIC_CHECK(sizeof(_From) <= sizeof(_To));
    STATIC_CHECK(sizeof(_From) <= sizeof(_To), msg_compiled);
    /*  
        STATIC_CHECK(sizeof(_From) > sizeof(_To));
        STATIC_CHECK(sizeof(_From) > sizeof(_To), msg_not_compiled);
    */


    return *(reinterpret_cast<_To*> (&from));
}

void CompileTimeErrorCheck(void)
{
    /*
        @Note
        - old time way make compile time error
            CompileTimeChecker<false>();
            ! it makes error, because not defined
            ! we can make error with arr[-1] too!

        -> better use 
            static_assert(<test cond>, "msg")
    */
    static_assert(true, "true");
    CompileTimeChecker<true>("ok message");
    // complite time error
    // CompileTimeChecker<false>("error message");

    int inttype = 1;
    float floattype = 0;
    floattype = safe_reinterper_cast<float>(inttype);
    cout << floattype << endl;
}

//------------------------------------------------------------------------------------------
/*Conversion*/
template <typename _T, typename _U>
class Conversion
{
private:
    typedef char Small;
    struct Big
    {
        Small dummy[2];
    };

    static Small Test(_U);
    static Big Test(...);
    static _T MakeT(void);
public:
    /*
        @Note
        using szieof compile time property
        it calculated in compiletime with return value
    */
    enum {exist = (sizeof(Test(MakeT())) == sizeof(Small))};
};

void ConversionCheck(void)
{
    std::cout << Conversion<double, int>::exist << std::endl;
    std::cout << Conversion<short, int>::exist << std::endl;
    std::cout << Conversion<char*, char>::exist << std::endl;
}

//------------------------------------------------------------------------------------------
/*dispatch*/
template <int N>
struct IntToType
{
    enum
    {
        value = N,
    };
};

template <typename _T, bool isPolymorphic>
struct NiftyContainerValueTraits
{
    typedef _T* ValueType;
};

template <typename _T>
struct NiftyContainerValueTraits<_T, false>
{
    typedef _T ValueType;
};

template <bool flag, typename _T, typename _U>
struct Select
{
    typedef _T Result;
};

template <typename _T, typename _U>
struct Select<false, _T, _U>
{
    typedef _U Result;
};

template <typename _T, bool isPolimorphic>
class NiftyCounter
{
public:
    typedef NiftyContainerValueTraits<_T, isPolimorphic> Traits;
    typedef typename Traits::ValueType ValueType;

    typedef typename Select<isPolimorphic, _T*, _T>::Result ValueType2;

public:
    void DoSomething(_T* pObj)
    {   
        DoSomething(pObj, IntToType<isPolimorphic>());
    }

private:
    void DoSomething(_T* pObj, IntToType<true>)
    {
        std::cout << "DoSomething 1" << std::endl;
    }

    void DoSomething(_T* pObj, IntToType<false>)
    {
        std::cout << "DoSomething 2" << std::endl;
    }
};

void DispatchCheck(void)
{   
    NiftyCounter<int, true> nc1;
    NiftyCounter<int, false> nc2;

    int i = 0;
    char* p = safe_reinterper_cast<char*> (&i);
    nc1.DoSomething(&i);
    nc2.DoSomething(&i);
}

static int _main(int argc, char** argv)
{
    CompileTimeErrorCheck();
    ConversionCheck();
    DispatchCheck();
    
    struct LocalStruct
    {
        //@Note : local structrue doesn't allow static variable
        //static int i;
        int i;
    };
    LocalStruct locals;

    struct TestObj
    {};
    sizeof(CompileTimeChecker<true> ((TestObj())));
    //@Note : below is Function -> to Make param as variable add extra ()
    //sizeof(CompileTimeChecker<true> (TestObj()));

    return 0;
}
