

#include <boost/mpl/deref.hpp>
#include <boost/mpl/less.hpp>
#include <boost/mpl/lower_bound.hpp>
#include <boost/mpl/sizeof.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/transform_view.hpp>
#include <boost/mpl/unpack_args.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/zip_view.hpp>

#include <iostream>
#include <vector>

using namespace std;
namespace boost {namespace mpl{}}
namespace mpl = boost::mpl;

/*
    @Problem?
    - in ordered Seq(!increasing) 
    - find first elements's size matches sizeof(e) >= _MinSize
*/

// !simple way
template <typename _Seq, typename _MinSize>
struct padded_size1
    : public mpl::sizeof_<
        typename mpl::deref<
            typename mpl::lower_bound<
                _Seq,
                _MinSize,
                mpl::less<mpl::sizeof_<mpl::_1>, mpl::_2>
            >::type
        >::type
    >
{};

// !use transform to simplify
template <typename _Seq, typename _MinSize>
struct padded_size2
    : public mpl::deref<
        typename mpl::lower_bound<
            typename mpl::transform<
                _Seq, 
                mpl::sizeof_<mpl::_>
            >::type,
            _MinSize
        >::type
    >
{};

/*
    ! final solution
    - use view for efficiency
    - lazy implement : it implements when it used;
*/

template <typename _Seq, typename _MinSize>
struct padded_size3
    : public mpl::deref<
        typename mpl::lower_bound<
            typename mpl::transform_view<
                _Seq, 
                mpl::sizeof_<mpl::_>
            >::type,
            _MinSize
        >::type
    >
{};

static int _main(int argc, char** argv)
{
    typedef mpl::vector<char, int, int, float, double, double> types;
    
    // - mpl::sizeof_<int>
    typename padded_size1<types, mpl::int_<2>>::type result1;
    cout << typeid(result1).name() << endl;
    
    // - mpl::sizeof_<int>
    typename padded_size2<types, mpl::int_<2>>::type result2;
    cout << typeid(result2).name() << endl;

    // - mpl::sizeof_<int>
    typename padded_size3<types, mpl::int_<2>>::type result3;
    cout << typeid(result3).name() << endl;

    /*
        @Note 
        - zip_view / uppack_args example
    */
    mpl::transform_view<
        mpl::zip_view<mpl::vector<int, char, int>>,
        mpl::plus<
            mpl::at<mpl::_, mpl::int_<0>>,
            mpl::at<mpl::_, mpl::int_<1>>,
            mpl::at<mpl::_, mpl::int_<2>>
        >
    >;
    //same as
    mpl::transform_view<
        mpl::zip_view<mpl::vector<int, char, int>>,
        mpl::unpack_args<mpl::plus<mpl::_, mpl::_, mpl::_>>
    >;

    
    return 0;
}