
#include <chrono>  
#include <functional>
#include <iostream>
#include <vector>

int Half(int x)
{
    return x /2;
}

struct Half_t
{
    int operator()(int x)
    {
        return x / 2;
    }
};

struct MyHalf
{
    int value;
    int Half(int x)
    {
        return x / 2;
    }

    int HalfValue(void)
    {
        return value / 2;
    }
};

//--------------------------------------------------------------------------------
/*perfomance check*/
class Base 
{
public:
    Base(void) {}
    virtual ~Base(void) {}
    virtual int func(int i) = 0;
};

class Derived : public Base
{
public:
    Derived(int base) {base_ = base;}
    virtual ~Derived(void) {}
    virtual int func(int i)
    {
        return i * base_;
    }
private:
    int base_;
};

struct Functor
{
    int base_;

    Functor(int base)
    {
        base_ = base;
    }

    int operator() (int i)
    {
        return i * base_;
    }
};

const int base = 10;
int BareFunction(int i)
{
    return base * i;
}

static int _main(int argc, char** argv)
{
    MyHalf obj;
    obj.value = 10;
    
    std::function<int(int)> fn1 = Half;
    std::function<int(int)> fn2 = &Half;
    std::function<int(int)> fn3 = Half_t();
    std::function<int(int)> fn4 = [](int x) {return x / 2;};
    std::function<int(int)> fn5 = std::bind(&MyHalf::Half, &obj, std::placeholders::_1);
    std::cout << fn1(10) << std::endl;
    std::cout << fn2(10) << std::endl;
    std::cout << fn3(10) << std::endl;
    std::cout << fn4(10) << std::endl;
    std::cout << fn5(10) << std::endl;

    std::function<int(MyHalf&)> fn6 = &MyHalf::HalfValue;
    std::cout << fn6(obj) << std::endl;

    std::cout << std::endl;

    //--------------------------------------------------------------------------------

    const int tryCount = 10000;
    int total = 0;

    /*virtual member function call*/
    Base* p = new Derived(10);
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < tryCount; ++i)
    {
        total += p->func(i);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "virtual member function call " << total << " ";
    std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << std::endl << std::endl;

    /*bare function call*/
    total = 0;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < tryCount; ++i)
    {
        total += BareFunction(i);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "bare function call " << total << " ";
    std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << std::endl << std::endl;

    /*functor call*/    
    Functor functor(10);
    total = 0;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < tryCount; ++i)
    {
        total += functor(i);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "functor call " << total << " ";
    std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << std::endl << std::endl;

    /*lambda call*/
    int base = 10;
    auto lambda = [&base](int i) 
    {
        return i * base;
    };

    total = 0;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < tryCount; ++i)
    {
        total += lambda(i);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "lambda call " << total << " ";
    std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << std::endl << std::endl;

    /*std::function call*/
    std::function<int (int)> func = &BareFunction;
    total = 0;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < tryCount; ++i)
    {
        total += func(i);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "std::function call " << total << " ";
    std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << std::endl << std::endl;
    
    return 0;
}