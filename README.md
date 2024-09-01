myShell(), CS214 SPR24

Project by: Ivan Pan(ip278)

Implementation of myShell(): Design
**myShell.c**; 
     -First, isAtty() is used to check if there is a file to be inputted which will automatically indicate batch mode. If that fails, then we check for amount of arguments if more than 1, its batch mode again. If 1 then it is interactivemmode. Then this leads to the main argument function, with a while loop that reiterates till either batch mode hits EOF or a exit statement, OR the cliet/user types in exit.
     -Within the main arugment loop, (This is interactive mode) there is a readLine function that uses read() to read input from the terminal. Then it will parse those lines into different command arguments using splitcmds() function. Which uses a token system to seperates them using whitespaces into tokens and put into a token array. This is then returned back to the main argument function, which will then initiate command checking for either built in commands, external commands, or path files.
        -If it is a built in command it will lead to checking if the string is either "cd", "pwd", or "which". If it is a "cd", the cd function will activate, which will change directories while changing that directory exists. Printing an error statement if it does not. If it is a "pwd" command then the current working directory will be printed. If it is "which" command then it will first check if there is a second argument for the which to correctly function. If there is, then it will take that second argument and go through checkExternalCMDS() function to check a path that leads to that program and print out the path.
        -If it is a external command, it will first check for the path to get the program. Once the path is found, the path will copied and passed over the exeEternalCMD() function, which will fork and use execv() to perform the commands.
        -If it is a path file then, it will follow the same process as external command. First it checks that path is legit and once its found, it will be execueted using the exeEternalCMD() function.
    -(in batch mode), Batch mode is more simpler. If interactive mode is not activated then the main loop argument will intialize batch mode and read through 1 line at a time while performing checkCMDS() similarly to interactivemode except there is no user input only lines from the file. Once all lines are completed, the system will auto exit.
    -There is a redirect, pipe, and wildcards checks that will be performed when checking commands, in order to make sure that all pipes, redirects, and wildcards are correctly handled.
        -wildcards are checked by looking for any * within the arguments. If found and the argument before that is a valid command. It will use the wildcard() function to perform expansion. This use glob() to look for all matches and then that will be inserted into the arguments. For later exe of the command.
        -Pipes is checked by looking for "|" in the arguments. If found then the system will activate the pipe function which will separate the front command before the | and the back command after the |. This will create two commands that will act as a stdin and stdout. Then those commands will be tokenized again to make it readable for the system. Then the system exe the commands using fork() and making two sub processes.
        -redirection are seperated into < >, which checks for each one in the arguments. If any of them are found, respective redirection function will activate and it will remove either <> and the path. Then recreate the command argument list by shifting all the later arguments forward. Now that there is a complete command, fork() will be used with dup2() wtih stdin or stdout, to ensure that the redirection is correctly handled.
    -There is also a childCMD and lastExitCMD check to see if the child processes commands are successful and if the last commands are successful, this is to handle conditionals. Which is also check for in the checkCMDS() function. Else will only run if the previous command was unsuccessful, and then will only run if the previous was successful.

**testCases**
Provided testing: 
    ./myShell myscript.sh
    -This tests for basic builtin and external commands that myShell() should be able to run. Examples are cd, changing directories using .. and testCases directory. Also runs the pwd command to double check that directories are correctly being changed. Echo is used to notify the users of whats going on and also checks that the external command echo is usable. ls is used to ls files within testCases, specifically t*t.txt files. Which command is also tested through which echo and which cat to make sure their locations are correctly revealed.

    ./myShell mypipe.sh
    -This tests for all pipe functions that needs to be correct. Examples are piping two rm functions first, rm t1t.txt | rm t4t.txt. Which checks that both commands are being runned. Then touch t1t.txt | touch t4t.txt is ran to check that the files are being replaced. To ensure that rm and touch is correctly being handled, you may type anything in those files and they should be gone after running this test. ls | wc and ls | grep my is also used to check that input and output redirects by pipes are correctly being done.

    ./myShell myredirect.sh
    -This tests for all redirect functions that needs to be working. Examples are using the wc -l to check for total lines. This is used with < on a linecounter.txt that has 6 lines. Which will cause the system to output 6 as a result. Since it takes the input of linecounter. Then it tests for > by doing "echo hi guys > output.txt" which will output that message into output.txt which can be checked. Then it will test for both usages of < > at the same time. "wc -l < linecounter.txt > output.txt" This will end up getting an input of 6, which will be outputted to output.txt with a 6 in it.

    ./myShell myconditionals.sh
    -This tests for condtionals such as 'else' and 'then'. First we check the standard cd failed statement with an else pwd, to make sure else works correctly. Then we do a successful cd to make sure then works with a pwd. Afterwards, we will check for a double else for more complex situations. Where the last statement should only run if the first 2 fails. Which is true for my program. Both cd will fail and the last else of pwd will perform.

    cat <any .sh> | ./myShell
    -This tests that inputs can be redirected to myShell program and that it can also correctly read the inputs. Use any of the above sh files for testing and the output should be the same, which indicates that myShell program correctly reads the pipe inputs.


__HOW TO COMPILE__
$ make
executables: ./myShell
Interactive mode: ./myShell
BatchMode: ./myShell <file.sh>

Provided testing: ./myShell myscript.sh ./myShell mypipe.sh ./myShell myredirect.sh ./myShell myconditonals.sh
