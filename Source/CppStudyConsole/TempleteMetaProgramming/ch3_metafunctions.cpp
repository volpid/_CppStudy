
#include <boost/mpl/equal.hpp>
#include <boost/mpl/minus.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/plus.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/vector_c.hpp>

#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

#include <iostream>

using namespace std;
namespace mpl = boost::mpl;

//using Dimention = int[7];
//const Dimention mess = {1, 0, 0, 0, 0, 0, 0};
//const Dimention length = {0, 1, 0, 0, 0, 0, 0};
//const Dimention time = {0, 0, 1, 0, 0, 0, 0};
//const Dimention force = {1, 1, -2, 0, 0, 0, 0};

using Singed_types = boost::mpl::vector<signed char, short, int, long>;

static const int five = mpl::int_<5>::value;

using mass1 = mpl::vector<mpl::int_<1>, mpl::int_<0>, mpl::int_<0>, mpl::int_<0>, mpl::int_<0>, mpl::int_<0>, mpl::int_<0>>;
//using length = mpl::vector<mpl::int_<0>, mpl::int_<1>, mpl::int_<0>, mpl::int_<0>, mpl::int_<0>, mpl::int_<0>, mpl::int_<0>>
//using time = mpl::vector<mpl::int_<0>, mpl::int_<0>, mpl::int_<1>, mpl::int_<0>, mpl::int_<0>, mpl::int_<0>, mpl::int_<0>>
//using charge = mpl::vector<mpl::int_<0>, mpl::int_<0>, mpl::int_<0>, mpl::int_<1>, mpl::int_<0>, mpl::int_<0>, mpl::int_<0>>
//using temperature = mpl::vector<mpl::int_<0>, mpl::int_<0>, mpl::int_<0>, mpl::int_<0>, mpl::int_<1>, mpl::int_<0>, mpl::int_<0>>
//using intensity = mpl::vector<mpl::int_<0>, mpl::int_<0>, mpl::int_<0>, mpl::int_<0>, mpl::int_<0>, mpl::int_<1>, mpl::int_<0>>
//using amount_of_substance = mpl::vector<mpl::int_<0>, mpl::int_<0>, mpl::int_<0>, mpl::int_<0>, mpl::int_<0>, mpl::int_<0>, mpl::int_<1>>
//using velocity = mpl::vector<mpl::int_<0>, mpl::int_<1>, mpl::int_<-1>, mpl::int_<0>, mpl::int_<0>, mpl::int_<0>, mpl::int_<0>>
//using acceleration = mpl::vector<mpl::int_<0>, mpl::int_<1>, mpl::int_<-2>, mpl::int_<0>, mpl::int_<0>, mpl::int_<0>, mpl::int_<0>>
//using momentum = mpl::vector<mpl::int_<1>, mpl::int_<1>, mpl::int_<-1>, mpl::int_<0>, mpl::int_<0>, mpl::int_<0>, mpl::int_<0>>
//using force = mpl::vector<mpl::int_<1>, mpl::int_<1>, mpl::int_<-2>, mpl::int_<0>, mpl::int_<0>, mpl::int_<0>, mpl::int_<0>>

using mass = mpl::vector_c<int, 1, 0, 0, 0, 0, 0, 0>; 
using length = mpl::vector_c<int, 0, 1, 0, 0, 0, 0, 0>;
using time = mpl::vector_c<int, 0, 0, 1, 0, 0, 0, 0>;
using charge = mpl::vector_c<int, 0, 0, 0, 1, 0, 0, 0>;
using temperature = mpl::vector_c<int, 0, 0, 0, 0, 1, 0, 0>;
using intensity = mpl::vector_c<int, 0, 0, 0, 0, 0, 1, 0>;
using amount_of_substance = mpl::vector_c<int, 0, 0, 0, 0, 0, 0, 1>;
using velocity = mpl::vector_c<int, 0, 1, -1, 0, 0, 0, 0>;
using acceleration = mpl::vector_c<int, 0, 1, -2, 0, 0, 0, 0>;
using momentum = mpl::vector_c<int, 1, 1, -1, 0, 0, 0, 0>;
using force = mpl::vector_c<int, 1, 1, -2, 0, 0, 0, 0>;
using scala = mpl::vector_c<int, 0, 0, 0, 0, 0, 0, 0>;

template <typename _T, typename _Dimensions>
struct Quantity
{
    explicit Quantity(_T x)
        : value_(x)
    {}

    _T Value(void) const 
    {
        return value_;
    }

    template <typename _OtherDimentions>
    Quantity(const Quantity<_T, _OtherDimentions> rhs)
        : value_(rhs.Value())
    {   
        BOOST_STATIC_ASSERT((mpl::equal<_Dimensions, _OtherDimentions>::type::value));
    }

private:
    _T value_;
};

template <typename _T, typename _Dimensions>
Quantity<_T, _Dimensions> operator+(Quantity<_T, _Dimensions> x, Quantity<_T, _Dimensions> y)
{
    return Quantity<_T, _Dimensions>(x.Value() + y.Value());
}

template <typename _T, typename _Dimensions>
Quantity<_T, _Dimensions> operator-(Quantity<_T, _Dimensions> x, Quantity<_T, _Dimensions> y)
{
    return Quantity<_T, _Dimensions>(x.Value() - y.Value());
}

BOOST_STATIC_ASSERT((mpl::plus<mpl::int_<2>, mpl::int_<3>>::type::value == 5));

//template <typename _T1, typename _D1, typename _D2>
//Quantity<_T, typename mpl::transform<_D1, _D2, mpl::plus>::type> operator*(Quantity<_T, _D1> x, Quantity<_T, _D2> y)
//{}

struct plus_f
{
    template <typename _T1, typename _T2>
    struct apply
    {
        typedef typename mpl::plus<_T1, _T2>::type type;
    };
};

template <typename _T, typename _D1, typename _D2>
Quantity<_T, typename mpl::transform<_D1, _D2, plus_f>::type> operator*(Quantity<_T, _D1> x, Quantity<_T, _D2> y)
{
    typedef typename mpl::transform<_D1, _D2, plus_f>::type dim;
    return Quantity<_T, dim>(x.Value() * y.Value());
}

template <typename _T, typename _D1, typename _D2>
Quantity<_T, typename mpl::transform<_D1, _D2, mpl::minus<mpl::_1, mpl::_2>>::type> operator/(Quantity<_T, _D1> x, Quantity<_T, _D2> y)
{
    typedef typename mpl::transform<_D1, _D2, mpl::minus<mpl::_1, mpl::_2>>::type dim;
    return Quantity<_T, dim>(x.Value() / y.Value());
}

//template <typename _F, typename _X>
//struct Twice
//{
//    typedef typename _F::template apply<_X>::type once;
//    typedef typename _F::template apply<once>::type type;
//};

/*
    @note 
    - use _Type::template apply
    - apply is related to _Type -> use template keyworkd
*/
//template <typename _F, typename _X>
//struct Twice : public _F::template apply<typename _F::template apply<_X>::type>
//{};

//template <typename _UnaryMatchFucntion, typename _Arg>
//struct apply1 : _UnaryMatchFucntion::typename apply<_Arg>
//{};
//
//template <typename _F, typename _X>
//struct Twice : apply1<_F, typename apply1<_F, _X>::type>
//{};

static int _main(int argc, char** argv)
{
    Quantity<float, length> l(1.0f);
    Quantity<float, mass> m(2.0f);

    //compile time error!
    //m = l;

    cout << l.Value() << endl;
    cout << m.Value() << endl;

    Quantity<float, length> l1(1.0f);
    Quantity<float, length> l2(2.0f);

    Quantity<float, length> l3 = l1 + l2;
    Quantity<float, length> l4 = l1 - l2;

    cout << l3.Value() << endl;
    cout << l4.Value() << endl;
        
    Quantity<float, force> f(1.0f);
    Quantity<float, acceleration> a(1.0f);
    Quantity<float, mass> ms = f / a;
    Quantity<float, force> fc = m * a;

    cout << f.Value() << endl;
    cout << a.Value() << endl;
    cout << ms.Value() << endl;
    cout << fc.Value() << endl;

    

    return 0;
}

