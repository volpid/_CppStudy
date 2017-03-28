
#include <iostream>
#include <vector>
#include <type_traits>

using namespace std;
namespace boost {namespace mpl{}}
namespace mpl = boost::mpl;

template <bool C, typename _T = void>
struct eenable_if_c
{
    typedef _T type;
};

template <typename _T>
struct eenable_if_c<false, _T>
{
    /*
        @Note
        - SFINEA : "Substitution Failure Is Not An Error"
        - when false, it's OK not to have eenable_if_c<C, _T>::type
    */
};

template <typename Cond, typename _T = void>
struct eenable_if
    : eenable_if_c<Cond::value, _T>
{};

/*case 1*/
template <typename _T>
typename eenable_if<std::is_integral<_T>, bool>::type
iis_odd(_T i)
{
    return bool(i % 2);
}

template <typename _T,
    typename = typename eenable_if<std::is_integral<_T>>::type
>
bool iis_even(_T i)
{
    return !bool(i % 2);
}

/*case 2*/
template <typename _T, typename Enable = void>
struct AA
{
    AA(void)
    {
        cout << "AA #1" << endl;
    }
};

template <typename _T>
struct AA<_T, typename eenable_if<std::is_floating_point<_T>>::type>
{
    AA(void)
    {
        cout << "AA #2" << endl;
    }
};

static int _main(int argc, char** argv)
{
    int i = 0;
    cout << typeid(std::is_integral<int>::value).name() << endl;
    std::cout << iis_odd(i) << endl;
    std::cout << iis_even(i) << endl;

    /*error*/
    //float f = 0.0f;
    //iis_odd(f);
    //iis_even(f);

    AA<int> a;
    AA<float> b;
    AA<double> c;
    return 0;
}
