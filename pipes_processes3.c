#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

/**
 * Executes the command "cat scores | grep <argument> | sort".
 * Uses three processes and two pipes to chain the commands.
 */

int main(int argc, char **argv)
{
  // Require at least one command line argument (the search term)
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <search_term>\n", argv[0]);
    return 1;
  }

  int pipefd1[2]; // Pipe between cat (P1) and grep (P2)
  int pipefd2[2]; // Pipe between grep (P2) and sort (P3)
  
  int pid1, pid2;

  char *cat_args[] = {"cat", "scores", NULL};
  char *grep_args[] = {"grep", argv[1], NULL}; // Use CLI argument
  char *sort_args[] = {"sort", NULL};

  // make the first pipe
  if (pipe(pipefd1) == -1) {
    perror("Pipe 1 failed");
    return 1;
  }

  pid1 = fork();

  if (pid1 == 0)
    {
      // P2 (Child) gets here to handle "grep <argument>"
      
      // Make the second pipe BEFORE forking the grandchild
      if (pipe(pipefd2) == -1) {
        perror("Pipe 2 failed");
        return 1;
      }
      
      pid2 = fork();
      
      if (pid2 == 0) 
        {
           // P3 (Child's Child / Grandchild) gets here to handle "sort"
           
           // Replace standard input with input part of second pipe
           dup2(pipefd2[0], 0);
           
           // Close all unused pipe ends in the grandchild
           close(pipefd2[1]); // Write end of pipe 2
           close(pipefd2[0]); // Read end of pipe 2 (after dup)
           
           // Grandchild also inherited pipe 1 from P2, so close those too
           close(pipefd1[0]);
           close(pipefd1[1]);
           
           // Execute sort
           execvp("sort", sort_args);
        }
      else 
        {
           // P2 (Child) continues here
           
           // Replace standard input with input part of first pipe (from cat)
           dup2(pipefd1[0], 0);
           
           // Replace standard output with output part of second pipe (to sort)
           dup2(pipefd2[1], 1);
           
           // Close all unused pipe ends in the child
           close(pipefd1[1]); // Write end of pipe 1
           close(pipefd1[0]); // Read end of pipe 1 (after dup)
           close(pipefd2[0]); // Read end of pipe 2
           close(pipefd2[1]); // Write end of pipe 2 (after dup)
           
           // Execute grep
           execvp("grep", grep_args);
        }
    }
  else
    {
      // P1 (Parent) gets here and handles "cat scores"

      // Replace standard output with output part of first pipe
      dup2(pipefd1[1], 1);

      // Close unused read half of first pipe
      close(pipefd1[0]);
      // Close original write half of pipe 1 (after dup)
      close(pipefd1[1]);

      // Execute cat
      execvp("cat", cat_args);
    }
    
  return 0;
}