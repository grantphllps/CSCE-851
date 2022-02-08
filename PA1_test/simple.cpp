#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <vector>

int main() {
    pid_t pid;

    //TEST 1:
    //char* argumentList[] = {"ls",NULL};

    //TEST 2:
    // char ** argumentList = new char*[2];
    // argumentList[0] = new char[2];
    // argumentList[0] = "ls";
    // argumentList[1] = NULL;

    //TEST 3:
    std::vector<std::string> argumentCopy;
    argumentCopy.push_back("echo");
    argumentCopy.push_back("get");
    argumentCopy.push_back("fukt");

    int n = argumentCopy.size();

    std::cout << "There are " << n << "elements in the vector\n";

    char ** argumentList = new char*[n+1];
    for(size_t i = 0; i < argumentCopy.size(); i++){
        argumentList[i] = new char[argumentCopy[i].size() + 1];
        strcpy(argumentList[i], argumentCopy[i].c_str());
    }

    argumentList[n] = NULL;

    for (int i = 0; i < argumentCopy.size(); i++) {
        std::cout << "|" << argumentList[i] << "|" << std::endl;
    }

    //fork a child process
    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Fork Failed");
        return 1;
    }
    else if (pid == 0) {
        printf("I am the child %d\n",pid);
        execvp(argumentList[0], argumentList);
    }
    else {
        printf("I am the parent %d\n",pid);
        wait(NULL);
        printf("Child Complete\n");
    }
} 