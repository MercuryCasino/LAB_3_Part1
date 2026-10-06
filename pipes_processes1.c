// C program to demonstrate use of fork() and pipe() 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <string.h> 
#include <sys/wait.h> 
  
int main() 
{ 
    int fd1[2];  // Used to store two ends of first pipe 
    int fd2[2];  // Used to store two ends of second pipe 
  
    char fixed_str1[] = "howard.edu"; 
    char fixed_str2[] = "gobison.org";
    char input_str[100];
    pid_t p;
  
    if (pipe(fd1) == -1) 
    { 
        fprintf(stderr, "Pipe 1 Failed\n"); 
        return 1; 
    } 
    if (pipe(fd2) == -1) 
    { 
        fprintf(stderr, "Pipe 2 Failed\n"); 
        return 1; 
    } 
    
    // Parent prompts for the first string 
    printf("Input : "); 
    // %99s prevents buffer overflow by limiting input to 99 chars + null terminator
    if (scanf("%99s", input_str) != 1) {
        fprintf(stderr, "Input Failed\n");
        return 1;
    }
    
    p = fork(); 
  
    if (p < 0) 
    { 
        fprintf(stderr, "fork Failed\n"); 
        return 1; 
    } 
  
    // Parent process (P1)
    else if (p > 0) 
    { 
        close(fd1[0]);  // Close reading end of pipe 1
        close(fd2[1]);  // Close writing end of pipe 2
  
        // Write input string to pipe 1 and check for errors 
        if (write(fd1[1], input_str, strlen(input_str) + 1) == -1) {
            fprintf(stderr, "Parent write failed\n");
            return 1;
        }
        
        char final_str[200];
        // Read from pipe 2 and check for errors
        if (read(fd2[0], final_str, 200) == -1) {
            fprintf(stderr, "Parent read failed\n");
            return 1;
        }

        // P1 concatenates "gobison.org" to the string received from child
        strcat(final_str, fixed_str2);

        // Print the final string to stdout
        printf("Output : %s\n", final_str); 
        
        // Close remaining ends of the pipes 
        close(fd1[1]);  
        close(fd2[0]); 

        // Wait for child process to terminate 
        wait(NULL);
    } 
  
    // Child process (P2)
    else
    { 
        close(fd1[1]);  // Close writing end of pipe 1 
        close(fd2[0]);  // Close reading end of pipe 2
      
        char concat_str[200]; 
        // Read string from pipe 1 and check for errors
        if (read(fd1[0], concat_str, 200) == -1) {
            fprintf(stderr, "Child read failed\n");
            return 1;
        }

        // Concatenate the first fixed string ("howard.edu")
        strcat(concat_str, fixed_str1);
        
        // Print the intermediate concatenated string (matching parent spacing exactly)
        printf("Output : %s\n", concat_str); 

        // P2 prompts user for a second input 
        char second_input[100];
        printf("Input : "); 
        if (scanf("%99s", second_input) != 1) {
            fprintf(stderr, "Second Input Failed\n");
            return 1;
        }

        // P2 appends the second input to the string 
        strcat(concat_str, second_input);

        // P2 passes this new string back to P1 via the second pipe
        if (write(fd2[1], concat_str, strlen(concat_str) + 1) == -1) {
            fprintf(stderr, "Child write failed\n");
            return 1;
        }

        // Close remaining ends of the pipes 
        close(fd1[0]); 
        close(fd2[1]); 
  
        exit(0); 
    } 

    return 0; 
}