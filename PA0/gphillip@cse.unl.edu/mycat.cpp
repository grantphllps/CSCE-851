#include <iostream>
#include <fstream>

int main(int argc, char** argv)
{
    std::string filename(argv[1]);
    std::ifstream input_file(filename);
    std::string currentLine;

    while (getline(input_file,currentLine))
        std::cout << currentLine << "\n";
        
    return 0;
}