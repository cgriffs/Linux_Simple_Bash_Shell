//Linux Simple SSI (Shell Interpreter)
//Caleb Griffin
//V00934910
//--------------------------------------------------------
//                      LIBRARIES
//--------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/unistd.h>

//--------------------------------------------------------
//                   INITIALIZATION
//--------------------------------------------------------
void printUserID();
void takeUserInput();
void runSimpleCommand(char**);
void cd(char**);
void bg(char**);
void addToBg(char**, pid_t);
void deleteFromBg(pid_t);
void listBg();
void checkBackground();

// GLOBAL
typedef struct bgNode {
    char token[500];
    pid_t pid;
    struct bgNode* next;
} bgnode;
bgnode* head = NULL;
int bgTotalRunning = 0;

//--------------------------------------------------------
//                        MAIN
//--------------------------------------------------------
int 
main() {
    printf("_______________________\n");
    printf("________WELCOME________\n");
    printf("________TO  SSI________\n");
    printf("_______________________\n");
    
    // RUNNING SHELL IN LOOP
    for(;;){
        printUserID();
        takeUserInput();
    }
    return 0;
}

//--------------------------------------------------------
//                      FUNCTIONS                      
//--------------------------------------------------------
//Function that handles the required user id prompt
void
printUserID(){
	char* username; 
    username = getlogin();
	
    char hostname[100];
    gethostname(hostname, sizeof(hostname));
	
    char cwd[100];
    getcwd(cwd, sizeof(cwd));
    printf("%s@%s: %s > ",username,hostname,cwd);
}
//--------------------------------------------------------
//Function that takes user input and handles it 
void
takeUserInput(){
    char input[250];
    scanf("%[^\n]%*c",input);
    char* commands = strtok(input," ");
    
    char* token[250];
    int i = 0;
    while (commands) {
        token[i++] = commands;
        commands = strtok(NULL, " ");
    }
    token[i] = NULL;

    //Check for Running Background Processes
    checkBackground();
    //Change Directory
    if (!strcmp(token[0], "cd")) {
        cd(token);
    } 
    //bg handling
    else if (!strcmp(token[0], "bg")) {
        bg(token);
    } 
    //bglist handling
    else if (!strcmp(token[0], "bglist")) {
        listBg();
    } 
    //run normal(simple) command
    else {
        runSimpleCommand(token);
    }
}
//--------------------------------------------------------
//Function to run simple ssi commands
void
runSimpleCommand(char** token){
    pid_t pid = fork();

    if (pid == 0) {
        execvp(token[0], token);
    } else {
        waitpid(pid, NULL, 0);
    }
}
//--------------------------------------------------------
//Function to change current working directory
void
cd(char** token){
    int newdir;

    //Move Up directory
    if (token[1] == NULL || !strcmp(token[1], "..")) {
        newdir = chdir("../");
    } 
    //Return Home directory
    else if (!strcmp(token[1], "~")) {
        newdir = chdir(getenv("HOME"));
    } 
    //Follow User Input Path
    else {
        newdir = chdir(token[1]);
    }
}
//--------------------------------------------------------
//All following Function's to help run background exec
void
bg(char** token){
    pid_t pid = fork();

    if (pid == 0) {
        execvp(token[1], token + 1);
    } else if (pid > 0) {
        addToBg(token, pid);
    }
}
//--------------------------------------------------------
//Function help from tutorial
void
addToBg(char** token, pid_t pid){
    bgnode* newNode = (bgnode*)malloc(sizeof(bgnode));
    newNode->pid = pid;
    newNode->token[0] = '\0';
    int i = 1;

    while(token[i] != NULL) {
        strcat(newNode->token, token[i]);
        strcat(newNode->token, " ");
        i++;
    }
    newNode->next = head;
    head = newNode;
    bgTotalRunning++;
}
//--------------------------------------------------------
//Function help from tutorial
void 
deleteFromBg(pid_t pid){
    bgnode* temp = head;
    
    if (head->pid == pid) {
        printf("%d: %s has terminated.\n", temp->pid, temp->token);
        head = head->next;
    } else {
        while(temp->next->pid != pid) 
            temp = temp->next;

        printf("%d: %s has terminated.\n", temp->next->pid, temp->next->token);
        temp->next = temp->next->next;
        free(temp->next);
    }
    bgTotalRunning--;
}
//--------------------------------------------------------
void
listBg(){
    bgnode* temp = head;

    int i=1;
    while(temp != NULL) {
        printf("%d: %s %d\n", temp->pid, temp->token, i);
        temp = temp->next;
        i++;
    }
    printf("Total Background Jobs: %d\n", bgTotalRunning);
}
//--------------------------------------------------------
//Function help from tutorial
void checkBackground() {
    if (bgTotalRunning > 0) {
        pid_t pid = waitpid(0, NULL, WNOHANG);

        while(pid > 0) {
            deleteFromBg(pid);
            pid = waitpid(0, NULL, WNOHANG);
        }
    }
}