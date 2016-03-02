// Chris O'Brien
// CS-311 (Concepts of Operating Systems)
// 9/15/15

#include <stdio.h>  // acts like "import java.io.*;"
#include <stdlib.h> // for library function prototypes
#include <string.h> // for manipulating strings
#include <signal.h> // for SIGINT?
#include <dirent.h> // necessary for implementing RLS command
#define MAX_LINE 80 // 80 characters per line on input should be enough.

// declare functions before they're called
void getInput (int index, char inputBuffer[]); // put historyBuffer[index] into inputBuffer
int getLength (int index);
int parseNum (char chars[]);
void recursiveLS(char * path, int numSpaces); // recursively lists all files in a directory

// declare global variables
char historyBuffer[10][MAX_LINE]; // records the last 10 commands
int cmdCount, // counts the number of commands
  i2, // for testing
  rls; // flag that indicates whether "RLS..." has been entered

void setup(char inputBuffer[], char *args[],int *flag,int length) {
  /* Added length as a parameter so that I could do some work in main().
  int i,      // loop index for accessing inputBuffer array
    start,  // index where beginning of next command parameter is
    ct,     // index of where to place the next parameter into args[]
    index, // used in loops
    charPos; // indicates a character's position in a string
  char * charPtr; // used to locate '/' at the beginning of the path
  //dirent curFile; // pointer to the current file
  //struct dirent *readdir(DIR *curFile);
  DIR  * dirPtr; // pointer to a directory
  struct dirent *curFile; // pointer to the current file being printed
  ct = 0; // initialize counter
  //length = read(0,inputBuffer,MAX_LINE);  

  start = -1;
  if (length == 0)
    exit(0);            /* ^d was entered, end of commands */
  if (length < 0){
    perror("error reading the command");
    exit(-1);           /* terminate with error code of -1 */
  }
  
  for (i=0;i<length;i++){ /* examine every character in the inputBuffer */
    switch (inputBuffer[i]){
    case ' ':
    case '\t' :               /* argument separators */
      if(start != -1){
	args[ct] = &inputBuffer[start];    /* set up pointer */
	ct++;
      }
      inputBuffer[i] = '\0'; /* put in a null to end the argument */
      start = -1;
      break;
      
    case '\n':                 /* same comment as for ' ','\t' */
      if (start != -1){
	args[ct] = &inputBuffer[start];     
	ct++;
      }
      inputBuffer[i] = '\0';
      start = -1;
      /* tried to print input after it was processed by setup
	 for (index=0; index <= ct; index++){ // print input
	 printf(args[index]);
	 printf("\n");
	 } */
      args[ct] = NULL; /* no more arguments to this command */
      break;
      
    default :             /* some other character */
      if (start == -1) start = i;
      if (inputBuffer[i] == '&'){
	*flag = 1;
	inputBuffer[i] = '\0';
      }
    } /* end of switch */
  } /* end of for */
  args[ct] = NULL; 
  if(rls){
    i2 = 0;
    charPtr = strchr(args[1], '.');
    charPos = charPtr-args[1];
    if (charPos == 1)
      printf("ERROR: Can't follow '.' and '..' paths");
    charPtr = strchr(args[1], '/');
    charPos = charPtr-args[1];
    if(charPos == 0){
      printf("Listing Path: %s {\n", args[1]);
      recursiveLS (args[1], 2);
    }
    else printf("ERROR: path must start with '/'!\n");
  }
} /* end of setup routine */

void recursiveLS(char * path, int numSpaces){
  int i; // counter
  char *name, *temp;
  DIR  *dirPtr; // pointer to a directory
  struct dirent *curFile; // pointer to the current file being printed
  if ((dirPtr = opendir(path)) == NULL)
    perror("Error: Couldn't open file ");
  else {
    while ((curFile = readdir(dirPtr)) != NULL){
      i2++;
      if(i2>5) break;
      name = curFile->d_name;
      for (i=0; i < numSpaces; i++) printf(" "); // print spaces for readability
      printf ("%s\n", name);
    } // end while
    for (i=0; i < numSpaces-2; i++) printf(" "); // print spaces for readability
    printf("} // done listing %s\n", path);
  } // end else
  closedir(dirPtr);
  if ((dirPtr = opendir(path)) == NULL)
    perror("Error: Couldn't open file ");
  else {
    while ((curFile = readdir(dirPtr)) != NULL){
      name = curFile->d_name;
      if (curFile->d_type == 4 && strcmp(name, ".") && strcmp(name, "..")) { // avoid infinite loops
        printf("\n");
        for (i=0; i < numSpaces; i++) printf(" "); // print spaces for readability
        temp = (char *) malloc((sizeof(char)) * 255);
        strcpy(temp, path);
        printf("Path length: %d", strlen(path));
        if(strlen(path) == 1) strcat(temp, "/");
        strcat(temp, name); // append name to existing path
        printf ("Now listing path: %s {\n", temp);
        recursiveLS(temp, numSpaces+2);
      } // end if
    } // end while
  } // end else
  closedir(dirPtr); // close directory stream
}

void shellHandler() {
  if (cmdCount == 0) {
    printf("No commands yet!\n");
    return;
  }
  printf("Past commands (10 max):\n");
  int i, j; // indices
  if (cmdCount > 10) {
    for (i=9; i >= 0; i--) { // go through the entire array
	    printf("#%d: ", cmdCount-i);
	    for (j=0; historyBuffer[9-i][j] != '\n'; j++) { // go through each row (one less because history starts at 0)
	      printf("%c", historyBuffer[9-i][j]);
	    } // end inner for
	    printf("\n");
    } // end outer for
  } // end if
  else { // <10 commands
    for (i=0; i < cmdCount; i++) { // go through the entire array
	    printf("Command #%d: ", i+1);
	    for (j=0; historyBuffer[i][j] != '\n'; j++) { // go through each row
	      printf("%c", historyBuffer[i][j]);
	    } // end inner for
	    printf("\n");
    } // end outer for
  } // end inner else
  printf("\nInput Command:\n");
} // end method

int main (void) {
  // DECLARATIONS
  int flag;             /* equals 1 if a command is followed by '&' */
  char *args[MAX_LINE/2+1]; /* command line (of 80) must have < 40 arguments */ 
  int child,            /* process id of the child process */
    status,          /* result from execvp system call*/
    length, // size of inputBuffer[]
    temp, // for char->int conversion
    i, j, // indices
    illegal, // boolean for illegal history call
    historyNum, // stores cmd called in historyBuffer
    offset, // difference between historyNum and historyIndex
    historyIndex; // stores index (0-9) corresponding to command #historyNum
  char inputBuffer[MAX_LINE], test;   /* buffer to hold the command entered */
  
  // INITIALIZATIONS
  cmdCount = 0;
  illegal = 0;
  signal(SIGINT, shellHandler);

  while (1) {            /* Program terminates normally inside setup */
    flag = 0;
    rls = 0;
    printf("Input Command:\n"); // read input
    length = read(0,inputBuffer,MAX_LINE); 
    
    if (inputBuffer[0] == '!') { // if it's a call to historyBuffer (handle "!..."
      // validate input
      if (length > 2) {
	// determine number after '!' (cmd to run)
	if (length == 3 && inputBuffer[1] == '!') { // run last command
	  if(cmdCount == 0) {
	    printf("ERROR: You haven't put in any commands yet! Try again.\n");
	    continue;
	  }
	  historyNum = cmdCount;
	}
	else historyNum = parseNum(inputBuffer);
	historyNum--; // because array starts at 0, not 1
	if (historyNum < 0 || historyNum < cmdCount-11 || historyNum > cmdCount-1){
	  printf("ERROR: command called (#%d) is out of historyBuffer's range! Please try again.\n", historyNum+1); // +1 to account for offset
	  illegal = 1;
	  continue; // restart while loop
	}
	// find index in historyBuffer corresponding to historyNum
	if (cmdCount > 10){
	  offset = cmdCount-10;
	  historyIndex = historyNum - offset;
	} else historyIndex = historyNum;
	getInput(historyIndex, inputBuffer);
	temp = length;
	length = getLength(historyIndex); // update length
	for(i=0; i<length; i++) printf("%c", inputBuffer[i]);
	printf("\n");
      }
    } // end if for '!...'

    // add command to historyBuffer
    if (cmdCount < 10) {
      for (i=0; i<length; i++){
	     historyBuffer[cmdCount][i] = inputBuffer[i];
      } // end for
      cmdCount++;
    } // end if
    else { // >10 commands
      for (i=1; i < 10; i++) { // make room for new command (delete row 0)
	     for (j=0; historyBuffer[i][j] != '\n'; j++){
	       historyBuffer[i-1][j] = historyBuffer[i][j];
	     }
	     historyBuffer[i-1][j] = '\n'; // append '\n'
      } // end for
      for (i=0; i<length; i++)
	historyBuffer[9][i] = inputBuffer[i];
      cmdCount++;
    } // end else

    if (inputBuffer[0] == 'R' && inputBuffer[1] == 'L' && inputBuffer[2] == 'S') // handle RLS
      rls = 1; // flip RLS flag
    printf("Length = %d\n", length);

    setup(inputBuffer,args,&flag,length);       /* get next command */ // send command to setup()

    if ((illegal == 0 || illegal == 1) && rls == 0) { // check for illegal history call, temporarily nullified
      child = fork();          /* creates a duplicate process! */
      switch (child) {
	
      case -1: 
	perror("could not fork the process");
	break; /* perror is a library routine that displays a system
		  error message, according to the value of the system
		  variable "errno" which will be set during a function 
		  (like fork) that was unable to successfully
		  complete its task. */
	
      case 0: /* here is the child process */
	status = execvp(args[0],args);
	if (status != 0){
	  perror("Error in execvp!");
	  exit(-2); /* terminate this process with error code -2 */
	}
	break;
	
      default : 
	if(flag==0) /* handle parent, wait for child */
	  while (child != wait((int *) 0)) ;
	
      } // end switch
    } // end if
  } // end big while
} // end main

void getInput (int index, char inputBuffer[]){ // put historyBuffer[index] into inputBuffer
  int i; // counter
  if (cmdCount < 11) {
    for (i = 0; historyBuffer[index][i] != '\n'; i++) {
      inputBuffer[i] = historyBuffer[index][i];
    } // end for
    if (historyBuffer[index][i] == '\n') inputBuffer[i] = '\n'; // append '\n'
  } // end if
  else { // >10 commands
    for (i = 0; historyBuffer[index][i] != '\n'; i++)
      inputBuffer[i] = historyBuffer[index][i];
    if (historyBuffer[index][i] == '\n') inputBuffer[i] = '\n'; // append '\n'
  }
} // end getInput

int getLength (int index) { 
  int counter;
  for (counter = 0; historyBuffer[index][counter] != '\n'; counter++) ; // count through row
  counter++; // append '\n'
  return counter;
} // end getLength

int parseNum (char chars[]) {
  int index, result, temp;
  result = 0;
  if (chars[0] != '!') {
    printf("ERROR: Why are you sending that string to parseNum()?\n");
    return -1; // error value
  }
  for (index=1; chars[index] != '\n'; index++) {
    result *= 10; // make room for new value
    temp = chars[index] - '0'; // convert new value
    result += temp; // add new value
  }
  return result;
}
