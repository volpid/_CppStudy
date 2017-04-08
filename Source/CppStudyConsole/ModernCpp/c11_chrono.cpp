
#define _CRT_SECURE_NO_WARNINGS
#include <chrono>
#include <ctime>
#include <iostream>

#include <Windows.h>

long Fibonachi(int n)
{
    if (n < 3) 
    {
        return 1;
    }

    return Fibonachi(n - 1) + Fibonachi(n - 2);
}

static int _main(int argc, char** argv)
{
    /*
        @Note
        - struct system_clock
            // wraps GetSystemTimePreciseAsFileTime/GetSystemTimeAsFileTime

        - struct steady_clock
	        // wraps QueryPerformanceCounter
            
        - high_resolution_clock
            // == steady_clock
            typedef steady_clock high_resolution_clock;
    */
    std::chrono::time_point<std::chrono::system_clock> ch_start;
    std::chrono::time_point<std::chrono::system_clock> ch_end;
    
    ch_start = std::chrono::system_clock::now();
    std::cout << "Fibo(42) : " << Fibonachi(30) << std::endl;
    ch_end = std::chrono::system_clock::now();

    std::chrono::duration<double> ch_elapsed_seconds = ch_end - ch_start;
    std::time_t ch_end_time = std::chrono::system_clock::to_time_t(ch_end);

    std::cout << "fibo compute at " << std::ctime(&ch_end_time);
    std::cout << "elapsed time : " << ch_elapsed_seconds.count() << std::endl;
    std::cout << std::endl;

    /*
        @Note
        - use perfomance counter (old way)
    */
    LARGE_INTEGER pc_start;
    LARGE_INTEGER pc_end;
    LARGE_INTEGER pc_freq;

    QueryPerformanceFrequency(&pc_freq);

    QueryPerformanceCounter(&pc_start);
    std::cout << "Fibo(42) : " << Fibonachi(30) << std::endl;
    QueryPerformanceCounter(&pc_end);

    std::cout << "fibo elapsed : ";
    std::cout << static_cast<double> (pc_end.QuadPart - pc_start.QuadPart) / static_cast<double> (pc_freq.QuadPart);

    return 0;
}