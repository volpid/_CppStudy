
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/or.hpp>
#include <boost/type_traits.hpp>

#include <iostream>

namespace mpl = boost::mpl;

template <bool X>
struct bool_
{
    static bool const value = X;
    typedef bool_<X> type;
    typedef bool value_type;
    operator bool(void) const
    {
        return X;
    }
};

typedef bool_<false> false_type;
typedef bool_<true> true_type;

struct fast_swap
{
    template <typename _ForwardIterator1, typename _ForwardIterator2>
    static void do_it(_ForwardIterator1 i1, _ForwardIterator2 i2)
    {
        std::swap(i1, i2);
    }
};

struct reliable_swap
{
    template <typename _ForwardIterator1, typename _ForwardIterator2>
    static void do_it(_ForwardIterator1 i1, _ForwardIterator2 i2)
    {
        
        typename std::iterator_traits<_ForwardIterator1>::value_type tmp = *i1;
        *i1 = *i2;
        *i2 = tmp;
    }
};

template <typename _T>
struct param_type : public mpl::if_<
    typename boost::is_scalar<_T>::type, 
    //_T,
    mpl::identity<_T>,
    //const _T&
    typename boost::add_reference<const _T>::type
>::type
{};

//==
template <typename _C, typename _TrueMetaFunc, typename _FalseMetaFunc>
struct eval_if : public mpl::if_<_C, _TrueMetaFunc, _FalseMetaFunc>::type
{};

template <typename _T>
class Holder
{
public:
    Holder(typename param_type<_T>::type x)
    {
    }

private:
    _T x;
};

template <typename _T>
struct add_reference
{
    typedef _T& type;
};

template <typename _T>
struct param_type2 : mpl::eval_if<
    mpl::or_<
        boost::is_scalar<_T>, 
        boost::is_stateless<_T>,
        boost::is_reference<_T>
    >,
    mpl::identity<_T>,
    add_reference<_T>
>
{};

// 정수 래퍼
template <int N>
struct int_
{
    static const int value = N;
    
    typedef int_<N> type;
    typedef int value_type;
    typedef mpl::int_<N + 1> next;
    typedef mpl::int_<N - 1> prior;

    operator int(void) const
    {
        return N;
    }
};

// 일반 정수 래퍼
template <typename _T, _T N>
struct integral_c
{
    const _T value = N;

    //typedef integral_c<_T, N> value;
    typedef _T value_type;
    typedef mpl::integral_c<_T, N + 1> next;
    typedef mpl::integral_c<_T, N - 1> prior;

    operator int(void) const
    {   
        return N;
    }
};

template <typename _N1, typename _N2>
struct equal_to : mpl::bool_<(_N1::value == _N2::value)>
{};

static int _main(int argc, char** argv)
{
    //mpl::if_<
    //    mpl::bool_<use_swap>,
    //    fast_swap,
    //    reliable_swap>
    //>::type::do_it(i1, i2)

    return 0;
}