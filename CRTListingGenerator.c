/*
 * NAME: Alana Ruth
 *
 * UNIX LOGIN ID: AR463521
 *
 * DISCUSSION SECTION: Mon. 10:25-11:20
 *
 * Takes input from the command line in the form flag, inputfile, outputfile,
 * and produces the correct output as specified by the flag, either the
 * listing of the MAL program, the cross reference table for the MAL program,
 * or both.
 *
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#define TRUE 1
#define FALSE 0

/* define a struct for help with the use column */
typedef struct node{
  int lineNum;
  struct node * next;
} node_type;

/* function prototypes */
void makeListing(FILE * inp, FILE * out);
void makeCRT(FILE * inp, FILE * out, char inpName[]);
void compareToks(node_type *h, char *t, char *label, int lineCount);
int isBlank(char line[80]);
int hasLongLine(FILE * inp);

int main(int argc, char **argv){

  /* check to make sure there are 4 parameters on the command line */
  if (argc != 4){
    fprintf(stderr, "%s", "Wrong number of parameters on the command line.\n");
    return 0;
  }
  /* file pointers for input and output file */
  FILE *input, *output;

  /* open the input file for reading */
  if ((input = fopen(argv[2], "r")) == NULL){
    /* open failed */
    fprintf(stderr, "%s%s%s", "Could not open file ", argv[2], " for reading.\n");
    return 0;
  }

  /* open the output file for writing */
  if ((output = fopen(argv[3], "w")) == NULL){
    /* open failed */
    fprintf(stderr, "%s%s%s", "Could not open file ", argv[3], " for writing.\n");
    return 0;
  }

  /* compare the first parameter on the command line to -l, -c, and -c */
  if (strcmp(argv[1], "-l") == 0){
    if(hasLongLine(input)){
      /* if the file contains a line that is over 80 chars */
      return 0;
    }
    rewind(input);

    makeListing(input, output);
  }
  else if (strcmp(argv[1], "-c") == 0){
    if(hasLongLine(input)){
      /* if the file contains a line that is over 80 chars */
      return 0;
    }
    rewind(input);

    makeCRT(input, output, argv[2]);
  }
  else if (strcmp(argv[1], "-b") == 0){
    if(hasLongLine(input)){
      /* if the file contains a line that is over 80 chars */
      return 0;
    }
    rewind(input);

    makeListing(input, output);
    fputs("\n\n", output);

    /* rewind the file to the beginning so it reads again */
    rewind(input);

    makeCRT(input, output, argv[2]);
  }
  /* if none of the above, print an error message and stop */
  else{
    fprintf(stderr, "%s", "Invalid flag.\n");
    return 0;
  }


  if (fclose(input) == EOF){
    /* error in closing input file */
    fprintf(stderr, "%s", "Error in closing input file.\n");
  }
  if (fclose(output) == EOF){
    /* error in closing output file */
    fprintf(stderr, "%s", "Error in closing output file.\n");
  }

  return 0;
}

/* function to write the listing to the output file */
void makeListing(FILE * inp, FILE * out){
  char line[80]; /* max length of a line is 80 */
  int count = 1; /* the line counter starts at 1 */
  int i;         /* index for the line of chars */

  /* while loop iterates while there are still lines in the file */
  while (fgets(line, 81, inp)){
    i = 0;

    /* if the line is not all white space */
    if (!isBlank(line)){
      /* prints the line count and the line, then increments the line counter */
      fprintf(out, "%d\t", count);
      fputs(line, out);
      count++;
    }
    else{
      /* only prints the line of white spaces */
      fputs(line, out);
    }
  }
  return;
}

/* function to write the Cross Reference Table to the output file */
void makeCRT(FILE * inp, FILE * out, char inpName[]){

  /* prints the header for the Cross Reference Table */
  fputs("Cross Reference Table\n\n", out);
  fputs("\tIdentifier\tDefinition\tUse\n\n", out);

  /* define local variables */
  char line[80], line2[80], *token, *savePtr;
  int hasLabel;
  char label[11];
  int i, j, k;
  int count = 0;
  int count2;
  FILE * inp2;
  node_type* uses[100];
  node_type* h;

  /* while loop iterates while there are still lines in the file */
  while (fgets(line, 81, inp)){
    /* re-initialize all local variables */
    hasLabel = FALSE;
    i = 0; j = 0; k = 0;
    h = NULL;
    count++; /* increment line counter each time */

    /* makes each index of label NULL */
    while (j < 11){
      label[j] = '\0';
      j++;
    }

    /* checks if the line has a label or if it is a comment line */
    if (!isspace(line[0]) && line[0] != '#'){
      hasLabel = TRUE;
    }

    /* if the line is blank, decrement the counter */
    if(isBlank(line)){
      count--;
    }

    if (hasLabel){
      /* store the characters of the line into label up until the ':' */
      while (line[i]!= ':'){
        label[i] = line[i];
        i++;
      }

      /* print out the results in the first two columns */
      fputs("\t", out);
      fputs(label, out);
      fprintf(out, "\t\t%d\t\t", count);

      /* allocate memory for the head */
      if ((uses[k] = (node_type*)malloc(sizeof(node_type))) == NULL){
        fprintf(stderr, "%s", "Node allocation failed.\n");
        return;
      }

      /* initialize the head to be the line where the label is defined */
      uses[k]->lineNum = count;
      uses[k]->next = NULL;
      h = uses[k];

      /* open the file for reading again, but in inp2 */
      if((inp2 = fopen(inpName, "r")) == NULL){
        /* open falied */
        fprintf(stderr, "%s%s%s", "Could not open file ", inpName, " for reading again.\n");
      }

      /* inner while loop reads through inp2 file from the beginnning
       * and checks for all instances of the identifier to add
       * to the uses column */
      count2 = 0;
      while (fgets(line2, 81, inp2)){
        count2++;

        /* if the line is blank, decrement the counter */
        if(isBlank(line2)){
          count2--;
        }

        /* parse through the line, comparing the tokens to the labels each time */
        token = savePtr = NULL;
        token = strtok_r(line2, "\t \n,", &savePtr);
        while(token != NULL && token[0] != '#'){
          compareToks(h, token, label, count2);
          token = strtok_r(NULL, "\t \n,", &savePtr);
        }
      }

      /* close inp2 */
      if (fclose(inp2) == EOF){
        /* error in closing inp2 file */
        fprintf(stderr, "%s", "Error in closing input file.\n");
      }

      /* print the results */
      while(h->next != NULL){
        h = h->next;
        fprintf(out, "%d ", h->lineNum);
      }

      fputs("\n", out);
      k++; /* go to the next element in the node_type uses array, */
    }
  }
  return;
}

/* function compares the token to the label and adds the lineNum to the uses array */
void compareToks(node_type *h, char *t, char *label, int lineCount){
  /* local variables */
  node_type *newNode = NULL;
  node_type *temp = h;

  /* while loop puts temp pointing to the last element in the linked list */
  while (temp->next != NULL){
    temp = temp->next;
  }

  /* compare the label to the token and make sure that line number isn't already in the uses array */
  if(strcmp(t, label) == 0 && temp->lineNum != lineCount){
    /* allocate memory for the node */
    if ((newNode = (node_type*)malloc(sizeof(node_type))) == NULL){
      fprintf(stderr, "Node allocation failed.\n");
      return;
    }

    /* add the newNode to the end of the linked list */
    newNode->lineNum = lineCount;
    newNode->next = NULL;
    temp->next = newNode;
  }
  return;
}


/* function returns true if the line contains all whitespace, false otherwise */
int isBlank(char line[80]){
  int i = 0;
  int isBlank = TRUE;

  while (line[i] != '\n'){
    if (!isspace(line[i])){
      /* if there is a non white space character, isBlank is false */
      isBlank = FALSE;
    }
    i++;
  }

  return isBlank;
}

/* function to check whether a line is longer than 80 characters */
int hasLongLine(FILE * inp){
  int charCount = 0;
  int c;
  /* reads the file char by char, storing each char in c */
  while (TRUE){
    c = getc(inp);
    charCount++;

    if(c == EOF)
      break;

    else if (c == '\n'){
      if (charCount > 80){
        /* if the line is longer than 80 characters, print error message and return true */
        fprintf(stderr, "%s", "Error: File contains line with more than 80 characters.\n");
        return TRUE;
      }
      charCount = 0;
    }
  }
  return FALSE;
}

