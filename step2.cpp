#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string>
#include <vector>
#include <fstream>
#include <dirent.h>
#include <wordexp.h>
#include <sstream>
#include <errno.h>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

//Reads one line of stdin to run as a command
std::string  ReadCommand() {
  int size = PATH_MAX +1;
  char * mypath =(char *)  malloc((PATH_MAX+1)*sizeof(*mypath));;
  mypath = getcwd(mypath, (size_t) size);
  std::cout<<"myShell:";
  std::cout<<mypath;
  std::cout<<" $ ";
  std::string input;
  getline(std::cin, input);
  free(mypath);
  return input; //system calls only operate on char *'s
}

//gets the directories in the PATH environment variable to search through for programs
void GetPath(std::vector<std::string> * paths) {
  //std::vector<std::string>  pathvariables; //= new std::vector<std::string>;
  char * directories = getenv("PATH");
  char * segment;
  while ((segment = strsep(&directories, ":")) !=NULL) {
    std::string mysegment(segment);
    paths->push_back(mysegment);
  }
}

//searches for correct program in directories found from PATH environment variable
std::string searchPathdirectories(std::string  mycommand, std::vector<std::string> * Directories ) {
  std::string slash = "/";
  const char * cmd = mycommand.c_str();
  std::string input = mycommand;
  DIR * dirp = NULL;
    for (std::vector<std::string>::const_iterator i = Directories->begin(); i!=Directories->end(); ++i) {
      //std::cout<<"My directory is:"<<*i<<std::endl;
      if ((*i).c_str() != NULL) {
	dirp = opendir((*i).c_str()); //get a directory stream corresponding to input
      }
      if (dirp == NULL) {
	//	perror("The following error occurred");
	//exit(EXIT_FAILURE);
	return input;
      }
      struct dirent * dp; //direct structure
      while ((dp = readdir(dirp))!= NULL) {
	std::string mypath = (*i);
	mypath.append(slash);
	std::string filename(dp->d_name);
	mypath.append(filename);
	// std::cout<<"My path is:"<<mypath<<std::endl;
	if ((strcmp(dp->d_name, cmd) == 0)) {
	  //std::cout<<"File is :"<<mypath<<std::endl;
	  input.assign(mypath);
	  break;
	}
	mypath.clear();
      }
      closedir(dirp);
    }
  return input;
}


//makes a vector of strings composed of the command and the arguments inputted
//A vector is built first and then translated to a char ** as a vector
//gives additional sizing information for each typed argument that makes
//conversions to char * easier
std::vector<std::string> * arglistbuilder(std::string Term_input) {
  std::vector<std::string> * myargs = new std::vector<std::string>;
  bool space = false;
  std::stringstream mystream;
  mystream.str(Term_input);//stringstream of arguments
  int c;
  std::string current;
  while ((c = ((mystream.get())))!=EOF) {
    //checks if we have whitespace after escape sequence
    if (space == true) {
      space = false;
      continue;
    }
    //check if c is not a space and not a backslash
    const char d = (c - '\0');
    if ((c != '\\')&&(!isspace(c))){
      current+= d;
    }
    //if the character is a slash check next character
    else if (c == '\\') {
      int e = mystream.peek();
      //if the next character is a space
      if ((isspace(e))) {
	char g = (e - '\0');
	//set bool to true so that we don't append the backslash to our string
	space= true;
	current += g;
      }
    }
    //if the character is a space and we have chars in our current string, add it to the ver
    else if ((isspace(c))&&(!(current.empty()))) {
      myargs->push_back(current);
      current.clear();
    }
  }//add last argument to the vector
  if (!(current.empty())) {
    myargs->push_back(current);
  }
  return myargs;
}

std::string tildaexpansion(const char * name) {
    std::string replacement;
    wordexp_t p;
    wordexp(name, &p, 0);
    replacement.assign((p.we_wordv)[0]);
    wordfree(&p);
    return replacement;
}

//converts the vector of commands to a char* array that can be passed into execve
//also checks if command has a backslash or a tilda
//will expand the command name to get the correct filename to execute on
char ** const argarray(std::vector<std::string> * argument_vector, std::vector<std::string> *  Files) {
  int size = argument_vector->size();
  //use vector sizing to figure out correct amount of space needed for char **
  char ** const newarg = (char ** const)  malloc((argument_vector->size()+1)*sizeof(*newarg));
  std::string replacement;
  //if there is no / in the command, check if the command can be found in directories from PATH variable
  if (argument_vector->at(0).find('/') == std::string::npos) {
    replacement = searchPathdirectories(argument_vector->at(0), Files);
    if (!replacement.empty()) {
    argument_vector->at(0) = replacement;
    }
  }
  else {
    char actualpath [PATH_MAX+1];
    //if the command has a tilda, expand the filename so that we can execute on the correct filename
     if (argument_vector->at(0).find('~') != std::string::npos) {
       replacement = tildaexpansion(argument_vector->at(0).c_str());
     }
     else {
       //expands the inputted name from a relative directory to its absolute path
       char *  result = realpath(argument_vector->at(0).c_str(), actualpath);
       if (result != NULL) {
	 replacement.assign(result);
       }
     }
     //changes the stored relative command pathname to its absolute one if it exists 
     if (!(replacement.empty())) {
       argument_vector->at(0) = replacement;
     }
  }
  for (int i = 0; i < size; i++) {
    newarg[i] = (char *) malloc((argument_vector->at(i).length()+1)*sizeof(*(newarg[i])));
    strncpy(newarg[i], argument_vector->at(i).c_str(), (argument_vector->at(i).length()+1));
  }
  newarg[size] = NULL; //last argument is NULL
  return newarg;
}

//gets rid of data structures allocated on the heap: the char** that will be passed to execve
//and the vector that was allocated on the stack and the map that contains key and value pairs
//for the environment variables
void cleanup(int arraysize, char ** const arg_array) {
  for (int i=0; i<arraysize; i++) {
      free(arg_array[i]);
    }
  free(arg_array);
  // free(command);
  //delete arg_vector;
}


//handles the cd command inputted to the Terminal
bool changedir(std::vector<std::string> * argument_list) {
  std::string directory;
  bool cd = false;
  int error = 0;
  //checks if a user inputted "cd" into the Terminal
  if ((strcmp("cd", argument_list->at(0).c_str()))==0) {
      cd = true;
      //checks if cd was inputted along with arguments
      if (argument_list->size() >= 2) {
	directory.assign(argument_list->at(1));
	//checks if the argument has a tilda and will expand
	//argument to an absolute path
	if (directory.find('~') != std::string::npos) {
	  directory = tildaexpansion(directory.c_str());
	}
	//checks if changedir failed
	error = chdir(directory.c_str());
      }
      else {
	error = chdir(getenv("HOME"));
      }
    }
    if (error == -1) {
      //prints an error for changedir 
      perror("changedir");
    }
    return cd;
}

//builds a Map of keys and values for environment variables
std::map<std::string, std::string> *  environment_map(char** myenvironment) {
  std::map<std::string, std::string> *  mymap = new std::map<std::string, std::string>;
   char * s = *myenvironment;
  for (int i = 1;s; i++) {
    std::string temporary(s);
    size_t location = temporary.find("=");
    std::string key = temporary.substr(0, location);
    std::string value = temporary.substr(location+1);
    mymap->insert(std::pair<std::string, std::string>(key, value));
    s = *(myenvironment+i);
  }
  return mymap;
}

//function for set commands
bool set_variable(std::string input, std::vector<std::string> * command_list,
		  std::map<std::string, std::string> *  mymap) {
  bool set_command = false;
  std::string first_arg = command_list->at(0);

  //checks if the 1st word of the command typed in is "set" and there are at least 2 arguments
  if ((first_arg.compare("set") == 0) ) {
    set_command = true;
    if (command_list->size() >= 3 ) {
      std::string second_arg = command_list->at(1);
      //uses the map of key and values of the environment variable to find the value
      //for the variable typed in

      std::map<std::string, std::string>::iterator location = mymap->find(second_arg);
      //if the variable typed in matches a variable in the map
      if (location != mymap->end()) {
	int second_arg_size = command_list->at(1).length();
	//it gets the 2nd argument typed in the Terminal and replaces the current value for the key found
	//with the value typed in as the 2nd argument to the Terminal command
	size_t locale = input.find(command_list->at(1));
	std::string new_value = input.substr(locale+second_arg_size+1);
	mymap->erase(mymap->find(second_arg));
	mymap->insert(std::pair<std::string, std::string> (second_arg, new_value));
      }
    }
  }
  return set_command;
}

//will print the variable name when $var is typed is
bool variable_name(char * cmd, std::map<std::string, std::string> *  environment_map) {
  bool variable = false;
  std::string command(cmd);
  if (command.find('$') == 0) {
    variable = true;
    command.erase(command.begin()); //erase first character to have just the variable
    //search for the variable in our MAP
    //prints it out if it finds it
    std::map<std::string, std::string>::iterator location = environment_map->find(command);
    if (location != environment_map->end()) {
      std::cout<<(environment_map->find(command)->second)<<std::endl;
    }
  }
  return variable;
}
//exports the variable typed in if:
//the variable exists, and a variable is typed along with "export"
bool exportvariable(std::map<std::string, std::string> * variables, std::vector<std::string> * myargs) {
  bool export_var = false;
  std::string command(myargs->at(0));
  if ((command.compare("export") == 0) && myargs->size() >= 2) {
    export_var = true;
    std::string var = myargs->at(1);
    std::map<std::string, std::string>::iterator locale = variables->find(var);
    if (locale != variables->end()) {
      std::string value = locale->second;
      int err = setenv(var.c_str(), value.c_str(),  1);
      //variables = environment_map(environ);
      if (err == -1) {
	perror("setenv");
	exit(EXIT_FAILURE);
      }
    }
  }
  return export_var;
}


//this runs three built in commands of cd, $var,  and set var value
//this checks if any actually ran by examining the booleans returned by each function
//if any ran, this will return a true boolean
bool builtcommand(std::map<std::string, std::string> * mymap, std::vector<std::string> * myargs, std::string input, char * cmd) {
  bool universal = false;
  bool  var = variable_name(cmd, mymap);
  bool set = set_variable(input, myargs, mymap);
  // std::cout<<"SET is:"<<set<<std::endl;
  // bool  export_cmd = exportvariable(mymap, myargs);
  bool cd = changedir(myargs);
  if ((var == true)||(set == true)||(cd == true)){
    universal = true;
  }
  //  std::cout<<"universal is:"<<universal<<std::endl;
  return universal;
}

std::pair<std::string, std::string> redirect(std::string input, int *  file_descriptor) {
  std::pair<std::string, std::string> result;
  result.first = input;
  size_t ret;
  if(( ret = input.find(">")) != std::string::npos) {
    *(file_descriptor) = 1;
    result.first = input.substr(0,input.find(">"));
    result.second= input.substr(input.find(">")+1);
  }
  else if ((ret = input.find("<")) !=std::string::npos) {
    result.first = input.substr(0,input.find("<"));
    result.second = input.substr(input.find("<")+1);
  }
  return result;
}


//forks, executes a child process and has parent process wait until child process ends
//takes in other data structures created before as inputs
void runCommand(std::pair<std::string, std::string> redirect_pair, int * file_descriptor, char ** const argv,
		char * original_argument,  char ** environment) {
  int description;
  /*
  if (file_descriptor == 0) {
    
    
  }
  */
  /*
  if((*file_descriptor) == 1) {
    std::cout<<"Second arg is:"<<redirect_pair.second<<std::endl;
    description = open(redirect_pair.second.c_str(), O_WRONLY| O_CREAT| S_IRUSR | S_IWUSR);
    if (description == -1) {
      perror("Redirect");
      exit(EXIT_FAILURE);
    }    
  }
  */
  // else if (file_descriptor == 2) {
    
  // }
  //  pid_t group_id = getpgid(0);
  // uid_t user_d = getuid();
  pid_t process = fork();
  int status; //gives info on the child process
  //if fork fails
  if (process == -1) {
    perror("Error with Fork");
    exit(EXIT_FAILURE);
  }
  //Child process
  else if (process == 0) {
    //if we didn't find the command in the PATH directories AND the original typed command didn't have a slash
    if ((*file_descriptor) == 1) {

      
      description = open(redirect_pair.second.c_str(), O_CREAT|O_TRUNC|O_RDWR, 0664);       // mode_t mode = 0644;

      if (description == -1) {
	perror("Redirect");
	exit(EXIT_FAILURE);
      }
      std::cout<<"In child file_descriptor"<<std::cout;
      dup2(description, 1);
      close(description);
    }
    //we should output "Command not found" and exit
    if((strcmp(argv[0], original_argument)==0)) {
      if ((strchr(original_argument, '/')==NULL)) {
	std::cout<<"Command "<<argv[0]<<" not found"<<std::endl;
	exit(EXIT_FAILURE);
      }
    }
    //if the command was found in directories searched through the PATH variable, we should execute the command
    //with execve. We have passed in the current environment variables as well as the char** that hols the command
    //and all of its arguments
    if ((*file_descriptor) == 1) {
      close(description);
    } 
    int error = execve(argv[0], argv, environment);
    if (error == -1) {
      perror("execute");
      exit(EXIT_FAILURE);
    }
    //  if ((*file_descriptor) == 1) {
    //  fchown(description, user_d,  group_id);
    //}
  }
  //it is the parent process
  else{
    //wait until the child process executes, is suspended, or terminates to proceed
    pid_t child = waitpid(process, &status, WUNTRACED);
    if (child == -1) {
      perror("waitpid Error");
      exit(EXIT_FAILURE);
    }
    else if (WIFSIGNALED(status)) {
	std::cout<<"Program was killed by signal "<<WTERMSIG(status)<<std::endl;
    }
  //check if signal just exited normally
    else if (WIFEXITED(status)) {
	std::cout<<"Program exited with status "<<WEXITSTATUS(status)<<std::endl;
    }
  }
}
//if there is a pipe
  
int main(void) {
  //environment variables char ** used to build the map of key and values for the environment variables
  extern char **environ;

   //build environment variable map

  std::map<std::string, std::string> *  myenvironment = environment_map(environ);
  //builds the vector of directories to search for executables through

  //reads command from Terminal
  std::string input = ReadCommand();
  std::string path(myenvironment->at("PATH"));;

  int * file_descript = (int *)  malloc(1*sizeof(*file_descript));
  (*file_descript) = -2;
  std::pair<std::string, std::string> redirect_check = redirect(input, file_descript);
 
  //this boolean examines if set,  cd or $var ran in one child process
  bool universal;

  //this one checks if export ran
  bool export_var = false;

  //this one checked if the string inputted in NULL
  bool empty = false;
  
  //gets the value of the PATH environment variable's key
  std::vector<std::string> *  MyFiles = new std::vector<std::string>;
  GetPath(MyFiles); // = new std::vector<std::string>;
  while ((input.compare("exit") != 0)&&(!input.empty())) {

    //reads next command if last one was all white space
    if (empty == true) {
      empty = false;
      input = ReadCommand();
    }
    //exits in case last command was empty string and this command was exit
    if (input.compare("exit") ==0) {
      break;
    }
    //this vector has the actual command and its arguments that are typed into the shell
    std::vector<std::string> *  myvector = arglistbuilder(redirect_check.first);
     if (myvector->empty()) {
      empty = true;
      continue;
    }
     
    int array_size = myvector->size();
    char * cmd =(char *)  malloc((myvector->at(0).length()+1)*sizeof(*cmd));
    strncpy(cmd, myvector->at(0).c_str(), myvector->at(0).length()+1);
    //runs 3 built in commands: set, $var, cd
    universal = builtcommand(myenvironment, myvector, input, cmd);
    //checks if the input was an export command
    export_var = exportvariable(myenvironment, myvector);
    if (universal !=  true) {
      if (export_var != true) {
	//make a child and parent process if set, export, $var or cd were not typed in
	char ** const newargs = argarray(myvector, MyFiles);
	runCommand(redirect_check, file_descript, newargs, cmd, environ);
 	cleanup(array_size, newargs);
      }
    }
    //updates the list of directories to search through if the PATH variable changes and is exported
    if ((path.compare(myenvironment->at("PATH")) != 0)) {
      if (export_var != false) {
	MyFiles->clear();
	GetPath(MyFiles);
	path.assign(myenvironment->at("PATH"));
      }
    }
    //read a command at the end of the loop
    input = ReadCommand();
    (*file_descript) = -2;
    redirect_check = redirect(input, file_descript);
     free(cmd);
     delete myvector;
  }
  //not continually built in each iteration of the loop so deleted separately
  delete myenvironment;
  free(file_descript);
  delete MyFiles;
  return EXIT_SUCCESS;
}
