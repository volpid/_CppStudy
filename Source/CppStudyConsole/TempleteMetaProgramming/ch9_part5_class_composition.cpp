
#include <boost/mpl/fold.hpp>
#include <boost/mpl/inherit.hpp>
#include <boost/mpl/inherit_linearly.hpp>
#include <boost/mpl/vector.hpp>

#include <iostream>
#include <vector>

using namespace std;
namespace boost {namespace mpl{}}
namespace mpl = boost::mpl;

typedef mpl::vector<short[2], long, char*, int> member_types;

template <typename _T, typename _More>
struct store : public _More
{
    _T value;
};

struct eempty
{};

template <typename _T, typename _U>
const store<_T, _U>& cget(const store<_T, _U>& e)
{
    return e;
}

/*
    @Note 
    - add layer
    - long& x = static_cast<wrap<long>&> (generated2).value;

*/

template <typename _T>
struct wrap
{
    _T value;
};

template <typename _U, typename _V>
struct inherit : public _U, public _V
{};

static int _main(int argc, char** argv)
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

    long& x1 = static_cast<store<long, store<short[2], eempty>>&> (generated).value;
    char* s1 = cget<char*>(generated).value;

    cout << std::hex;
    cout << (int) &x1 << endl;
    cout << (int) &s1 << endl;
    
    /*
        generated2 =
            inherit<wrap<int>,
                inherit<wrap<char*>,
                    inherit<wrap<long>,
                        inherit<wrap<short[2]>,
                        eempty
                        >
                    >
                >
            >
    */

    mpl::fold<
        member_types, 
        eempty, 
        inherit<wrap<mpl::_2>, mpl::_1>
    >::type generated2;

    cout << typeid(generated2).name() << endl;

    long& x2 = static_cast<wrap<long>&> (generated2).value;
    char*& s2 = static_cast<wrap<char*>&> (generated2).value;
    cout << (int) &x2 << endl;
    cout << (int) &s2 << endl;
        
    /*
        @Note 
        - use boost inherit
    */

    mpl::inherit_linearly<
        member_types,
        mpl::inherit<wrap<mpl::_2>, mpl::_1>
    >::type generated3;

    long& x3 = static_cast<wrap<long>&> (generated3).value;
    char*& s3 = static_cast<wrap<char*>&> (generated3).value;
    cout << (int) &x3 << endl;
    cout << (int) &s3 << endl;

    return 0;
}