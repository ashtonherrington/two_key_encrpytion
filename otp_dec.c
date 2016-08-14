#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char *argv[]){

   //if there are not at least 2 arguments provide the user with an error
   if(argc <4){
      fprintf(stderr, "usage: otp_enc textfile keyfile port\n");
      exit(1);
   }
   
   //the third parameter is saved in the int variable socket     
   int socketNum = atoi(argv[3]);
  
   //the first argument is opened as a read only file
   int file1 = open(argv[1], O_RDONLY, 0);
   //a very large char buffer is declared to store keys up to 70000
   char buffer[150000];
   //the entire buffer is filled with null terminators
   bzero(buffer, 150000);

   //the number of characters read from file1 (argument1) are stored in file1chars variable
   int file1chars = read(file1, buffer, 150000);
   close(file1);
   
   //the second argument is opened as a read only file
   int file2 = open(argv[2], O_RDONLY, 0);
   //the buffer is re-initialized full of null terminators
   bzero(buffer, 150000);

   //the number of characters read from parameter 2 is stored in file2chars
   int file2chars = read(file2, buffer, 150000);
   close(file2);

   //if the number of characters read from parameter2 is less than parameter1 
   if(file2chars < file1chars){
      //the error is printed and the program exits with value 1
      fprintf(stderr, "Error: key '%s' is too short\n", argv[2]);
      exit(1);
   }

   //varible declared to store the socket identifier
   int sockfd = socket(AF_INET, SOCK_STREAM, 0);
   //if sockfd is -1 inform user of failure and exit
   if(sockfd == -1){
      fprintf(stderr, "Error opening socket\n");
      exit(1);
   }
  
   //set the size of the packet to 15000
   int sendSize = 150000;
   int res = 0;

   //an attempt to change socket options, I dont think this worked, but I dont want to delete it from working code
   res = setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sendSize, sizeof(sendSize));

   //the server is set equal to localhost, ie the same machine
   struct hostent *server = gethostbyname("localhost");
   //if the function does not resolve exit gracefully
   if(server == NULL){
      printf("ERROR, no such host\n");
      exit(0);
   }

   //declare a server address structure and fill in its components
   struct sockaddr_in serv_addr;
   //fill the server address with null characters
   bzero((char *) &serv_addr, sizeof(serv_addr)); 
   //set the family to AF_INET addressing mode
   serv_addr.sin_family = AF_INET;
   //set the address of the server address structure equal to that of the servers
   bcopy((char *)server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
   //set the port to the Big-Endian conversion of the port variable passed to the program
   serv_addr.sin_port = htons(socketNum);

   //now attempt to make the connection to the socket 
   if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1){
      fprintf(stderr, "Error connecting here\n");
      exit(1);
   }
   
   //variable declared to store the value if the write succeeds to the socket
   int writeStatus, readStatus;
   
   //declare a char array to store the size of the packet
   char numberToRead[10];
   //fill the array with null terminators
   bzero(numberToRead, 10);
   //now use sprintf to convert the number in file1chars into characters
   sprintf(numberToRead, "%d", file1chars);

   
   int n, len;
   //cycle through contents of the numberToread array
   for(n = 0; n < 10; n++){
      //the first null terminator is now converted into a newline character
      if(numberToRead[n] == '\0'){
         numberToRead[n] = '~';
	 numberToRead[n+1] = '\n';
	 //length is set equal to 1 more than the current value of n
	 
	 len = n + 2;
	 break;
      }
   }
   
   //a single line with the packet size number is written to the socket
   int firstWrite = write(sockfd, numberToRead, len);

   //the buffer is again cleared by filling it with null terminators
   bzero(buffer, 150000);
   
   //file objects opened using argv 1 and 2 as the name parameters
   FILE * fp;
   fp = fopen(argv[2], "r");
   FILE * fp1;
   fp1 = fopen(argv[1], "r");

   //the buffer is filled with the contents of the second parameter
   fgets(buffer, 150000, fp);
   
   //bytes written is initialized to 0
   int bytesWritten = 0;
   //bytes to write is going to be equal to the number of characters in file1 (so that they provide matching input)
   int bytesToWrite = file1chars;

   //as long as the bytesWritten is below bytes to write the loop iterates
   while(bytesWritten != bytesToWrite){
   
      //a number is declared to store the amount of bytes written this iteration
      int writtenThisTime;
   
      //this loop occurs as long as written tims time is not in error
      do{
	 //write to the socket
	 writtenThisTime = write(sockfd, buffer, (bytesToWrite- bytesWritten));   
      }while((writtenThisTime == -1) && (errno == EINTR));
   
      //if there is a serious error with the write exits with status of 1
      if(writtenThisTime == -1){
         exit(1);
      
      }
      //written this time value is added to the bytes written accumulator
      bytesWritten += writtenThisTime; 
   }
   
   //the buffer is filled with null terminators
   bzero(buffer, 150000);

   //the buffer is now filled with the contents of the first argument
   fgets(buffer, 150000, fp1);
   
   //bytes written is again set to 0 to start the process over
   bytesWritten = 0;
   //again bytes to write is set to file1chars (so if the key is longer than file, they write same amount)
   bytesToWrite = file1chars;

   //as long as bytesWritten is below bytes to write the loop iterates
   while(bytesWritten != bytesToWrite){
   
      //a number is declared to store the amount of bytes written this iteration
      int writtenThisTime;
   
      //this loop occurs as long as written times time is not in error
      do{
	 //write to the socket
         writtenThisTime = write(sockfd, buffer, (bytesToWrite- bytesWritten));   
      }while((writtenThisTime == -1) && (errno == EINTR));
   
      //if there is a serious error with the write exists with status of 1
      if(writtenThisTime == -1){
         exit(1);
      
      }
      //written this time is added to the bytes written accumulator
      bytesWritten += writtenThisTime; 
   }

   //if writestatus is <0 an error appears, I believe this is antiquated unneeded code, leaving it for now
   if(writeStatus < 0){
      fprintf(stderr, "Error writing to socket\n");
      exit(1);
   }

   //again clear the buffer before accepting the information back from the socket
   bzero(buffer, 150000);
  
   //both fo the file pointers are closed as they have served their purpose
   fclose(fp);
   fclose(fp1);

   //readstatus is set equal to the return value from the socket returning information  
   readStatus = read(sockfd, buffer, (bytesToWrite-1));
   
   //if the status is negative (-1), then error is reported and program closes
   if(readStatus < 0){
      fprintf(stderr, "Error writing to socket\n");
      exit(1);
   }

   //the contents in the buffer are printed to the screen (or to the re-directed file)
   printf("%s\n", buffer);
   //the socket is closed
   close(sockfd); 

   //program ends
   exit(0);
}
