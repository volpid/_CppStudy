
#include <iostream>
#include <vector>

struct NoDefault
{
    NoDefault(void) = delete;
};

struct NoCopy
{
    NoCopy(void) = default;
    NoCopy(const NoCopy&) = delete;
};

template <typename _T>
void Swap(_T& a, _T& b)
{
    static_assert(std::is_copy_constructible<_T>::value, "swap require copying");
    static_assert(noexcept(std::is_nothrow_move_constructible<_T>::value), "swap may throw");

    auto c = b;
    b = a;
    a = c;
}

template <typename _T>
struct DataStructure
{
    static_assert(std::is_default_constructible<_T>::value, "DataStructure require default constructure");
};

static int _main(int argc, char** argv)
{
    int a, b;
    Swap(a, b);

    NoCopy nc_a, nc_b;
    //#compile time error : swap require copying
    //Swap(nc_a, nc_b);

    DataStructure<int> ds_ok;
    //#compile time error : DataStructure require default constructure
    //DataStructure<NoDefault> ds_fail;

    return 0;
}