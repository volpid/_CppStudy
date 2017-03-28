
#include <boost/type_traits.hpp>

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <type_traits>
#include <vector>

using namespace std;
namespace boost {namespace mpl{}}
namespace mpl = boost::mpl;

template <typename _R, typename _F, typename _G>
class compose_fg
{
public:
    compose_fg(const _F& _f, const _G& _g)
        : f(_f), g(_g)
    {}

    template <typename _T>
    _R operator()(const _T& x) const
    {
        return f(g(x));
    }

private:
    _F f;
    _G g;
};

float sin_squared(float x)
{
    return std::sin(std::sin(x));
}

/*make easy to compose*/
template <typename _R, typename _F, typename _G>
compose_fg<_R, _F, _G> compose(const _F& f, const _G& g)
{
    /*
        @Note
        - function can inference parameter's type 
    */
    return compose_fg<_R, _F, _G>(f, g);
};

/*
    @Note
    - use EBCO (Empty Base Class Object) 
*/

template <typename _R, typename _F, typename _G>
class compose_fg_error : public _F
{
public:
    typedef _R result_type;

    compose_fg_error(const _F& _f, const _G& _g)
        : _F(_f), g(_g)
    {}

    template <typename _T>
    _R operator()(const _T& x)
    {
        const _F& f = *this;
        return f(g(x));
    }

private:
    _G g;
};

/*
    @Solution
    - use other layer
*/

template <typename _F, bool F_empty, typename _G, bool G_empty>
class storage;

template <typename _R, typename _F, typename _G>
class compose_fg_ok
    : public storage<
        _F, std::is_empty<_F>::value,
        _G, std::is_empty<_G>::value
    >
{
public:
    typedef storage<
        _F, std::is_empty<_F>::value,
        _G, std::is_empty<_G>::value
    > base;

    compose_fg_ok(const _F& f, const _G& g)
        : base(f, g)
    {}

    template <typename _T>
    _R operator()(const _T& x)
    {
        const _F& f = this->get_f();
        const _G& g = this->get_g();

        return f(g(x));
    }
};

//specialize above

template <typename _F, typename _G>
class storage<_F, false, _G, false>
{
public:
    storage(const _F& _f, const _G& _g)
        : f(_f), g(_g)
    {}

    const _F& get_f(void) const {return f;}
    const _G& get_g(void) const {return g;}

private:    
    _F f;
    _G g;
};

template <typename _F, typename _G>
class storage<_F, false, _G, true> : public _G
{
public:
    storage(const _F& _f, const _G& _g)
        : _G(g), f(_f)
    {}

    const _F& get_f(void) const {return f;}
    const _G& get_g(void) const {return *this;}

private:    
    _F f;
};

template <typename _F, typename _G>
class storage<_F, true, _G, false> : public _F
{
public:
    storage(const _F& _f, const _G& _g)
        : _F(_f), g(_g)
    {}

    const _F& get_f(void) const {return *this;}
    const _G& get_g(void) const {return g;}

private:    
    _G g;
};

template <typename _F, typename _G>
class storage<_F, true, _G, true> : public _F, public _G
{
public:
    storage(const _F& _f, const _G& _g)
        : _F(_f), _G(_g)
    {}

    const _F& get_f(void) const {return *this;}
    const _G& get_g(void) const {return *this;}
};

static int _main(int argc, char** argv)
{
    float input[5] = {0.0f, 0.1f, 0.2f, 0.3f, 0.4f};
    float output1[5];
    float output2[5];
    float output3[5];
    float output4[5];

    for (float value : input)
    {
        cout << "input : " << value << endl;
    }

    //test set 1
    cout << endl << "test1" << endl;
    float* ignored = std::transform(
        input, 
        input + 5,
        output1,
        compose_fg<float, std::negate<float>, float (*)(float)>(std::negate<float>(), &sin_squared)
    );

    for (float value : output1)
    {
        cout << "input : " << value << endl;
    }

    //test set 2
    cout << endl << "test2" << endl;
    float* ignored2 = std::transform(
        input, 
        input + 5,
        output2,
        /*  
            @Note
            - return type can't be inferenced!
            - _F : std::negate<float>
            - _G : float (*)(float x)
        */
        compose<float>(std::negate<float>(), &sin_squared)
    );

    for (float value : output2)
    {
        cout << "input : " << value << endl;
    }
    
    /*
        @Note 
        - sizeof(compose_fg) == 8

        - negate is empty class
        - ! but class must have size -> sizeof(negate) == 1
            template<class _Ty = void>
            struct negate
            {	// functor for unary operator-
                typedef _Ty argument_type;
                typedef _Ty result_type;

                constexpr _Ty operator()(const _Ty& _Left) const
                {	// apply operator- to operand
                    return (-_Left);
                }
            };        
    */

    //test set 3
    cout << endl << "test3" << endl;

    cout << "compose_fg " << sizeof(compose_fg<float, std::negate<float>, float (*)(float)>) << endl;
    cout << "negate " << sizeof(std::negate<float>) << endl;

    cout << "compose_fg_error " << sizeof(compose_fg_error<float, std::negate<float>, float (*)(float)>) << endl;
    cout << "negate " << sizeof(std::negate<float>) << endl;

    float* ignored3 = std::transform(
        input, 
        input + 5,
        output3,
        compose_fg_error<float, std::negate<float>, float (*)(float)>(std::negate<float>(), &sin_squared)
    );

    //compose_fg_error<float, float (*)(float), std::negate<float>>(std::negate<float>(), &sin_squared)    
    /*
        @Error
        - _F must be templete
    */

    boost::is_empty<int>::value;

    for (float value : output3)
    {
        cout << "input : " << value << endl;
    }

    /*
        @Final solution
        - struct selection
    */

    cout << sizeof(compose_fg_ok<float, std::negate<float>, std::bit_and<float>>) << endl;
    cout << sizeof(compose_fg_ok<float, float (*)(float), std::negate<float>>) << endl;
    cout << sizeof(compose_fg_ok<float, std::negate<float>, float (*)(float)>) << endl;
    cout << sizeof(compose_fg_ok<float, float (*)(float), float (*)(float)>) << endl;

    return 0;
}