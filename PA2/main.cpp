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
#define READ_END 0
#define WRITE_END 1


int main(int argc, char** argv)
{
    std::string input_line;
    bool tFlag = false;

    for (int i=0;i<MAX_ALLOWED_LINES;i++) { // Limits the shell to MAX_ALLOWED_LINES
        // Only print the prompt if the -t option is given.
        if (argc > 1 && strcmp(argv[1],"-t") == 0){
            tFlag = true;
        }
        
        if (!(tFlag)) {
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
            bool alwaysExecuteNext = true;
            //bool piping[2]; // Is there any piping bewteen commands. piping[0] inicates a pipe bewteen current and previous, piping[1] indicates a pipe beween current and next
            int fdp[2];
            int previousPipeReadEnd;
            bool firstPipe = true;
            bool piping;

            // After the shell commands are parsed for a single line, begin to go through the commands in the line
            for (const auto& cmd : shell_commands) {
                bool pipeF = false;
                bool pipeM = false;
                bool pipeL = false;

                if (wstatus == 0 || alwaysExecuteNext) { //If the exit status is 0, execute the next command in shell commands

                    std::vector <std::string> argumentCopy = cmd.args; 
                    argumentCopy.insert(argumentCopy.begin(),cmd.cmd.c_str());

                    //Convert the string of vectors to an arrary of Character arrays for execvp
                    int n = argumentCopy.size();
                    char ** argumentList = new char*[n+1];
                    for(size_t i = 0; i < argumentCopy.size(); i++) {
                        argumentList[i] = new char[argumentCopy[i].size() + 1];
                        strcpy(argumentList[i], argumentCopy[i].c_str());
                    }
                    argumentList[n] = NULL; //argument list must be NULL terminated

                    
                        if (cmd.cout_mode == ostream_mode::pipe && cmd.cin_mode != istream_mode::pipe) {
                            pipeF = true;
                            pipeM = false;
                            pipeL = false;
                            piping = true;
                            firstPipe = true;
                        }
                        else if (cmd.cout_mode != ostream_mode::pipe && cmd.cin_mode == istream_mode::pipe) {
                            pipeF = false;
                            pipeM = false;
                            pipeL = true;
                            piping = true;
                        }
                        else if (cmd.cout_mode == ostream_mode::pipe && cmd.cin_mode == istream_mode::pipe) {
                            pipeF = false;
                            pipeM = true;
                            pipeL = false;
                            piping = true;
                        }
                        else {
                            bool pipeF = false;
                            bool pipeM = false;
                            bool pipeL = false;
                            piping = false;
                        }

                    if (pipeF || pipeM) { //if we are writing to a pipe, create a new one
                        //std::cout << "making the pipe\n";
                        pipe(fdp);
                        if (firstPipe) {
                            previousPipeReadEnd = fdp[READ_END];
                            firstPipe = false;
                        }
                    }
                    
                    //Fork the child process
                    pid_t pid = fork();
                    
                    if (pid < 0) {
                        // fprintf(stderr, "Fork Failed");
                        return 1;
                    }
                    else if (pid == 0) {
                        // Need to take care of the input/output redirecting
                        int fd;
                        // output redirecting
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
                        else if (cmd.cout_mode == ostream_mode::pipe) {
                            //std::cout << "Closing the write end in the child\n";
                            dup2(fdp[WRITE_END],STDOUT_FILENO);
                            //close(fdp[WRITE_END]);
                        }
                        if (piping) {
                            if (pipeF || pipeM)
                                close(fdp[WRITE_END]);
                        }
                        
                        
                        // input redirecting
                        if (cmd.cin_mode == istream_mode::file) {
                            if ((fd = open(cmd.cin_file.c_str(), O_RDONLY, 0644)) < 0) {
				                perror(cmd.cin_file.c_str());	/* open failed */
				                exit(1);
		                    }
                            dup2(fd,STDIN_FILENO);
                        }
                        else if (cmd.cin_mode == istream_mode::pipe) {
                            //std::cout << "closing the read end in the child\n";
                            dup2(previousPipeReadEnd,STDIN_FILENO);
                            //close(previousPipeReadEnd);
                        }
                        if (piping) {
                            close(fdp[READ_END]);
                            if (pipeM)
                                close(previousPipeReadEnd);
                        }
                        
                        //after redirecing is handled, execute the command
                        //std::cout << "Executing " << argumentList[0] << std::endl;
                        execvp(argumentList[0], argumentList);
                        exit(0);
                    }
                    else {
                        if (piping) {
                            if (pipeM)
                                close(previousPipeReadEnd);
                            previousPipeReadEnd = fdp[READ_END];
                            if (pipeF || pipeM)
                                close(fdp[WRITE_END]);
                            if (pipeL)
                                close(fdp[READ_END]);
                        }   
                        // printf("I am the parent %d\n",pid);
                        wait(&wstatus);
                        //close(fdp[WRITE_END]);
                        // Now that the child process has terminated, need to evaluate the exit status and decide whether
                        // or not to indicate whether or not the next command gets executed
                        if (cmd.next_mode == next_command_mode::always) {
                            wstatus = 0;
                        }
                        else if (cmd.next_mode == next_command_mode::on_success && wstatus == 0) {
                            wstatus = 0;
                        }
                        else if (cmd.next_mode == next_command_mode::on_fail && wstatus != 0) {
                            wstatus = 0;
                        }
                        else {
                            wstatus = 1;
                        }
                    }
                }
                //This is to catch if the current command was not executed, but the next one should
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
            if (!tFlag)
                std::cout << "osh: " << e.what() << "\n";
            else
                std::cout << e.what() << "\n";

        }
    }

    // std::cout << std::endl;
}