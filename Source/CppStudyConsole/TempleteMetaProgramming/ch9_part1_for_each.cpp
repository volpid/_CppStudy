
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/vector.hpp>

#include <iostream>
#include <vector>
#include <type_traits>

using namespace std;
namespace boost {namespace mpl{}}
namespace mpl = boost::mpl;

/* print types */
struct print_type
{
    template <typename _T>
    void operator()(_T) const
    {
        cout << typeid(_T).name() << endl;
    }
};

template <typename _T>
struct wrap
{};

struct print_type2
{
    template <typename _T>
    void operator()(wrap<_T>) const
    {
        cout << typeid(wrap<_T>).name() << endl;
        cout << typeid(_T).name() << endl;
    }
};

void PrintTypes(void)
{
    typedef mpl::vector<int, long, char*> s;

    //use mpl::for_each
    mpl::for_each<s>(print_type());

    //use wrapper
    mpl::for_each<mpl::transform<s, wrap<mpl::_1>>::type>(print_type2());

    //mpl support below 
    mpl::for_each<s, wrap<mpl::_1>>(print_type2());
}

/* visit types */
struct visit_type
{
    template <typename _Visitor>
    void operator()(_Visitor) const
    {
        //visitor must have static visit function
        _Visitor::visit();
    }
};

template <typename _T>
struct print_visitor
{
    static void visit(void)
    {
        cout << typeid(_T).name() << endl;
    }
};

void VisitTypes(void)
{
    typedef mpl::vector<int, long, char*> s;

    //we don't pass _T object to visit_type
    mpl::for_each<s, print_visitor<mpl::_1>>(visit_type());
}

static int _main(int argc, char** argv)
{
    PrintTypes();
    VisitTypes();

    return 0;
}