
#include <iostream>
#include <vector>

#include <type_traits>

std::is_union<int>;

struct true_type_
{
    enum {value = true};
};

struct false_type_
{
    enum {value = false};
};

template <typename _T>
struct RemoveConst
{
    typedef _T Type;
};

template <typename _T>
struct RemoveConst<const _T>
{
    typedef _T Type;
};

template <typename _T1, typename _T2>
struct IsSame : public false_type_
{};

template <typename _T>
struct IsSame<_T, _T> : public true_type_
{};

template <typename _T>
struct IsVoid : IsSame<typename RemoveConst<_T>::Type, void>
{};

template <typename _T>
struct IsNullptr: IsSame<typename RemoveConst<_T>::Type, std::nullptr_t>
{};

template <bool value = false>
struct ValueType : public false_type_
{};

template <>
struct ValueType<true> : public true_type_
{};

#define _IS_INTERGRAL_SEPCIALIZATION_(x) \
    template <> \
    struct is_integral_base<x> : public true_type_ \
    {} 

template <typename _T>
struct is_integral_base : public false_type_
{};

_IS_INTERGRAL_SEPCIALIZATION_(signed int);
_IS_INTERGRAL_SEPCIALIZATION_(signed short);
_IS_INTERGRAL_SEPCIALIZATION_(signed long);
_IS_INTERGRAL_SEPCIALIZATION_(signed long long);
_IS_INTERGRAL_SEPCIALIZATION_(unsigned int);
_IS_INTERGRAL_SEPCIALIZATION_(unsigned short);
_IS_INTERGRAL_SEPCIALIZATION_(unsigned long);
_IS_INTERGRAL_SEPCIALIZATION_(unsigned long long);
_IS_INTERGRAL_SEPCIALIZATION_(char16_t);
_IS_INTERGRAL_SEPCIALIZATION_(char32_t);
//_IS_INTERGRAL_SEPCIALIZATION_(wchar_t);

template <typename _T>
struct isIntergral : is_integral_base<typename RemoveConst<_T>::Type>
{};

template <typename _T>
struct isFloatPoint : public IsSame<typename RemoveConst<_T>::Type, float>
{};

template <bool value = false>
struct value_type : public false_type_
{};

template <>
struct value_type<true> : public true_type_
{};

template <typename _T>
struct IsArithmetic 
    : public value_type<isIntergral<_T>::value || isFloatPoint<_T>::value>
{};

template <typename _T>
struct IsFundamental 
    : public value_type<
        IsArithmetic<_T>::value 
        || IsNullptr<_T>::value
        || IsVoid<_T>::value
    >
{};

template <typename _T>
struct IsCommpound : public value_type<!IsFundamental<_T>::value>
{};

template <typename _T>
struct IsPointerBase : public false_type_
{};

template <typename _T>
struct IsPointerBase<_T*> : public true_type_
{};

template <typename _T>
struct IsPointer : IsPointerBase<typename RemoveConst<_T>::Type>
{};

template <typename _T>
struct IsReference : public false_type_
{};

template <typename _T>
struct IsReference<_T&> : public true_type_
{};

template <typename _T>
struct IsReference<_T&&> : public true_type_
{};

template <typename _T>
struct IsConst : public false_type_
{};

template <typename _T>
struct IsConst<const _T> : public true_type_
{};

template <typename _T>
struct IsVolatile : public false_type_
{};

template <typename _T>
struct IsVolatile<volatile _T> : public true_type_
{};

template <typename _T, bool IsArithMatic_ = IsArithmetic<_T>::value>
struct IsSignedBase : value_type<_T(-1) < _T(0)>
{};

template <typename _T>
struct IsSignedBase<_T, false> : public false_type_
{};

template <typename _T>
struct IsSigned : public IsSignedBase<_T>
{};

template <typename _T, bool IsArithMatic_ = IsArithmetic<_T>::value>
struct IsUnsignedBase : value_type<_T(0) < _T(-1)>
{};

template <typename _T>
struct IsUnsignedBase<_T, false> : public false_type_
{};

template <typename _T>
struct IsUnsigned : public IsUnsignedBase<_T>
{};

template <typename _T>
struct IsClass
{
    struct Two
    {
        char c[2];
    };

    template <typename _U>
    static char IsClassTester(void (_U::*)(void));
    template <typename _U>
    static Two IsClassTester(...);

    /*
        @Note
        - use sizeof trick
        - ignore union case!
    */

    enum
    {
        value = sizeof(IsClassTester<_T>(0)) == sizeof(char),
    };
};

class AA
{};

static int _main(int argc, char** argv)
{
    std::cout << "IsSame<int, float> " << IsSame<int, float>::value << std::endl;
    std::cout << "IsSame<int, int> " << IsSame<int, int>::value << std::endl;
    std::cout << std::endl;

    std::cout << "IsVoid<int> " << IsVoid<int>::value << std::endl;
    std::cout << "IsVoid<void> " << IsVoid<void>::value << std::endl;
    std::cout << "IsVoid<const void> " << IsVoid<const void>::value << std::endl;
    std::cout << std::endl;

    std::cout << "IsNullptr<int*> " << IsNullptr<int*>::value << std::endl;
    std::cout << "IsNullptr<std::nullptr_t> " << IsNullptr<std::nullptr_t>::value << std::endl;
    std::cout << "IsNullptr<decltype(nullptr)> " << IsNullptr<decltype(nullptr)>::value << std::endl;
    std::cout << "IsNullptr<const std::nullptr_t> " << IsNullptr<const std::nullptr_t>::value << std::endl;
    std::cout << std::endl;

    std::cout << "isIntergral<int>::value " << isIntergral<int>::value << std::endl;
    std::cout << "isIntergral<const unsigned long long> " << isIntergral<const unsigned long long>::value << std::endl;
    std::cout << "isIntergral<int*> " << isIntergral<int*>::value << std::endl;
    std::cout << "isIntergral<float> " << isIntergral<float>::value << std::endl;
    std::cout << std::endl;

    std::cout << "IsArithmetic<AA> " << IsArithmetic<AA>::value << std::endl;
    std::cout << "IsArithmetic<int> " << IsArithmetic<int>::value << std::endl;
    std::cout << "IsArithmetic<int&> " << IsArithmetic<int&>::value << std::endl;
    std::cout << "IsArithmetic<float*> " << IsArithmetic<float*>::value << std::endl;
    std::cout << std::endl;

    std::cout << "IsFundamental<AA> " << IsFundamental<AA>::value << std::endl;
    std::cout << "IsFundamental<int> " << IsFundamental<int>::value << std::endl;
    std::cout << "IsFundamental<int&> " << IsFundamental<int&>::value << std::endl;
    std::cout << "IsFundamental<float*> " << IsFundamental<float*>::value << std::endl;
    std::cout << std::endl;

    std::cout << "IsCommpound<AA>" << IsCommpound<AA>::value << std::endl;
    std::cout << "IsCommpound<int> " << IsCommpound<int>::value << std::endl;
    std::cout << "IsCommpound<int&> " << IsCommpound<int&>::value << std::endl;
    std::cout << "IsCommpound<float*>::value " << IsCommpound<float*>::value << std::endl;
    std::cout << std::endl;

    std::cout << "IsPointer<AA> " << IsPointer<AA>::value << std::endl;
    std::cout << "IsPointer<int> " << IsPointer<int>::value << std::endl;
    std::cout << "IsPointer<int&> " << IsPointer<int&>::value << std::endl;
    std::cout << "IsPointer<float*> " << IsPointer<float*>::value << std::endl;
    std::cout << std::endl;

    std::cout << "IsReference<AA> " << IsReference<AA>::value << std::endl;
    std::cout << "IsReference<int> " << IsReference<int>::value << std::endl;
    std::cout << "IsReference<int&> " << IsReference<int&>::value << std::endl;
    std::cout << "IsReference<float*> " << IsReference<float*>::value << std::endl;
    std::cout << std::endl;

    std::cout << "IsSigned<AA> " << IsSigned<AA>::value << std::endl;
    std::cout << "IsSigned<int> " << IsSigned<int>::value << std::endl;
    std::cout << "IsSigned<unsigned int> " << IsSigned<unsigned int>::value << std::endl;
    std::cout << "IsSigned<int&>::value " << IsSigned<int&>::value << std::endl;
    std::cout << "IsSigned<float*>::value " << IsSigned<float*>::value << std::endl;
    std::cout << std::endl;

    std::cout << "IsUnsigned<AA> " << IsUnsigned<AA>::value << std::endl;
    std::cout << "IsUnsigned<int> " << IsUnsigned<int>::value << std::endl;
    std::cout << "IsUnsigned<unsigned int> " << IsUnsigned<unsigned int>::value << std::endl;
    std::cout << "IsUnsigned<int&> " << IsUnsigned<int&>::value << std::endl;
    std::cout << "IsUnsigned<float*> " << IsUnsigned<float*>::value << std::endl;
    std::cout << std::endl;
    
    std::cout << "IsClass<AA> " << IsClass<AA>::value << std::endl;
    std::cout << "IsClass<int> " << IsClass<int>::value << std::endl;
    std::cout << "IsClass<int&> " << IsClass<int&>::value << std::endl;
    std::cout << "IsClass<float*> " << IsClass<float*>::value << std::endl;

    return 0;
}