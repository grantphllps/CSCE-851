#include <iostream>
#include <vector>
#include <string>

int main() {
    std::vector <std::string> ingredient(4);

    ingredient[0] = "eggs";
    ingredient[1] = "milk";
    ingredient[2] = "cheese";
    ingredient[3] = "butter";

    for (std::string ing: ingredient)
        std::cout << ing << std::endl;
    std::cout << "----------" << std::endl;
    std::cout << "Adding flour to the front" << std::endl;
    std::cout << "----------" << std::endl;

    auto it = ingredient.insert(ingredient.begin(),"flour");
    //auto it = vec.insert(vec.begin(), 3);
    
    for (std::string ing: ingredient)
        std::cout << ing << std::endl;

    it = ingredient.insert(ingredient.end(),"NULL");

    std::cout << "----------" << std::endl;
    std::cout << "Adding fun to the end" << std::endl;
    std::cout << "----------" << std::endl;

    for (std::string ing: ingredient)
        std::cout << ing << std::endl;

    const int numel = ingredient.size();

    std::cout << "The vector has " << numel << " elements" << "\n";
    char** argumentList[numel];

    char ** arr = new char*[ingredient.size()];
    for(size_t i = 0; i < ingredient.size(); i++){
    arr[i] = new char[ingredient[i].size() + 1];
    strcpy(arr[i], ingredient[i].c_str());
}

    std::cout << "----------" << std::endl;
    std::cout << "Here is the full list as a char array" << std::endl;
    std::cout << "----------" << std::endl;

    for (int i = 0; i < ingredient.size(); i++) {
        std::cout << arr[i] << std::endl;
    }

}

// #include <iostream>
// #include <string>
// #include <vector>

// int main() {
//     // Vector with 5 integers
//     // Default value of intergers will be 0.
//     std::vector <int> vecOfInts(5);

//     for (int x: vecOfInts)
//         std::cout << x << std::endl;
