
#include <assert.h>
#include <exception>
#include <iostream>
#include <stdio.h>

using namespace std;

//class WINDOW {};
//class BUTTON {};
//class SCROLLBAR {};
//
//class WidgetFactory
//{
//public:
//    virtual WINDOW* CreateWindow(void) {return nullptr;}
//    virtual BUTTON* CreateButton(void) {return nullptr;}
//    virtual SCROLLBAR* CreateScrollbar(void) {return nullptr;}
//    
//};
//
//template <typename _T>
//_T* MakeRedWidget(WidgetFactory& factory)
//{
//    _T* pW = factory.Create<_T> ();
//    pW->SetColor(RED);
//    return pW;
//}

template <typename _U, typename _T>
struct Typelist
{
    typedef _U Head;
    typedef _T Tail;
};

struct Nulltype
{};

#define TypeList_1(T1) Typelist<T1, Nulltype>
#define TypeList_2(T1, T2) Typelist<T1, TypeList_1(T2)>
#define TypeList_3(T1, T2, T3) Typelist<T1, TypeList_2(T2, T3)>
#define TypeList_4(T1, T2, T3, T4) Typelist<T1, TypeList_3(T2, T3, T4)>

template <typename _TList> 
struct Length;

template <>
struct Length<Nulltype>
{
    enum {value  = 0};
};

template <typename _T, typename _U>
struct Length<Typelist<_T, _U>>
{
    enum {value = 1 + Length<_U>::value};
};

template <typename TList, unsigned int Index>
struct TypeAt;

template <typename _Head, typename _Tail>
struct TypeAt<Typelist<_Head, _Tail>, 0>
{
    typedef _Head Result;
};

template <typename _Head, typename _Tail, unsigned int i>
struct TypeAt<Typelist<_Head, _Tail>, i>
{
    typedef typename TypeAt<_Tail, i - 1>::Result Result;
};

template <typename _TList, typename _Type>
struct IndexOf;

template <typename _T>
struct IndexOf<Nulltype, _T>
{
    enum {value = -1};
};

template <typename _T, typename _Tail>
struct IndexOf<Typelist<_T, _Tail>, _T>
{
    enum {value = 0};
};

template <typename _Head, typename _Tail, typename _T>
struct IndexOf<Typelist<_Head, _Tail>, _T>
{
    enum {temp = IndexOf<_Tail, _T>::value};
    enum {value = ((temp == -1) ? (-1) : (1 + temp))};
};

template <typename Tlist, typename _T>
struct Append;

template <>
struct Append<Nulltype, Nulltype>
{
    typedef Nulltype Result;
};

template <typename _T>
struct Append<Nulltype, _T>
{
    typedef TypeList_1(_T) Result;
};

template <typename _Head, typename _Tail>
struct Append<Nulltype, Typelist<_Head, _Tail>>
{
    typedef Typelist<_Head, _Tail> Result;
};

template <typename _Head, typename _Tail, typename _T>
struct Append<Typelist<_Head, _Tail>, _T>
{
    typedef Typelist<_Head, typename Append<_Tail, _T>::Result> Result;
};

template <typename _Tlist, typename _T>
struct Erase;

template <typename _T>
struct Erase<Nulltype, _T>
{
    typedef Nulltype Result;
};

template <typename _T, typename _Tail>
struct Erase<Typelist<_T, _Tail>, _T>
{
    typedef _Tail Result;
};

template <typename _T, typename _Head, typename _Tail>
struct Erase<Typelist<_Head, _Tail>, _T>
{
    typedef Typelist<_Head, typename Erase<_Tail, _T>::Result> Result;
};

template <typename _Tlist, typename _T>
struct EraseAll;

template <typename _T>
struct EraseAll<Nulltype, _T>
{
    typedef Nulltype Result;
};

template <typename _T, typename _Tail>
struct EraseAll<Typelist<_T, _Tail>, _T>
{
    typedef typename EraseAll<_Tail, _T>::Result Result;
};

template <typename _T, typename _Head, typename _Tail>
struct EraseAll<Typelist<_Head, _Tail>, _T>
{
    typedef Typelist<_Head, typename Erase<_Tail, _T>::Result> Result;
};

template <typename _Tlist> 
struct NoDuplicates;

template <>
struct NoDuplicates<Nulltype>
{
    typedef Nulltype Result;
};

template <typename _Head, typename _Tail>
struct NoDuplicates<Typelist<_Head, _Tail>>
{
    //typedef typename EraseAll<_Tail, _Head>::Result S1;
    //typedef typename Append<_Head, S1>::Result Result; 

    typedef typename NoDuplicates<_Tail>::Result S1;
    typedef typename Erase<S1, _Head>::Result S2;
    typedef Typelist<_Head, S2> Result;
};

template <typename _Tlist, typename _T, typename _R>
struct Replace;

template <typename _T, typename _R>
struct Replace<Nulltype, _T, _R>
{
    using Result = Nulltype;
};

template <typename _Tail, typename _T, typename _R>
struct Replace<Typelist<_T, _Tail>, _T, _R>
{
    using Result = Typelist<_R, _Tail>;
};

template <typename _Head, typename _Tail, typename _T, typename _R>
struct Replace<Typelist<_Head, _Tail>, _T, _R>
{
    using Result = Typelist<_Head, typename Replace<_Tail, _T, _R>::Result>;
};

template <typename Tlist, template <typename> class Unit>
class GenScatterHierachy;

template <typename _T1, typename _T2, template <typename> class Unit>
class GenScatterHierachy<Typelist<_T1, _T2>, Unit>
    : public GenScatterHierachy<_T1, Unit>, public GenScatterHierachy<_T2, Unit>
{
public:
    using TList = Typelist<_T1, _T2>;
    using LeftBase = GenScatterHierachy<_T1, Unit>;
    using RightBase = GenScatterHierachy<_T2, Unit>;
};

template <typename _AtomicType, template <typename> class Unit>
class GenScatterHierachy : public Unit<_AtomicType>
{
public:
    using LeftBase = Unit<_AtomicType>;
};

template <template <typename> class Unit>
class GenScatterHierachy<Nulltype, Unit>
{};

template <typename _T>
struct Holder
{
    _T value;
};

class EmptyType {};

typedef GenScatterHierachy<TypeList_3(int, int, char), Holder> WidgetInfo;

template <
    typename _TList, 
    template <typename _AtomicType, typename _Base> class _Unit,
    typename _Root = EmptyType
>
class GenLinearHierachy;

template <
    typename _T1, 
    typename _T2, 
    template <typename, typename> class _Unit,
    typename _Root
>
class GenLinearHierachy<Typelist<_T1, _T2>, _Unit, _Root>
    : public _Unit<_T1, GenLinearHierachy<_T2, _Unit, _Root>>
{
public:    
};

template <
    typename _T, 
    template <typename, typename> class _Unit,
    typename _Root
>
class GenLinearHierachy<TypeList_1(_T), _Unit, _Root> 
    : public _Unit<_T, _Root>
{
public:    
};

static int _main(int argc, char** argv)
{
    TypeList_3(int, int, char) IntIntChar;
    cout << Length<decltype(IntIntChar)>::value << endl;
    cout << Length<Nulltype>::value << endl;

    cout << typeid(TypeAt<decltype(IntIntChar), 0>::Result).name() << endl;
    cout << typeid(TypeAt<decltype(IntIntChar), 2>::Result).name() << endl;

    cout << IndexOf<decltype(IntIntChar), int>::value << endl;
    cout << IndexOf<decltype(IntIntChar), char>::value << endl;
    cout << IndexOf<decltype(IntIntChar), char*>::value << endl;

    cout << typeid(decltype(IntIntChar)).name() << endl;
    cout << typeid(Append<decltype(IntIntChar), Nulltype>::Result).name() << endl;
    cout << typeid(Append<decltype(IntIntChar), decltype(IntIntChar)>::Result).name() << endl;
    cout << typeid(Append<decltype(IntIntChar), char>::Result).name() << endl;
    
    cout << typeid(Erase<decltype(IntIntChar), int>::Result).name() << endl;
    cout << typeid(EraseAll<decltype(IntIntChar), int>::Result).name() << endl;

    WidgetInfo wi;
    cout << typeid(wi).name() << endl;
    cout << sizeof(wi) << endl;

    return 0;
}

