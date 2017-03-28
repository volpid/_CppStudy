
#include <iostream>

/*
    @Note
        - template version of binary 
*/
template <unsigned int N>
struct BinaryTemplate
{
public:
    static const unsigned int value = BinaryTemplate<N / 10>::value * 2 + N % 10;
};

template <>
struct BinaryTemplate<0>
{
    static const unsigned int value = 0;
};

/*  
    @Note
        - constexpr version of binary
        - it can be used in dynamic
*/

constexpr unsigned int BinaryRecursiveFunc(unsigned int n)
{
    return (n == 0) ? (0) : (n % 10 + 2 * BinaryRecursiveFunc(n / 10));
}

unsigned int BinaryForFunc(unsigned int n)
{
    unsigned int result = 0;
    for (unsigned int bit = 0x1; n; n /= 10, bit <<= 1)
    {
        if (n % 10)
        {
            result += bit;
        }
    }
    return result;
}

static int _main(int argc, char** argv)
{
    //template version
    constexpr unsigned int one = BinaryTemplate<1>::value;
    constexpr unsigned int three = BinaryTemplate<11>::value;
    constexpr unsigned int five = BinaryTemplate<101>::value;
    constexpr unsigned int nine = BinaryTemplate<1001>::value;

    std::cout << one << std::endl;
    std::cout << three << std::endl;
    std::cout << five << std::endl;
    std::cout << nine << std::endl;

    constexpr unsigned int one1 = BinaryRecursiveFunc(1);
    constexpr unsigned int three3 = BinaryRecursiveFunc(11);
    std::cout << one1 << std::endl;
    std::cout << three3 << std::endl;

    int input;
    std::cin >> input;
    // compiletime error
    //constexpr unsigned inputValue = BinaryRecursiveFunc(input);    
    //constexpr unsigned inputValue2 = BinaryForFunc(input);
    const unsigned inputValue = BinaryRecursiveFunc(input);    
    const unsigned inputValue2 = BinaryForFunc(input);
    
    std::cout << inputValue << std::endl;
    std::cout << inputValue2 << std::endl;

    return 0;
}