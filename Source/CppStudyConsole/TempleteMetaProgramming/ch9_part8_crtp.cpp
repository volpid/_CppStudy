
#include <cassert>
#include <iostream>
#include <list>
#include <vector>

using namespace std;
namespace boost {namespace mpl{}}
namespace mpl = boost::mpl;

/*
    @Note
    - CRTP : Curiously Recurring Templete Pattern

    - basic form

        class X : public base<X>
        {}
*/

template <typename _T>
struct ordered
{   
    bool operator>(const _T& rhs) const
    {
        const _T& self = static_cast<const _T&>(*this);
        return rhs < self;
    }
};

class Int : public ordered<Int>
{
public:
    explicit Int(int x) 
        : value(x)
    {}

    bool operator<(const Int& rhs) const
    {
        return this->value < rhs.value;
    }
        
private:
    int value;
};

/*none member friend function*/
namespace crtp
{
    template <typename _T>
    struct signed_nubmer
    {
        friend _T abs(_T x)
        {
            cout << "call crtp::signed_nubmer()" << endl;
            cout << typeid(_T).name() << endl;
            return (x < 0.0f) ? (-x) : (x);
        }
    };
}

class Float : crtp::signed_nubmer<Float>
{
public:
    Float(float x) 
        : value_(x)
    {}

    Float operator-(void) const
    {
        cout << " Float operator-(void) const " << value_ << endl;
        return Float(-value_);
    }

    bool operator<(float x) const
    {
        cout << " Float operator<(float x) const " << value_ << endl;
        return value_ < x;
    }

private:
    float value_;
};

/*ambiguous*/

namespace utility
{
    template <typename _Iterator>
    void clear(const _Iterator& start, const _Iterator& finish)
    {
        cout << "clear #1" << endl;
    }

    template <typename _Iterator>
    int merge(_Iterator start, _Iterator finish)
    {
        cout << " call int merge(_Iterator start, _Iterator finish)" << endl;
        clear(start, finish);

        return 0;
    }
}

/*solve ambiguous*/

template <typename _Dervied>
struct vehicle
{};

template <typename _Derived>
struct screw
{};

template <typename _Vehicle>
void ddrive(const vehicle<_Vehicle> v)
{
    const _Vehicle& v1 = static_cast<const _Vehicle&> (v);
    cout << "call drive vehicle" << endl;
}

template <typename _Screw>
void ddrive(const screw<_Screw> v)
{
    const _Screw& v1 = static_cast<const _Screw&> (v);
    cout << "call drive screw" << endl;
}

struct VV : public vehicle<VV>
{};

struct DD : public screw<DD>
{};

namespace paint
{
    template <typename _Canvas, typename _Color>
    void clear(const _Canvas& canvas, const _Color& color)
    {
        cout << "clear #2" << endl;
    }

    struct some_canvas 
    {};

    struct blank
    {};

    std::list<some_canvas> canvases(10);

    /*
        @Note 
        - this one call #1
    */
    

    //drive(v);
    //drive(d);
}


static int _main(int argc, char** argv)
{
    assert(Int(4) < Int(6));
    assert(Int(9) > Int(6));

    /*
        @Note 
        - ADT (Argument Dependent Lookup) also know koenig lookup
        - func abs find in crtp namespace
    */
    cout << endl;
    const Float minus_pi = -3.14159265;
    const Float Pi = abs(minus_pi);

    /*
        @Note 
        - solve ambiguous using CRTP
    */
    int x = utility::merge(paint::canvases.begin(), paint::canvases.end());

    cout << endl;
    VV v;
    DD d;
    ddrive(v);
    ddrive(d);

    return 0;
}