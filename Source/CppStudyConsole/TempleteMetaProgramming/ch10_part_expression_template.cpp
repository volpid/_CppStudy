
#include <iostream>
#include <vector>

using namespace std;
namespace boost {namespace mpl{}}
namespace mpl = boost::mpl;

/*
    @Note

    1. x = a + b + c

    -->
        Array operator+(const Array& a, const Array& b)
        {
            const std::size_t n = a.size();
            Array result;

            for (std::size_t row = 0; row < n; ++row)
            {
                for (std::size_t col = 0; col < n; ++col)
                {
                    result[row][col] = a[row][col] + b[row][col]
                }
            }

            return result;
        }

        - first a + b calc
        - then make temp object = t
        - then t + c calc
        - then assigned to x
*/

struct Array
{
    static const int maxSize = 5;

    Array(void)
    {
        int count = 0;
        for(float& item : arr)
        {
            item = count++;
        }
    }
        
    template <typename _Expr>
    Array& operator=(_Expr x)
    {
        for (size_t idx = 0; idx < maxSize; ++idx)
        {
            (*this)[idx] = x[idx];
        }

        return *this;
    }

    float& operator[](size_t index)
    {
        return arr[index];
    }

    float operator[](size_t index) const
    {
        return arr[index];
    }

    float arr[maxSize];
};

struct iiplus
{
    static float apply(float a, float b)
    {
        return a + b;
    }
};

struct iiminus
{
    static float apply(float a, float b)
    {
        return a - b;
    }
};

template <typename _L, typename _OpTag, typename _R>
struct Expression
{
    Expression(const _L& l, const _R& r)
        : l(l), r(r)
    {}

    /*delay until it used!*/
    float operator[](unsigned int index) const
    {
        return _OpTag::apply(l[index], r[index]);
    }

    const _L& l;
    const _R& r;
};

template <typename _L, typename _R>
Expression<_L, iiplus, _R> operator+(const _L& l, const _R& r)
{
    return Expression<_L, iiplus, _R>(l, r);
}

template <typename _L, typename _R>
Expression<_L, iiminus, _R> operator-(const _L& l, const _R& r)
{
    return Expression<_L, iiminus, _R>(l, r);
}

/*
    @Note
    1. a + b 
        = Expression<a, plus, b>
    2. a + b + c 
        = Expression<Expression<a, plus, b>, plus, c>

    ! operator+ 
        - dont calc anything 
        - just save arg refs

    3. (a + b)[1] 
        = plus::apply(a[1], b[1])
        = a[1] + b[1]

    4. (a + b + c)[1] 
        = plus::apply((a + b)[1], c[1])
        = plus::apply(plus::apply(a[1], b[1]), c[1])
        =  plus::apply(a[1] + b[1], c[1])
        = a[1] + b[1] + c[1]
*/


static int _main(int argc, char** argv)
{
    Array a;
    Array b;
    Array c;
    Array x;

    x = a + b + c;

    for (float item : x.arr)
    {
        cout << item << endl;
    }

    return 0;
}