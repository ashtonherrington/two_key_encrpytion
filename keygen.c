#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>


int main(int argc, char *argv[]){


   //seed the random number generator
   srand(time(NULL));

   //if there are not at least 2 arguments provide the user with an error
   if(argc <2){
      fprintf(stderr, "Error input length unspecified\n");
      exit(1);
   }

   //set length equal to the value in the first parameter + 1
   int length = atoi(argv[1]) + 1;

   //declare an array of characters the size of the first parameter plus 1
   char rSeq[length];
   
   //randomFill(&rSeq[length]);
   
   int i = 0;
   int modulus;

   while(i < (length-1)){
      modulus = rand() % 27;
      if(modulus == 26){
        rSeq[i] = ' ';
      }
      else{
        rSeq[i] = modulus + 'A';
      }
      i++; 
   }
   rSeq[i] = '\n';
   rSeq[i+1] = '\0';

   printf("%s", rSeq);

}

