
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/vector.hpp>

#include <algorithm>
#include <iostream>
#include <vector>
#include <type_traits>

using namespace std;
namespace boost {namespace mpl{}}
namespace mpl = boost::mpl;

/* if clause */
struct DummyIf
{
    static const int make_error = -1;
};

template <typename _T>
void f_error(_T)
{
    if (std::is_class<_T>::value)
    {
        cout << "class " << _T::make_error << endl;
    }
    else 
    {
        cout << "not class" << endl;
    }
}

template <bool>
struct f_impl
{
    template <typename _T>
    static void print(_T x)
    {
        cout << "class " << _T::make_error << endl;
    }
};

template <>
struct f_impl<false>
{
    template <typename _T>
    static void print(_T x)
    {
        cout << "not class " << x << endl;
    }
};

template <typename _T>
void f(_T x)
{
    static_assert(is_class<int>::value == false, "int is not class");

    //use other layer to impletement if     
    f_impl<is_class<_T>::value>::print(x);
}

void TestIfClause(void)
{
    f_error(DummyIf());
    //f_error(1); //error because 1 doesn't have make_error

    f(DummyIf());
    f(1);
}

/*tag dispatching*/

/*
    ! std iterator tag

    //	ITERATOR STUFF (from <iterator>)
    // ITERATOR TAGS (from <iterator>)
    struct input_iterator_tag
    {	// identifying tag for input iterators
    };

    struct _Mutable_iterator_tag	// TRANSITION, remove for Dev15
    {	// identifying tag for mutable iterators
    };

    struct output_iterator_tag
    : _Mutable_iterator_tag
    {	// identifying tag for output iterators
    };

    struct forward_iterator_tag
    : input_iterator_tag, _Mutable_iterator_tag
    {	// identifying tag for forward iterators
    };

    struct bidirectional_iterator_tag
    : forward_iterator_tag
    {	// identifying tag for bidirectional iterators
    };

    struct random_access_iterator_tag
    : bidirectional_iterator_tag
    {	// identifying tag for random-access iterators
    };
*/

/*
    ! std iterator category

    template<class _Iter>
	struct _Iterator_traits_base<_Iter, void_t<
		typename _Iter::iterator_category,
		typename _Iter::value_type,
		typename _Iter::difference_type,
		typename _Iter::pointer,
		typename _Iter::reference
		> >
	{	// defined if _Iter::* types exist
	    typedef typename _Iter::iterator_category iterator_category;
        ...
	};
*/

/*
    ! actual dispatching with tag
    _Iter_cat_t<remove_const_t<_InIt> -> iterator_category

    template<class _InIt, class _Diff> inline
	void advance(_InIt& _Where, _Diff _Off)
	{	// increment iterator by offset, arbitrary iterators
		// we remove_const_t before _Iter_cat_t for better diagnostics if the user passes an iterator that is const
	    _Advance1(_Where, _Off, _Iter_cat_t<remove_const_t<_InIt>>());
	}

    // MORE ITERATOR STUFF (from <iterator>)
	// TEMPLATE FUNCTION advance
    template<class _InIt, class _Diff> 
    inline void _Advance1(_InIt& _Where, _Diff _Off, input_iterator_tag)
	{	// increment iterator by offset, input iterators
    	for (; 0 < _Off; --_Off)
		++_Where;
	}

    template<class _BidIt, class _Diff> 
    inline void _Advance1(_BidIt& _Where, _Diff _Off, bidirectional_iterator_tag)
	{	// increment iterator by offset, bidirectional iterators
	    for (; 0 < _Off; --_Off)
		    ++_Where;
	    for (; _Off < 0; ++_Off)
		    --_Where;
	}

    template<class _RanIt, class _Diff> 
    inline void _Advance1(_RanIt& _Where, _Diff _Off, random_access_iterator_tag)
	{	// increment iterator by offset, random-access iterators
	    _Where += _Off;
	}
*/

static int _main(int argc, char** argv)
{
    TestIfClause();

    vector<int> v = {1, 2, 3, 4};
    auto it= v.begin();
    std::advance(it, 1);

    return 0;
}