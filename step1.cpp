#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <vector>
#include <map>
#include <string>
//Step 1
//Use std::getline to read a line of input from stdin (name of command)
//if WIFSIGNALED(status) --> print WTERMSIG(status)
//Prints shell beginning and reads line from standard input

//Reads one line of stdin to run as a command
std::string ReadCommand() {
  std::cout<<"myShell $ ";
  std::string input;
  std::cin >> input;
  return  input; //system calls only operate on char *'s
}

//runs the command read from stdin. Prints what signal killed the command or the status the command exited with
void runCommand(const char * line) {
  int ret = system(line);
  std::cout<<"ret value is :"<<ret<<std::endl;
  //check if signal is terminated by some signal
  if (WIFSIGNALED(ret)) {
    std::cout<<"Program was killed by signal "<<WTERMSIG(ret)<<std::endl;
  }
  //check if signal just exited normally
  else if (WIFEXITED(ret)) {
    std::cout<<"Program exited with status "<<WEXITSTATUS(ret)<<std::endl;
  }
}
std::vector<char *> * printenvironment(char** myenvironment) {
  std::vector<char *> *  environ_list = new std::vector<char *>;
  char * s = *myenvironment;
  for (int i = 1;s; i++) {
    //    printf("%s\n", s);
    environ_list->push_back(s);
    s = *(myenvironment+i);
  }
  for (std::vector<char *>::iterator it = environ_list->begin(); it != environ_list->end(); ++it) {
    std::cout<<"Environment is:"<<(*it)<<std::endl;
  }
  return environ_list;
}
//Runs the Bash program until exit or EOF encountered
void loopCommand() { 
  std::string input = ReadCommand();
  /*   while ((input.compare("exit")!=0)&&(!input.empty())) {
    runCommand(input.c_str());
    input = ReadCommand();
     }
  */
}

std::string redirect(std::string input, int file_descriptor) {
  std::string result;
  std::cout<<"Input was:"<<input<<std::endl;
  size_t ret;
  if(( ret = input.find(">")) != std::string::npos) {
    file_descriptor = 1;
    // std::cout<<"Found a >"<<std::endl;
    //std::cout<<"Find returns:"<<input.find(">")<<std::endl;
    result = input.substr(input.find(">")+1);
  }
  else if ((ret = input.find("<")) !=std::string::npos) {
    std::cout<<"Inside in redirect"<<std::endl;
    result = input.substr(input.find("<")+1);
  }
    std::cout<<"First IS:"<<result<<std::endl;
  return result;
}


int main(void) {
  //  extern char **environ;
  //build environment map
  // std::map<std::string, std::string> * environ_map = environment_map(environ);   
  //  for (std::map<std::string, std::string>::iterator it = environ_map->begin(); it != environ_map->end(); ++it) {
  // std::cout<<"Key is:"<<it->first<<" Value is:"<<it->second<<std::endl;
  // }
  //Step 1
  //loopCommand();
  //std::string input = ReadCommand();
  //runCommand(input.c_str());
  std::cout<<"Hello World!"<<std::endl;
  //  getthirdarg("set $PATH EVIL");
  //printenvironment(environ);
  int file = -1;
 std::string out = redirect("./myProgram 2 3 4 > out.txt", file);
 std::string out2 = redirect("./myprog < in.txt", file);
  return EXIT_SUCCESS;
}
