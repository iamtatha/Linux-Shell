/*HEADERS AND MACRO*/

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define MAX_BG_PROCESS 64




/*GLOBAL VARIABLES*/

int bg;
int fc;
pid_t boss;
pid_t group;
int fg;
int checkcd;



/*TOKENIZATION OF INPUT COMMAND*/


char **tokenize(char *line)
{
  checkcd = 1;
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0, countspace = 0;

  for (i = 0; i < strlen(line); i++)
  {

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t')
    {
      if (readChar == ' ')
      {
        countspace++;
        if (countspace == 2)
          checkcd = 0;
      }
      token[tokenIndex] = '\0';
      if (tokenIndex != 0)
      {
        tokens[tokenNo] = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
        strcpy(tokens[tokenNo++], token);
        tokenIndex = 0;
      }
    }
    else if (readChar != '&')
    {
      token[tokenIndex++] = readChar;
    }
  }

  free(token);
  tokens[tokenNo] = NULL;
  return tokens;
}




/*HANDLER FUNCTIONS*/


void handler(int sig)
{
  kill(bg, SIGKILL);
  printf("Shell: Background process finished\n");
  kill(fc, SIGKILL);
  while (waitpid(-1, NULL, WNOHANG) != 0);
  if (getpid() != boss)
    kill(getppid(), 24);
  exit(0);
}


void handlerc(int sig)
{
  killpg(fg, SIGINT);
  waitpid(fg, NULL, 0);
}

void handlerk(int sig)
{
  wait(NULL);
  exit(0);
}




/*MAIN FUNCTION*/


int main(int argc, char* argv[])
{

  //SIGNAL HANDLING

  signal(25, handlerk);
  signal(24, handler);
  signal(SIGINT, handlerc);

  
  //FLAGS

  boss = getpid();
  group = -1;
  fg = -1;


  //INITIALISATION

  char line[MAX_INPUT_SIZE];
  char **tokens;
  int i, bgcount = 0;
  pid_t arr[MAX_BG_PROCESS];


  // START SHELL

  while (1)
  {

    //PRINT PROMPT AND TAKE INPUT

    bzero(line, sizeof(line));
    printf("$ ");
    scanf("%[^\n]", line);
    getchar();

    line[strlen(line)] = '\n';
    tokens = tokenize(line);


    // CHECK IF THE COMMAND IS CD COMMAND

    char s1[] = "cd";
    int flagcd = 1; 
    for (int j=0; j<2; j++)
    {
      if ((line[j] != s1[j]) || (flagcd == 0))
      {
        flagcd = 0;
        break;
      }
    }


    // CHECK IF THE COMMAND IS EXIT COMMAND

    char s2[] = "exit";
    int flagexit = 1; 
    for (int j=0; j<4; j++)
    {
      if ((line[j] != s2[j]) || (flagexit == 0))
      {
        flagexit = 0;
        break;
      }
    }


    // IF IT IS CD COMMAND, IMPLEMENNT CD
    //printf("%d %d", flagcd, checkcd);
    if ((flagcd == 1) && (checkcd == 0))
    {
      printf("Incorrect Command\n");
    }
    else if (flagcd == 1)
    {
      chdir(tokens[1]);
    }


    // IF IT IS EXIT COMMAND AND THERE IS >0 BACKGROUND PROCESSES

    else if ((flagexit == 1) && (getpid() != boss))
    {
      kill(getppid(), 24);
    }


    // IF IT IS EXIT COMMAND AND THERE IS 0 BACKGROUND PROCESSES

    else if ((flagexit == 1) && (getpid() == boss))
    {
      exit(0);
    }


    // IF IT IS GENUINE FOREGROUND COMMAND [NEITHER CD NOR EXIT]

    else if (line[strlen(line) - 2] != '&')
    {
      fg = fork();

      if (fg < 0)
      {
        fprintf(stderr, "%s\n", "Unable to create child process!!\n");
      }
      else if (fg == 0)
      {
        execvp(tokens[0], tokens);
        printf("Command execution failed from %d!!\n", getpid());
        _exit(1);
      }
      else
      {
        int wc = wait(NULL);
      }
    }


    // IF IT IS BACKGROUND COMMAND

    else
    {
      fc = fork();

      if (fc < 0)
      {
        fprintf(stderr, "%s\n", "Unable to create child process!!\n");
      }

      else if (fc != 0)
      {
        bg = fork();

        if (bg < 0)
        {
          fprintf(stderr, "%s\n", "Unable to create child process!!\n");
        }

        else if (bg == 0)
        {
          setpgrp();
          execvp(tokens[0], tokens);
          printf("Command execution failed!!\n");
          _exit(1);
        }

        else
        {
          int wc = waitpid(bg, NULL, 0);
          printf("Shell: Background process finished\n");
          int k = kill(fc, 25);
          while (wait(NULL) != -1)
            ;
          continue;
        }
      }

      else
      {
        continue;
      }
    }


    // FREE UP TOKEN SPACE

    for (i = 0; tokens[i] != NULL; i++)
    {
      free(tokens[i]);
    }
    free(tokens);
  }
}