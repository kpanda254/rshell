#include "CommandToken.hpp"

void CommandToken::addArgument(char* arg) {
	arguments.push_back(arg);
}

string CommandToken::getName() {
	return commandName;
}

bool CommandToken::execute() {
	arguments.push_back(NULL);
	char* arr[1000];

	int size = arguments.size();
	for (int i = 1; i < size + 1; i++) {
		arr[i] = arguments.front();
		arguments.pop_front();
	}
	arr[0] = commandName;

	//execvp/fork implementation

	// handles "false" command
	string falseLiteral = "false";
	if (arr[0] == falseLiteral) {
		return false;
	}
	
	// handles "test" command
	string testLiteral = "test";
	string fTestFlag = "-f";
	string dTestFlag = "-d";
	string eTestFlag = "-e";
	
	string outputFlag = ">";
	string outputFlag2 = ">>";
	
	struct stat buf;
	if (arr[0] == testLiteral) {
		if (size > 2) {
			if (stat(arr[2], &buf) != -1) {
				if ((arr[1] != eTestFlag) && (arr[1] != fTestFlag) && (arr[1] != dTestFlag)) {
					perror("Invalid Test Flag");
					return false;
				}
				if (arr[1] == fTestFlag) {
					if (buf.st_size != 0) {
						if (S_ISREG(buf.st_mode) == 1) {
							cout << "(True)" << endl;
							return true;
						}
					}
				}
				else if (arr[1] == dTestFlag) {
					if (buf.st_mtime != 0) {
						if (S_ISDIR(buf.st_mode) == 1) {
							cout << "(True)" << endl;
							return true;
						}
					}
				}
				else {
					if (buf.st_size != 0) {
						cout << "(True)" << endl;
						return true;
					}
				}
				cout << "(False)" << endl;
				return false;
			}
		}
		// -e by default
		if (size == 2){
			if (stat(arr[1], &buf) != -1) {
				if (buf.st_size != 0) {
					cout << "(True)" << endl;
					return true;
				}
			}
		}
		cout << "(False)" << endl;
		return false;
	}
	else if (size == 3 && arr[1] == outputFlag) {
		int descriptor = open(arr[2], O_WRONLY| O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
		
		if (dup2(descriptor, 1) < 0) {
			perror("dup2");
			return false;
		}
		else {
			CommandToken* cmd2 = new CommandToken(arr[0]);
			cmd2->execute();
			
			return true;
		}
	}
	else if (size == 3 && arr[1] == outputFlag2) {
		int descriptor = open(arr[2], O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
		
		if (dup2(descriptor, 1) < 0) {
			perror("dup2");
			return false;
		}
		else {
			CommandToken* cmd2 = new CommandToken(arr[0]);
			cmd2->execute();
			
			return true;
		}
	}
  
	// array for storing commands from user input
	pid_t wait_for_result;
	int status;
	// fork child process
	pid_t child_process = fork();
	//Error messages after fork updates
	if (child_process < 0) {
		perror("Fork Failed");
		// terminates program upon recieving fork failure with return val of 1
		return false;
	}
	else if (child_process == 0) {
	  // execute command from our array of strings
	  // includes error handling for execvp
	  if (execvp(arr[0], arr) < 0) {
		  perror("Execvp Failed");
		  return false;
	  }
	}
	else {
		// wait for child process to finish
		wait_for_result = waitpid(child_process, &status, WUNTRACED);
	}
	return true;
}
