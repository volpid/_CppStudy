
#include <iostream>
#include <vector>

//begin / end for container
template <typename _Container>
auto begin_(_Container& _Cont) -> decltype(_Cont.begin())
{
    return (_Cont.begin());
}

template <typename _Container>
auto end_(_Container& _Cont) -> decltype(_Cont.end())
{
    return (_Cont.end());
}

// begin / end for array
template <typename _T, size_t _Size>
_T* begin_(_T(&Array)[_Size]) noexcept
{
    return (Array);
}

template <typename _T, size_t _Size>
_T* end_(_T(&Array)[_Size]) noexcept
{
    return (Array + _Size);
}

// display all element
template <typename _T>
void Show(_T& c)
{
    for (auto p = begin_(c); p != end_(c); ++p)
    {
        std::cout << *p << std::endl;
    }
}

// using custom range base
template <typename _T>
struct Point3D
{
    _T x, y, z;
};

template <typename _T>
_T* begin(Point3D<_T>& p) noexcept
{
    return &(p.x);
}

template <typename _T>
_T* end(Point3D<_T>& p) noexcept
{
    return &(p.z) + 1;
}

static int _main(int argc, char** argv)
{
    std::vector<int> v = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    Show(v);
    std::cout << std::endl;

    int x[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    Show(x);
    std::cout << std::endl;

    // custom range base for
    Point3D<int> p1 = { 1, 2, 3 };
    for (auto& n : p1)
    {
        std::cout << n << std::endl;
    }
    std::cout << std::endl;

    return 0;
}