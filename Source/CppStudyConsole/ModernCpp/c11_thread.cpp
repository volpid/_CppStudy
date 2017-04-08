
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

std::mutex mtx;

void foo(void)
{    
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "void foo(void)" << std::endl;
}

void bar(int x)
{
    mtx.lock();
    std::cout << "void bar(int x)" << x << std::endl;
    mtx.unlock();
}

static int _main(int argc, char** argv)
{
    std::vector<std::thread> threads;

    threads.emplace_back(std::thread(foo));
    threads.emplace_back(std::thread(bar, 10));

    std::cout << "main!" << std::endl;

    for (auto& t : threads)
    {
        t.join();
    }

    std::cout << "main! end!" << std::endl;
    
    return 0;
}