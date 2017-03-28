
#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <vector>

using namespace std;
namespace boost {namespace mpl{}}
namespace mpl = boost::mpl;

/*
    @Note   
    - below can be templete arg
        Pointers and references to specific functions
        Pointers and references to statically stored data
        Pointers to member functions
        And pointers to data members
*/

template <typename _R, typename _F, _F f, typename _G, _G g>
struct compose_fg2
{
    typedef _R result_type;

    template <typename _T>
    _R operator()(const _T& x) const
    {
        return f(g(x));
    }
};

float llog2(float x) 
{
    return std::log(x) / std::log(2.0f);
}

float sin_squared2(float x)
{
    return std::sin(std::sin(x));
}

using FloatFunc = float (*)(float);

static int _main(int argc, char** argv)
{
    float input[5] = {0.0f, 0.1f, 0.2f, 0.3f, 0.4f};
    float output[5];

    using ComposeFG = compose_fg2<float, FloatFunc, sin_squared2, FloatFunc, llog2>;

    float* ignored = std::transform(
        input,
        input + 5,
        output,
        ComposeFG());

    /*
        @Note
        - fucntion F / G is not member valable 
        - sizeof(ComposeFG)!
    */

    cout << typeid(ComposeFG).name() << std::endl;
    cout << "! size : " << sizeof(ComposeFG) << endl;

    return 0;
}