#include <iostream>

int main(int argc, char** argv)
{
    int i;
    for (i = 1;i < argc - 1; ++i)
        std::cout << argv[i] << " ";

    std::cout << argv[i] << "\n";

    return 0;
}