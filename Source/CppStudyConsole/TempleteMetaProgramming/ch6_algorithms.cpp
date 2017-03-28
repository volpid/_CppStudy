
#include <boost/mpl/equal.hpp>
#include <boost/mpl/equal_to.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mpl/list_c.hpp>
#include <boost/mpl/plus.hpp>
#include <boost/mpl/reverse.hpp>
#include <boost/mpl/shift_right.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/unique.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/vector_c.hpp>

#include <iostream>
#include <vector>

using namespace std;
namespace boost {namespace mpl{}}
namespace mpl = boost::mpl;

/*
    @Note 
    - Basic concep of fold
        fold(Seq, Prev, BiniaryOp) =
            if Seq is empty :
                Prev
            else :
                fold(
                    tail(Seq),
                    BiniaryOp(Prev, head(Seq))
                    BiniaryOp
                )

    ->
        !! Works like functional programming !!
        ! we can traverse Sequence with fold 

        Seq = [a, b, c, d]
        head(Seq) = [a], tail(Seq) = [b, c, d]

    //-------------------------------------------------------------------------------------
    - reverse fold (basic form)

        reverse_fold(Seq, Prev, BiniryOP) =
            if Seq is empty :
                Prev
            else :
                BiniryOP(
                    reverse_fold(tail(Seq), Prev, BiniryOP),
                    head(Seq)
                )

    -> Consider in/out op (actual used one)
            
        reverse_fold(Seq, Prev, OutOp, InOp = _1) = 
            if Seq is empty :
                Prev
            else :
                OutOp(
                    reverse_fold(
                        tail(Seq), 
                        InOp(Prev, head(Seq)),
                        OutOp,
                        InOo),
                    head(Seq)
                )
*/

static int _main(int argc, char** argv)
{
    /*
        @Note
        - sequential construct algorithm
    */
    typedef mpl::vector_c<int, 1, 2, 3> v123;
    typedef mpl::list_c<int, 1, 2, 3> l123;

    cout << typeid(v123).name() << endl;
    cout << typeid(l123).name() << endl;

    typedef mpl::plus<mpl::_1, mpl::int_<5>> add5;

    typedef mpl::transform<v123, add5>::type v678;
    typedef mpl::transform<l123, add5>::type l678;

    cout << endl << "after transformed" << endl << endl;

    cout << typeid(v678).name() << endl;
    cout << typeid(l678).name() << endl;

    /**/
    typedef mpl::transform<v123, add5, mpl::back_inserter<mpl::vector<>>> vv678;
    typedef mpl::reverse_transform<l123, add5, mpl::front_inserter<mpl::list<>>> ll678;
    typedef mpl::reverse_transform<v123, add5, mpl::front_inserter<mpl::list<>>> vl678;

    static_assert(mpl::equal<v678, vv678::type>::value == true, "two vector is same");
    static_assert(mpl::equal<l678, ll678::type>::value == true, "two list is same");
    static_assert(mpl::equal<l678, vl678::type>::value == true, "two list is same");    

    /*
        @Note
        - unique / reverse_unique can make different seq
    */
    typedef mpl::equal_to<
        mpl::shift_right<mpl::_1, mpl::int_<1>>,
        mpl::shift_right<mpl::_2, mpl::int_<1>>
    > same_except_last_bit;

    typedef mpl::vector_c<int, 0, 1, 2, 3, 4, 5> v;
    typedef mpl::vector_c<int, 0, 2, 4> vc024;
    typedef mpl::vector_c<int, 1, 3, 5> vc135;
    
    typedef mpl::unique<v, same_except_last_bit>::type v024;
    typedef mpl::reverse_unique<v, same_except_last_bit>::type v531;
    typedef mpl::reverse<v531>::type v135;
    
    static_assert(mpl::equal<vc024, v024>::value == true, "unique result");
    static_assert(mpl::equal<vc135, v135>::value == true, "reverse_unique result");
    static_assert(mpl::equal<v024, v135>::value == false, "unique/reverse_unique result is diff");

    cout << endl << endl;
    cout << typeid(v024).name() << endl;
    cout << typeid(v531).name() << endl;
    cout << typeid(v135).name() << endl;

    return 0;
}

