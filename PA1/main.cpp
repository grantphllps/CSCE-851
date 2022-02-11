/*
 * Copyright (c) 2022, Justin Bradley
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>

#include "command.hpp"
#include "parser.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#define MAX_ALLOWED_LINES 25
//ostream_mode file_comp = ostream_mode::file;
int main(int argc, char** argv)
{
    std::string input_line;

    for (int i=0;i<MAX_ALLOWED_LINES;i++) { // Limits the shell to MAX_ALLOWED_LINES
        // Print the prompt.
        if (argc > 1 && strcmp(argv[1],"-t") == 0){
            // std::cout << "im not doing osh" << std::endl;
        }
        else {
        std::cout << "osh> " << std::flush;
        }

        // Read a single line.
        if (!std::getline(std::cin, input_line) || input_line == "exit") {
            break;
        }

        try {
            // Parse the input line, store the shell commands the vector
            std::vector<shell_command> shell_commands = parse_command_string(input_line);
            int wstatus = 0;
            bool alwaysExecuteNext = true; //this is an override flag to indicate that the next command should always execute
            //After the shell commands are parsed, execute each of the commands in the vector:
            for (const auto& cmd : shell_commands) {

                if (wstatus == 0 || alwaysExecuteNext) { //If the exit status is 0, execute the next command in shell commands
                    pid_t pid;

                    /* Create a copy of the arguments vector and modify it 
                    to include the command name for the execvp function */
                    std::vector <std::string> argumentCopy = cmd.args; 
                    argumentCopy.insert(argumentCopy.begin(),cmd.cmd.c_str());

                    /* Convert the string of vectors to an arrary of Character arrays for execvp */
                    int n = argumentCopy.size();
                    char ** argumentList = new char*[n+1];
                    for(size_t i = 0; i < argumentCopy.size(); i++) {
                        argumentList[i] = new char[argumentCopy[i].size() + 1];
                        strcpy(argumentList[i], argumentCopy[i].c_str());
                    }
                    argumentList[n] = NULL; //argument list must be null terminated
                    
                    //fork the child process
                    pid = fork();
                    
                    if (pid < 0) {
                        // fprintf(stderr, "Fork Failed");
                        return 1;
                    }
                    else if (pid == 0) {
                        /* After the fork, need to check if the output or input needs redirected */
                        int fd;	// file descriptor
                        if (cmd.cout_mode == ostream_mode::file) { /* redirect output to a file */
                            if ((fd = open(cmd.cout_file.c_str(), O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) {
				                perror(cmd.cin_file.c_str());	/* open failed */
				                exit(1);
		                    }
                            dup2(fd,STDOUT_FILENO);
                        }
                        else if (cmd.cout_mode == ostream_mode::append) {
                            if ((fd = open(cmd.cout_file.c_str(), O_CREAT|O_APPEND|O_WRONLY, 0644)) < 0) {
				                perror(cmd.cin_file.c_str());	/* open failed */
				                exit(1);
		                    }
                            dup2(fd,STDOUT_FILENO);
                        }
                        if (cmd.cin_mode == istream_mode::file) {
                            //std::cout << "File input" << std::endl;
                            if ((fd = open(cmd.cin_file.c_str(), O_RDONLY, 0644)) < 0) {
				                perror(cmd.cin_file.c_str());	/* open failed */
				                exit(1);
		                }
                            dup2(fd,STDIN_FILENO);
                        }
                        execvp(argumentList[0], argumentList); //execvp the command with parameters
                    }
                    else {
                        // printf("I am the parent %d\n",pid);
                        wait(&wstatus);

                        if (cmd.next_mode == next_command_mode::always) {
                            wstatus = 0;
                            // std::cout << "I am going to execute the next command because of ';'" << std::endl;
                        }
                        else if (cmd.next_mode == next_command_mode::on_success && wstatus == 0) {
                            wstatus = 0;
                            // std::cout << "I am going to execute the next command beause the previous command was successful" << std::endl;
                        }
                        else if (cmd.next_mode == next_command_mode::on_fail && wstatus != 0) {
                            wstatus = 0;
                            // std::cout << "I am going to execute the next command because the the previous command failed" << std::endl;
                        }
                        else {
                            wstatus = 1;
                            // std::cout << "I will not execute the next command" << std::endl; 
                        }
                    }
                }
                if (cmd.next_mode == next_command_mode::always) {
                        alwaysExecuteNext = true;
                    }
                else {
                        alwaysExecuteNext = false;
                    }       
            }

            //Print the list of commands.
            // std::cout << "-------------------------\n";
            // for (const auto& cmd : shell_commands) {
            //     std::cout << cmd;
            //     std::cout << "-------------------------\n";
            // }
        }
        catch (const std::runtime_error& e) {
            std::cout << "osh: " << e.what() << "\n";
        }
    }

    // std::cout << std::endl;
}