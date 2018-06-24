
#include "vulkan_tool.h"

#include <fstream>
#include <iostream>

bool GetBinaryFileContent(const std::string& filename, std::vector<unsigned char>& content)
{
    content.clear();

    std::ifstream file(filename, std::ios::binary);
    if (file.fail() == true)
    {
        std::cout << "Could not open " << filename << " file." << std::endl;
        return false;
    }

    std::streampos begin;
    std::streampos end;
    begin = file.tellg();
    file.seekg(0, std::ios::end);
    end = file.tellg();

    if ((end - begin) == 0)
    {
        std::cout << "The " << filename << " file is empty" << std::endl;
        return false;
    }
    content.resize(static_cast<size_t> (end- begin));
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char *> (content.data()), end - begin);
    file.close();

    return true;
}