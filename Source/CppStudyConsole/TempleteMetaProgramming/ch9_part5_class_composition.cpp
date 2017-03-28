
#include <boost/mpl/fold.hpp>
#include <boost/mpl/vector.hpp>

#include <iostream>
#include <vector>

using namespace std;
namespace boost {namespace mpl{}}
namespace mpl = boost::mpl;

template <typename _T, typename _More>
struct store : public _More
{
    typedef _T value;
};

typedef mpl::vector<short[2], long, char, int> member_types;

struct eempty
{};

template <typename _T, typename _U>
const store<_T, _U>& cget(const store<_T, _U>& e)
{
    return e;
}

int _main(int argc, char** argv)
{
    /*
        generated =
            store<int,
                store<char*,
                    store<long,
                        store<short[2], eempty>
                    >
                >
            >
    */

    mpl::fold<
        member_types, 
        eempty,
        store<mpl::_2, mpl::_1>
    >::type generated;
    cout << typeid(generated).name() << endl;

    //char* s = cget<char*>(generated).value;

    return 0;
}