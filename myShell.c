#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h> 
#include <dirent.h>
#include <glob.h>
#include <fcntl.h>

#define BUFFER_SIZE 2048

int modeInteractive = 0; //indicates interactive or batch mode
int QUIT = 0; //indicate when to exit
char path[500]; //path to external programs for external exe or which cmd
char wildcardArgs[500]; //wildcard expansion
int lengthofArgs = -1; //Find the amount of arguments
int redirectPos = 0; //position where redirects are found
int lastExitCMD = 0; //Holds success(1) or failure(0) of last command
int childCMD = 0; //Holds child process success(1) or failure(0)
int pathFileCall = 0; //1 if first args starts with /, 0 if not
int test = 0; //For piping input into ./myShell

void checkCMDS(char* cmdLine, char** args, int cmdPos);
int checkForRWP(char** args, int cmdPos);
void myPipe(char** args, int cmdPos);
void redirectInput(char** args, int inputPos);
void redirectOutput(char** args, int outputPos);


char *readLine(int inputFD){ //Reads line from terminal only
	char *line = (char*)malloc(sizeof(char)*BUFFER_SIZE); //allocates buffer
    int bufferSize = BUFFER_SIZE;
    char bufferArr[BUFFER_SIZE];
    int bufferpos = 0, pos = 0;
	char letter;
	if (line == NULL){
		exit(EXIT_FAILURE);
	}
    int bytesToRead, indexOfWord = 0;
    while ((bytesToRead = read(STDIN_FILENO, bufferArr, BUFFER_SIZE - 1)) > 0) {
            for (int i = 0; i < bytesToRead; i++) { //Loops through buffer
                if (bufferArr[i] == '\n') { //checks for new line
                    if (!(indexOfWord < 1)) { //Checks if its at the end of word
                        line[indexOfWord] = '\0'; //Puts null terminator for end of string
                        indexOfWord = 0;
                        return line;
                    }
                    else{ //prevents splitcmd from bugging out
                        line[0] ='a';
                        line[1] = '\0';
                        return line;
                    }
                } 
                else { //adds char to form line
                    line[indexOfWord] = bufferArr[i];
                    indexOfWord++;
                }
            }
        }
    line[0] ='a';
    line[1] = '\0';
    return line;
}   

char **splitCMDS(char *cmdLine){ //Splits commands into arguments
	char **tokensArr = (char**)malloc(sizeof(char*)*254);
    int bufferSize = 128;
    int pos = 0;
	char *tokenize;
	char delimiter[10] = " ";
	if (tokensArr){
        tokenize = strtok(cmdLine, delimiter);
	}
    else{
        exit(EXIT_FAILURE);
    }
	while (tokenize != NULL)
	{
		tokensArr[pos] = tokenize;
        tokenize = strtok(NULL, delimiter);
		pos++;
	}
	tokensArr[pos] = NULL;

    lengthofArgs = -1;
    while (tokensArr[++lengthofArgs] != NULL){}

	return tokensArr;
}

void exeExternalCMDS(char **args) { //executes external CMDS, change some more
    childCMD = 1;
    pid_t pidExe;
    int childExitStatus, status;
    pidExe = fork();
    if (pidExe == 0) {
        // Child process
        if (wildcardArgs[0] != '\0'){
            status = execv(path, splitCMDS(wildcardArgs));
            if (status == -1) {
                lastExitCMD = 0;
                printf("Error executing program\n");
            }
        exit(EXIT_FAILURE);
        }
        else{
            status = execv(path, args);
            if (status == -1) {
                lastExitCMD = 0;
                printf("Error executing program\n");
        }
        exit(EXIT_FAILURE);
        }
    } 
    else if (pidExe < 0) {
        lastExitCMD = 0;
        printf("Error forking\n");
    } 
    else {
        do {
            waitpid(pidExe, &childExitStatus, WUNTRACED);
            if(childExitStatus == 0){ //checks if cmd succeeded
                lastExitCMD = 1;
            }
            else{
                lastExitCMD = 0;
            }
        } while (!WIFEXITED(childExitStatus) && !WIFSIGNALED(childExitStatus));
    }
    wildcardArgs[0] = '\0'; 
}

int copyFilePath(char** args, int whichCallee, int pos){ //helper function for checkExternalCMDS()
    //copies filepath
    int len = strlen(path);
    path[len] = '/';
    int oldLen = len+1;
    len = strlen(args[pos]) + 1;
    strncpy(&path[oldLen], args[pos], len);
    if (pathFileCall){
        strcpy(path, args[0]);
    }
    if (access(path, F_OK) == 0){
        if (whichCallee){
        }
        else{
            int RWP = checkForRWP(args, pos);
            if (RWP == 1){
                //pipe
                return 1;
            }
            if (RWP == 2){
                //redirect >
                redirectOutput(args, redirectPos);
                return 1;
            }
            if (RWP == 3){
                //redirect <
                redirectInput(args, redirectPos);
                return 1;
            }
            exeExternalCMDS(args);
        }
        return 1;
    }
    return 0;
}

int checkExternalCMDS(char **args, int whichCallee){ //Checks for external CMDS
    int checker = 0;
    int pos = 0;
    if (whichCallee){
        pos = 1;
    }
    if (access(args[pos], F_OK) == 0){
        }
        switch (1){
            case 1: 
                strcpy(path, "/usr/local/bin");
                checker = copyFilePath(args, whichCallee, pos);
                if (checker){
                    break;
                }
            case 2:
                strcpy(path, "/usr/bin");
                checker = copyFilePath(args, whichCallee, pos);
                if (checker){
                    break;
                }
            case 3:
                strcpy(path, "/bin");
                checker = copyFilePath(args, whichCallee, pos);
                if (checker){
                    break;
                }
        }
    return checker;
}

//Declarations of Builtin/Internal Shell Commands
void cdShell(char** args, int cmdPos);
void pwdShell(char** args);
void whichShell(char** args, int cmdPos);

int checkInternalCMDS(char** args, int cmdPos){ //checks for Internal CMDS
    if (!strcmp(args[cmdPos], "exit")){
        QUIT = 1;
        if (modeInteractive){
            printf("Exiting my Shell\n");
        }
        return 1;
    }
    else if (!strcmp(args[cmdPos], "cd")){
        childCMD = 1;
        cdShell(args, cmdPos);
        return 1;
    }
    else if (!strcmp(args[cmdPos], "pwd")){
        childCMD = 1;
        pwdShell(args);
        return 1;
    }
    else if (!strcmp(args[cmdPos], "which")){
        childCMD = 1;
        whichShell(args, cmdPos);
        return 1;
    }
    return 0;
}

void cdShell(char** args, int cmdPos){ //cd CMD
    if (args[cmdPos+1] != NULL){
        lastExitCMD = 1;
        if (chdir(args[cmdPos+1])){
            lastExitCMD = 0;
            printf("mysh: %s: %s: No such file or directory\n", args[cmdPos], args[cmdPos+1]);
        }
    }
}

void pwdShell(char** args){ //pwd CMD
    lastExitCMD = 1;
    char pwd[512];
    getcwd(pwd, sizeof(pwd));
    printf("%s\n", pwd);
}

void whichShell(char** args, int cmdPos){ //which CMD
    lastExitCMD = 0;
    if (args[cmdPos+1] != NULL){
        if (checkExternalCMDS(args, 1)){
            lastExitCMD = 1;
            printf("%s\n", path);
        }
    }
    path[0] = '\0';
}

void wildcard(char* wCard){ //wildcard expansion *
    int r;
    glob_t gstruct;
    r = glob(wCard, GLOB_ERR, NULL, &gstruct);
    if (r!=0){
        printf("No Matches\n");
        return;
    }
    char** found;
    found = gstruct.gl_pathv;
    strcpy(wildcardArgs, wCard);
    while(*found){
        strcat(wildcardArgs, " ");
        strcat(wildcardArgs, *found);
        found++;
    }
    strcpy(wCard, wildcardArgs);
    globfree(&gstruct);
}

void redirectInput(char** args, int inputPos){ //redirects input <
    if(inputPos > -1){
        childCMD = 0;
        char* inputFile = args[inputPos+1];
        args[inputPos] = NULL;
        args[inputPos+1] = NULL;
        for(int i = inputPos; i < lengthofArgs-2; i++){//adjusts cmd
            args[i] = args[i+2];
        }
        lengthofArgs -= 2;

        pid_t inputPID = fork();
            if (inputPID < 0){
                printf("Forking Error\n");
                lastExitCMD = 0;
                exit(EXIT_FAILURE);
            } 
            else if (inputPID == 0) {
                //open file
                int fileIn = open(inputFile, O_RDONLY);
                if (fileIn < 0){
                    lastExitCMD = 0;
                    printf("Error opening File\n");
                    exit(EXIT_FAILURE);
                }
                //activates dup2
                int dup2Num = dup2(fileIn, STDIN_FILENO);
                if (dup2Num < 0){
                    lastExitCMD = 0;
                    printf("Error dup2\n");
                    exit(EXIT_FAILURE);
                }
                close(fileIn);
                //exe cmds
                checkCMDS(" ", args, 0);
                childCMD = 1;
                lastExitCMD = 1;
                exit(EXIT_SUCCESS);
            }
        //waits for process to end
        
        wait(NULL);
        lastExitCMD = 1;
    }
}

void redirectOutput(char** args, int outputPos){ //redirects output >
    if(outputPos > -1){
        childCMD = 0;
        char* outputFile = args[outputPos+1];
        args[outputPos] = NULL;
        args[outputPos+1] = NULL;
        for(int i = outputPos; i < lengthofArgs-2; i++){//adjusts cmd
            args[i] = args[i+2];
        }
        lengthofArgs -= 2;

        pid_t outputPID = fork();
            if (outputPID < 0) {
                lastExitCMD = 0;
                printf("Forking Error\n");
                exit(EXIT_FAILURE);
            } 
            else if (outputPID == 0){ //opens file
                int fileout = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0640);
                if (fileout < 0){
                    lastExitCMD = 0;
                    printf("Error opening File\n");
                    exit(EXIT_FAILURE);
                } //activates dup2
                if (dup2(fileout, STDOUT_FILENO) < 0){
                    lastExitCMD = 0;
                    printf("Error opening File\n");
                    exit(EXIT_FAILURE);
                }
                close(fileout);
                //exe cmds
                checkCMDS(" ", args, 0);
                childCMD = 1;
                lastExitCMD = 1;
                exit(EXIT_SUCCESS);
            }
        //waits for process to end
        wait(NULL);
        lastExitCMD = 1;
    }
}

int checkForRWP(char** args, int cmdPos){ //checks for redirects, wildcards, and pipes
    for(int i = cmdPos; i < lengthofArgs; i++){
        if (strchr(args[i], '*')){ //wildcard
            wildcard(args[i]);
        }
        if (!strcmp(args[i], "|")){ //pipe
            myPipe(args, i);
            return 1;
            //checkCMDS(args[i+1], args, i);
        }
        else if (!strcmp(args[i], ">")){ //output redirect
            redirectPos = i;
            return 2;
        }
        else if (!strcmp(args[i], "<")){ //input redirect
            redirectPos = i;
            return 3;
        }
    }
    return 0;
}

void myPipe(char** args, int cmdPos){ //function to handle piping |
    if (cmdPos != -1) {
        // Split the command into two parts: before and after the pipe
        int childExitStatus;
        char **commandFirst = malloc(sizeof(char *) * (512));
        char **commandSecond = malloc(sizeof(char *) * (512));

        int cmd1len = 0, cmd2len = 0; //first and second command lengths
        //two loops to copy first and second command
        for (int i = 0; i < cmdPos; i++){
            commandFirst[i] = strdup(args[i]);
            cmd1len++;
        }
        for (int i = cmdPos + 1; i < lengthofArgs; i++){
            commandSecond[cmd2len] = strdup(args[i]);
            cmd2len++;
        }
        commandFirst[cmd1len] = NULL; //set both to NULL
        commandSecond[cmd2len] = NULL;

        //Creates piping process
        int pipeLink[2];
        if (pipe(pipeLink) == -1){
            lastExitCMD = 0;
            printf("Pipe Error\n");
            exit(EXIT_FAILURE);
        }

        pid_t pidFirst = fork();
        //first command fork
        if (pidFirst < 0){
            lastExitCMD = 0;
            printf("fork error\n");
            exit(EXIT_FAILURE);
        }
        else if (pidFirst == 0){
            //closes the links
            close(pipeLink[0]);
            dup2(pipeLink[1], STDOUT_FILENO);
            close(pipeLink[1]);
            //adjusts length of arguments and exe
            lengthofArgs = cmd1len;
            checkCMDS(" ",commandFirst, 0);
            lastExitCMD = 1;
            exit(EXIT_SUCCESS);
        }
        else{
            pid_t pidSecond = fork();
            //second command fork
            if (pidSecond == -1){
                lastExitCMD = 0;
                printf("fork error\n");
                exit(EXIT_FAILURE);
            }
            else if (pidSecond == 0){
                //closes links
                close(pipeLink[1]);
                dup2(pipeLink[0], STDIN_FILENO);
                close(pipeLink[0]);
                //adjusts length of arguments and exe
                lengthofArgs = cmd2len;
                checkCMDS(" ", commandSecond, 0);
                lastExitCMD = 1;
                exit(EXIT_SUCCESS);
            }
            else{
                //closes links
                close(pipeLink[0]);
                close(pipeLink[1]);
                //waits for subprocess to finish
                waitpid(pidFirst, &childExitStatus, WUNTRACED);
                waitpid(pidSecond, &childExitStatus, WUNTRACED);

                for (int id = 0; commandFirst[id] != NULL; id++){
                    free(commandFirst[id]);
                }
                for (int id = 0; commandSecond[id] != NULL; id++){
                    free(commandSecond[id]);
                }
                free(commandFirst);
                free(commandSecond);
                //Memory allocations to be freed
                return;
            }
        }
    }
}

void intializeBatch(char** args, int fd){ //activates batch mode
    if (fd == -1) {
        if (!test){
            printf("Error opening file\n");
        }
        return;
    }
    else {
        char bufferArr[2048], wordArr[200]; //bufferArr is for read() buffer, wordArr is for holding words
        int bytesToRead, indexOfWord = 0;

        while ((bytesToRead = read(fd, bufferArr, sizeof(bufferArr) - 1)) > 0) {
            for (int i = 0; i < bytesToRead; i++) { //Loops through buffer
                if (bufferArr[i] == '\n') { //checks for new line
                    if (!(indexOfWord < 1)) { //Checks if its at the end of word
                        wordArr[indexOfWord] = '\0'; //Puts null terminator for end of string
                        indexOfWord = 0;
                        args = splitCMDS(wordArr);
                        checkCMDS(wordArr, args, 0);
                        free(args);
                    }
                } 
                else {
                    wordArr[indexOfWord] = bufferArr[i];
                    indexOfWord++;
                }
            }
        }
    }
}

void intializeShell(char** argv, int fileOpened){ //intializes shell for either interactive or batch mode, main argument loop
    char* cmdLine;
	char** args;
    int fd = 0;
	while(QUIT == 0){
        if (modeInteractive){
            printf("mysh> ");
            fflush(stdout);
		    cmdLine=readLine(fd);
		    args=splitCMDS(cmdLine);
            checkCMDS(cmdLine, args, 0);
        }
        else{ //for batch mode
            intializeBatch(args, fileOpened);
            QUIT = 1;
        }

        if (modeInteractive){
            free(cmdLine);
		    free(args);
        }
	}
}

void checkCMDS(char* cmdLine, char** args, int cmdPos){ //checks for what kind of CMD
    pathFileCall = 0;
    if(childCMD == 0){
        childCMD = 1;
    }
    if (cmdLine[0] == '/'){
        pathFileCall = 1;
        copyFilePath(args, 0, 0);
    }
    else if (checkInternalCMDS(args, cmdPos)){}
    else if (checkExternalCMDS(args, 0)){}
    else if(!strcmp(args[0], "else")){
        if (!lastExitCMD){
            args[0] = NULL;
            for(int i = 0; i < lengthofArgs; i++){
                args[i] = args[i+1];
            }
            lengthofArgs -= 1;
            checkCMDS(cmdLine, args, 0);
        }
    }
    else if(!strcmp(args[0], "then")){
        if (lastExitCMD){
            args[0] = NULL;
            for(int i = 0; i < lengthofArgs; i++){
                args[i] = args[i+1];
            }
            lengthofArgs -= 1;
            checkCMDS(cmdLine, args, 0);
        }
    }
    else {lastExitCMD = 0;}
}

int main(int argc, char** argv){ //main function to activate shell
    modeInteractive = isatty(STDIN_FILENO);
    int fileOpened = -1;
    if(!modeInteractive){
        argc = 2;
        test = 1;
    }
    modeInteractive = 0;
    if (argc == 1){
        modeInteractive = 1;
        printf("Welcome to my Shell!\n");
        intializeShell(argv, -1);
    }
    else if (argc > 1){
        if (test){
            intializeBatch(argv, STDIN_FILENO);
        }
        else{
            fileOpened = open(argv[1], O_RDONLY);
        }
        modeInteractive = 0;
        intializeShell(argv, fileOpened);
        close(fileOpened);
    }
    else {
        printf("Invalid argument #\n");
    }

    return EXIT_SUCCESS;
}