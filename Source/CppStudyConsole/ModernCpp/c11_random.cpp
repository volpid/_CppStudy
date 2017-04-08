
#include <functional>
#include <iostream>
#include <random>
#include <vector>

static int _main(int argc, char** argv)
{   
    /*  
        @Note
        - defaut rng is mersenne twister (mt19937)
            typedef mt19937 default_random_engine;
    */

    std::vector<int> randomNumbers;

    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(1, 100);

    for (int cnt = 0; cnt < 10; ++cnt)
    {
        int dice_roll = distribution(generator);  
        randomNumbers.push_back(dice_roll);
    }

    std::cout << "test_1" << std::endl;
    for (int number : randomNumbers)
    {
        std::cout << number << " ";
    }
    std::cout << std::endl;
    randomNumbers.clear();

    
    auto dice = std::bind(distribution, generator);    
    for (int cnt = 0; cnt < 10; ++cnt)
    {
        int dice_roll = dice();  
        randomNumbers.push_back(dice_roll);
    }

    std::cout << "test_2" << std::endl;
    for (int number : randomNumbers)
    {
        std::cout << number << " ";
    }
    std::cout << std::endl;
    randomNumbers.clear();

    return 0;
}

