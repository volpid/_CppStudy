
#include <boost/mpl/at.hpp>
#include <boost/mpl/aux_/at_impl.hpp>
#include <boost/mpl/back.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/integral_c.hpp>
#include <boost/mpl/iterator_tags.hpp>
#include <boost/mpl/vector.hpp>

#include <boost/static_assert.hpp>
#include <boost/type_index.hpp>

#include <iostream>
#include <vector>

using namespace std;
namespace mpl = boost::mpl;

typedef mpl::vector<char, short, int, long, float, double> types;
typedef mpl::find<types, long>::type long_pos;

void UnusedFunctionStaticAssertIsNotAllowed(void)
{
    //static_assert(false, "error");
    static_assert(true, "ok");
}

template <typename _Iter>
struct next_
{
    typedef typename _Iter::next_ type;
};

template <typename _Iter>
struct deref_
{
    typedef typename _Iter::type type;
};

struct None 
{};

struct Tiny_tag
{};

template <typename _T0 = None, typename _T1 = None, typename _T2 = None>
struct Tiny
{
    typedef Tiny_tag tag;
    typedef Tiny type;
    typedef _T0 t0;
    typedef _T1 t1;
    typedef _T2 t2;
};

template <typename _Tiny, typename _Pos>
struct tiny_iterator 
{
    typedef mpl::random_access_iterator_tag category;
};

template <typename _Tiny, int N>
struct tiny_at;

template <typename _Tiny>
struct tiny_at<_Tiny, 0>
{   
    typedef typename _Tiny::t0 type;
};

template <typename _Tiny>
struct tiny_at<_Tiny, 1>
{   
    typedef typename _Tiny::t1 type;
};

template <typename _Tiny>
struct tiny_at<_Tiny, 2>
{   
    typedef typename _Tiny::t2 type;
};

namespace boost
{
    namespace mpl
    {
        template <typename _Tiny, typename _Pos>
        struct next<tiny_iterator<_Tiny, _Pos>>
        {
            typedef tiny_iterator<_Tiny, typename mpl::next<_Pos>::type> type;
        };

        template <typename _Tiny, typename _Pos>
        struct prior<tiny_iterator<_Tiny, _Pos>>
        {
            typedef tiny_iterator<_Tiny, typename mpl::prior<_Pos>::type> type;
        };

        
        template <typename _T0, typename _T1, typename _T2, typename _Pos>
        struct at<Tiny<_T0, _T1, _T2>, _Pos> : public tiny_at<Tiny<_T0, _T1, _T2>, _Pos::value>
        {};
    
        template <>
        struct at_impl<Tiny_tag>
        {
            template <typename  _Tiny, typename _N>
            struct apply : tiny_at<_Tiny, _N::value>
            {};
        };

        template <typename _Tiny, typename _Pos>
        struct deref<tiny_iterator<_Tiny, _Pos>> : public at<_Tiny, _Pos>
        {};

        template <typename _Tiny, typename _Pos, typename _N>
        struct advance<tiny_iterator<_Tiny, _Pos>, _N>
        {
            typedef tiny_iterator<
                _Tiny, 
                typename mpl::plus<_Pos, _N>::type
            > type;
        };

        template <typename _Tiny, typename _Pos1, typename _Pos2>
        struct distance<
            typename tiny_iterator<_Tiny, _Pos1>,
            typename tiny_iterator<_Tiny, _Pos2>
        > 
            : public mpl::minus<_Pos2, _Pos1>
        {};

        template <>
        struct begin_impl<Tiny_tag>
        {
            template <typename _Tiny>
            struct apply
            {
                typename tiny_iterator<_Tiny, int_<0>> type;
            };
        };

        //////end example 1
        //template <>
        //struct end_impl<Tiny_tag>
        //{
        //    template <typename _Tiny>
        //    struct apply
        //        : public eval_if<
        //            is_same<None, typename _Tiny::t0>,
        //            int_<0>,
        //            eval_if<
        //                is_same<None, typename _Tiny::t1>,
        //                int_<1>,
        //                eval_if<
        //                    is_same<None, typename _Tiny::t1>,
        //                    int_<1>,
        //                    // iterator end!
        //                    int_<2> 
        //                >
        //            >
        //        >
        //    {};
        //};

        template <typename _T0, typename _T1, typename _T2>
        struct Tiny_size : public mpl::int_<3>
        {};

        template <typename _T0, typename _T1>
        struct Tiny_size<_T0, _T1, None> : public mpl::int_<2>
        {};

        template <typename _T0>
        struct Tiny_size<_T0, None, None> : public mpl::int_<1>
        {};

        template <>
        struct Tiny_size<None, None, None> : public mpl::int_<0>
        {};

        template <>
        struct end_impl<Tiny_tag>
        {
            template <typename _Tiny>
            struct apply
            {
                typedef tiny_iterator<
                    _Tiny, 
                    typename Tiny_size<
                        typename _Tiny::t0,
                        typename _Tiny::t1,
                        typename _Tiny::t2
                    >::type
                > type;
            };
        };

        template <>
        struct size_impl<Tiny_tag>
        {
            template <typename _Tiny>
            struct apply 
                : public Tiny_size<
                    typename _Tiny::t0,
                    typename _Tiny::t1,
                    typename _Tiny::t2
                >
            {};
        };

        template <>
        struct clear_impl<Tiny_tag>
        {
            template <typename _Tiny>
            struct apply : public Tiny<>
            {};
        };

        template <>
        struct push_front_impl<Tiny_tag>
        {
            template <typename _Tiny, typename _T>
            struct apply : public Tiny<_T, typename _Tiny::t0, typename _Tiny::t1>
            {};
        };

        template <typename _Tiny, typename _T, int N>
        struct tiny_push_back;

        template <typename _Tiny, typename _T> 
        struct tiny_push_back<_Tiny, _T, 0> : Tiny<_T, None, None>
        {};

        template <typename _Tiny, typename _T> 
        struct tiny_push_back<_Tiny, _T, 1> 
            : public Tiny<
                typename _Tiny::t0, 
                _T, 
                None
            >
        {};

        template <typename _Tiny, typename _T> 
        struct tiny_push_back<_Tiny, _T, 2 > 
            : public Tiny<
                typename _Tiny::t0, 
                typename _Tiny::t1, 
                _T
            >
        {};
    }
}

static int _main(int argc, char** argv)
{
    static_assert(true, "no error");

    std::vector<int> x(10);
    std::vector<int>::iterator five_pos = std::find(x.begin(), x.end(), 5);

    typedef mpl::end<types> finish;
    BOOST_STATIC_ASSERT(!(boost::is_same<long_pos, finish>::value));
    static_assert(!boost::is_same<long_pos, finish>::value, "boost error");

    cout << typeid(long_pos).name() << endl;
    cout << boost::typeindex::type_id<long_pos>().raw_name() << endl;
    cout << boost::typeindex::type_id<long_pos>().pretty_name() << endl;

    //dereference
    typedef mpl::deref<long_pos>::type xType;
    cout << typeid(xType).name() << endl;
    cout << boost::typeindex::type_id<xType>().raw_name() << endl;
    cout << boost::typeindex::type_id<xType>().pretty_name() << endl;

    static_assert(boost::is_same<xType, long>::value, "not same?");

    typedef mpl::front<types>::type types_begin;
    typedef mpl::deref<
        mpl::begin<types>::type
    >::type types_begin2;

    cout << typeid(types_begin).name() << endl;
    cout << boost::typeindex::type_id<types_begin>().raw_name() << endl;
    cout << boost::typeindex::type_id<types_begin>().pretty_name() << endl;

    cout << typeid(types_begin2).name() << endl;
    cout << boost::typeindex::type_id<types_begin2>().raw_name() << endl;
    cout << boost::typeindex::type_id<types_begin2>().pretty_name() << endl;

    static_assert(boost::is_same<types_begin, types_begin2>::value, "type_begin is same");

    typedef mpl::back<types>::type type_end1;
    typedef mpl::deref<
        mpl::prior<
            mpl::end<types>::type        
        >::type
    >::type type_end2;

    cout << typeid(type_end1).name() << endl;
    cout << typeid(type_end2).name() << endl;
    static_assert(boost::is_same<type_end1, type_end2>::value, "type_end2 is same");


    typedef mpl::at<types, mpl::int_<2>>::type types_at2_1;
    typedef mpl::deref<
        mpl::advance<
            mpl::begin<types>::type,
            mpl::int_<2>
        >::type
    >::type types_at2_2;
    
    cout << typeid(mpl::int_<2>).name() << endl;
    cout << typeid(mpl::int_<2>::type).name() << endl;

    cout << typeid(types_at2_1).name() << endl;
    cout << typeid(types_at2_2).name() << endl;
    static_assert(boost::is_same<types_at2_1, types_at2_2>::value, "type_end2 is same");
    
    return 0;
}