
#include <functional>
#include <iostream>
#include <random>
#include <stdio.h>
#include <vector>

const int maxStream = 2048;
const int xorStreamSeed = 16383;
char xorStream[maxStream];

bool InitXorStream(void)
{
    std::default_random_engine engine(xorStreamSeed);
    std::uniform_int_distribution<int> distribution(0, 256);
    auto dice = std::bind(distribution, engine);

    for (int idx = 0; idx < maxStream; ++idx)
    {
        xorStream[idx] = dice();
    }

    return true;
}

char GetXORValue(unsigned int index)
{
    return xorStream[index % maxStream];
}

void PrintText(const char* text, int size)
{
    for (int idx = 0; idx < size; ++idx)
    {
        printf("%c", text[idx]);
    }
}

static int _main(int argc, char** argv)
{
    InitXorStream();

    const char* originalText = 
    R"==(//  (C) Copyright Dave Abrahams, Steve Cleary, Beman Dawes, Howard
//  Hinnant & John Maddock 2000-2003.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.)==";

    int size = strlen(originalText);
    
    char* encryptedText = new char[size];
    char* decryptedText = new char[size];

    for (int idx = 0; idx < size; ++idx)
    {
        encryptedText[idx] = originalText[idx] ^ GetXORValue(idx);
    }

    for (int idx = 0; idx < size; ++idx)
    {
        decryptedText[idx] = encryptedText[idx] ^ GetXORValue(idx);
    }

    std::cout << "original text" << std::endl << std::endl;
    PrintText(originalText, size);
    std::cout << std::endl << std::endl;

    std::cout << "encrypt text" << std::endl << std::endl;
    PrintText(encryptedText, size);
    std::cout << std::endl << std::endl;

    std::cout << "decrypt text" << std::endl << std::endl;
    PrintText(decryptedText, size);
    std::cout << std::endl << std::endl;

    return 0;
}