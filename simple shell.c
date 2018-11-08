    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <fcntl.h>

    // Required Libraries
    #include <string.h>
    #include <ctype.h>      // isalnum()
    #include <sys/types.h>  // fork() execvp() waitpid()
    #include <sys/wait.h>   // waitpid()
    // Defining Constants
    #define MAX_CHAR 100    // Maximum number of character that user should enter
    #define MAX_WORDS 15    // Maximum number of words in the command line
    #define DEL " "         // The delimiter, that will separate each word
    
    // Defining some colors :D :D 
    #define C_RED     "\x1b[91m"
    #define C_GREEN   "\x1b[92m"
    #define C_YELLOW  "\x1b[93m"
    #define C_BLUE    "\x1b[94m"
    #define C_MAGNETA "\x1b[95m"
    #define C_CYAN    "\x1b[96m"
    #define C_RESET   "\x1b[0m"
//-
    struct red{
        int count ;
        int index ;
    }in_red, out_red, pipe_red;


		char* input_file;         //k
		char* output_file;          //k


    // Defining Needed Global Variables
    char entered_line[MAX_CHAR] ;   // It will hold the  string input from the user
    char edited_line[2*MAX_CHAR] ;  // It will hold the previous string input from the user after checking each character
    char* word[MAX_WORDS+1] ;       // It will hold a pointers to the words strings after splitting the user input
    char cwd[30] ;                    // This will hold the current directory name    
    int words_num ;                 // It will hold the number of words in command line
    pid_t pid_1 ;
    pid_t pid_2 ;

    int piping_arr[2] ;

    // Defining Functions Prototype
    int testing_char(char c) ;  // This function is testing every character passed to it and check it's type
    void read_line() ;          // This function reads every command line and rearrange it with spaces
    int parse_line() ;          // This function is splitting the string input to word tokens

    void input_redirecting();
    void output_redirecting() ;
    void command_exec();
    void piping_exec() ;
    void resetting_flags() ;


int main(){


        while(1)  // We are taking user's commands for ever until he inputs exit, then the loop will break
            {

            // First of all, reset all flags
            resetting_flags();
            getcwd(cwd, 30) ;
            // Shell style:    sish:>DIRECTORY$ command_line
            printf("%s%s%s%s%s$ %s", C_GREEN,"sish:>", C_BLUE, cwd, C_GREEN, C_CYAN) ;    
            read_line();                // Start reading the user's command line and arrange it
            
            
            words_num = parse_line() ; // Start splitting the user's command line and get the number of command words

            // First, In the case that the user has inputted 1 word, and consist of 4 chars and equals to "exit"
            // Or EOF signal has been risen
            if     (words_num ==1 && word[0][4] == '\0' && strncmp(word[0], "exit", 4) == 0 || feof(stdin))
                {
                    printf(C_RED);    // Changing the output color
                    printf("%s%s\n", C_RED, "Terminating...") ;
                    exit(0) ;                             // End the program
                }

            // Second, If the user hasn't input any words
            else if(words_num == 0)
                {
                    printf("%s%s\n", C_RED,"No command has been entered") ;
                }

            // Third, If the command words is greater than permissible
            else if(words_num == -1)
                {
                    printf("%sError: Only %s%d%s words are allowed in one command line\n",C_RED, C_MAGNETA, MAX_WORDS, C_RED);
                }

            //-
            else if(words_num == -2)
                {
                    
                    printf("%s%s\n", C_RED,"Syntax Error") ;
                }

            // The desired case, if the users has inputted a number of words as a command
            else
                {
                // First, fork the incoming instruction and assign the return value at pid
                if (pipe_red.count == 0)
                    {
                    command_exec() ;
                    }
                else
                {
                    piping_exec();
                }

            }
    }

    return 0 ;
    }


    // Implementation of used functions

    // This Function returns:  1 => Permitted character,  2 => Redirection character,  0 => Denied character
    int testing_char(char c){
        // isalnum() is a standard function returns 0 if the passed character isn't a letter nor digit
        if (isalnum(c) != 0 || c == ' ' || c == '.'|| c == '-' || c == '/' || c== '$')
        {
            return 1 ;
        }
        else if(c== '<' || c =='>' || c=='|')
        {
            return 2 ;
        }
        else
        {
            return 0 ;
        }
    }

    /* This function first reads the input and put it in entered_line, then process it by doing 2 checks, the first is to
    check the denied characters and replace them with white spaces.
    second, since it's requested that the white spaces around the redirections are optional, so we will put them around it
     anyway to guarantee the white spaces around the redirections so it can be split as one word */
     // We are reading a char by char from entered_line and write the processed chars to edited_line
    void read_line(){   // fgets() is a standard function reads the stdin and put the input into array of characters that passed to it
        fgets(entered_line, MAX_CHAR, stdin);

        int i=0 ;           // array indicator for the array: entered_line
        int j = 0 ;         // array indicator for the array: edited_line
        int test_result ;   // It will hold the test result for each character which is produced by testing_char() [0,1,2]
        // Looping all off entered_line characters
        do
        {
            test_result = testing_char(entered_line[i]) ;
            if(test_result == 0)                        // First, If it's a denied character
            {
                edited_line[j] = ' ' ;                  // Write a white space instead
                j++ ;                                   // Move to the new writing location

            }
            else if(test_result == 2)                   // If it's a redirection char
            {                                           // write it as : white space - redirection - white space
                edited_line[j] = ' ';
                j++ ;
                edited_line[j] = entered_line[i] ;
                j++ ;
                edited_line[j] = ' ' ;
                j++ ;
            }
            else                                        // If it's a permitted character
            {
                edited_line[j] = entered_line[i] ;      // Write it directly
                j++ ;
            }
            // Move the entered_line indicator to the next element
            i++ ;

        } while(entered_line[i] != '\n' && entered_line[i+1] != '\0') ;      // Continue until the next element becomes Null, then it's the end of line

    }



    // This function is working in splitting the line into words according to the delimiter: white space
    // Returns: 0 => the user has entered null, -1 => there is words more than the permitted, i => number of entered words
    // Note: No matter how many white spaces between the 2-words, It will split them any way :)
    int parse_line(){
        int i = 0 ;  // Array indicator of: word[]
        // strtok() is a standard that splits the passed strings by the passed delimiter
        word[0] = strtok(edited_line, DEL) ;

        if (word[0] == NULL){               // If the users entered no thing, return 0
            return 0 ;
        }
        else
            {
            while (word[i] != NULL){
                i++ ;
                word[i] = strtok(NULL, DEL) ;
                if (i>15)
                    { return -1;}           // If the user has exceeded the number of permitted words

                }
                word[i+1] = NULL ;

                //-
                int k ;
                for(k=0; k<i; k++){
                    if(strncmp(word[k], "<", 1) == 0){
                        if (in_red.count  > 0){
                            return -2 ;
                        }
                        else{
                            in_red.count ++ ;
                            in_red.index = k ;
                            word[k] = NULL ;
                        }

                    }
                    else if(strncmp(word[k], ">", 1) == 0){
                        if (out_red.count > 0){
                            return -2 ;
                        }
                        else{
                            out_red.count ++ ;
                            out_red.index = k ;
                            word[k] = NULL ;
                        }

                    }
                    else if(strncmp(word[k], "|", 1) == 0)
                        {
                        if (pipe_red.count > 0){
                            return -2 ;
                        }
                        else{
                            pipe_red.count ++ ;
                            pipe_red.index = k ;
                            word[k] = NULL ;

                            }
                        }

                    }

                }

                return i ;                  // Desired case, return the number of split words

        }


void input_redirecting(){
    input_file=word[in_red.index +1];
    dup2(open(input_file, O_RDWR| O_CREAT, 0777) , 0);
}

void output_redirecting()
{
    output_file = word[out_red.index +1] ;
    dup2(open(output_file, O_RDWR| O_CREAT, 0777) , 1) ;
}

void command_exec()
{
    // If it's a CD command
    if (strncmp(word[0], "cd", 2) == 0)
    {
        if (chdir(word[1]) == -1) // Try the CD Command and check if it hasn't been Succeeded
        {   
            printf("%s%s%s%s\n", C_MAGNETA,word[1], C_RED, ": No such directory") ;
        }
        
    }
    
    
    // Not a CD Command
    else
    {
        pid_1 = fork() ;
        if ( pid_1 == 0)    // child process
            {   

            if (in_red.count == 1)
                {
                input_redirecting() ;
                }

            else if (out_red.count == 1)
                {
                output_redirecting() ;
                }
            execvp(word[0], word) ;     // execute the command
            printf("%s%s%s%s\n", C_RED, "Error in command: ", C_MAGNETA, word[0] );
            exit(1) ;
                
            }  

        else
            {         // parent process
            waitpid(pid_1, 0, 0);      // wait for the child process to finish and do nothing => continue the while loop
            }
    }
}

void piping_exec()
{
    pid_1 = fork() ;
    if(pid_1 ==0)
        {  //child
            pipe(piping_arr) ;
            pid_2 = fork() ;

            if (pid_2>0){     // parent of the child

                waitpid(pid_2, 0, 0) ;
                if(out_red.count>0)
                {
                    output_redirecting();
                }

                close(piping_arr[1]) ;
                dup2(piping_arr[0], STDIN_FILENO) ;
                execvp(word[pipe_red.index + 1], &word[pipe_red.index + 1]) ;
                printf("%s%s%s%s\n", C_RED, "Error in command: ", C_MAGNETA, word[pipe_red.index + 1] );
                exit(1) ;

            }
            else
                {     // child of the child
                if(in_red.count>0 )
                {
                    input_redirecting() ;
                }
                close(piping_arr[0]);
                dup2(piping_arr[1], STDOUT_FILENO) ;
                execvp(word[0], word) ;
                printf("%s%s%s%s\n",C_RED, "Error in command: ", C_MAGNETA, word[0] );
                exit(1) ;

                }
        }


    else
        {         //parent
        waitpid(pid_1, 0, 0) ;
        }

}



void resetting_flags()
{
    memset(entered_line, 0, MAX_CHAR) ;     // memset() is a standard function
    memset(edited_line, 0, 2*MAX_CHAR) ;    // Parameters(array to be set, setting value, length of array )
    memset(word, 0, MAX_WORDS) ;            // We are choosing setting value of 0 because we want to clear the arrays
    in_red.count = 0 ; out_red.count = 0 ; pipe_red.count = 0 ;

}



