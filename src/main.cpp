#include <iostream>
#include "gstTestApp.h"
#include <unistd.h>

using namespace std;

// Main function for the program
int main() {
   int count = 0;
   unsigned char *data;
   FILE *fileptr;
   long filelen;
   GstreamerTest test;
   test.StartPipeline();
   
   while(test.isRunning()){
      usleep(1000000);
      count++;
      
      // push a buffer test
      if(count == 5){
         // assumes running from build directory
         fileptr = fopen("../images/cap-20201025151646.jpg", "rb");    // Open the file in binary mode
         fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
         filelen = ftell(fileptr);             // Get the current byte offset in the file
         rewind(fileptr);                      // Jump back to the beginning of the file
         data = (unsigned char *)malloc(filelen);
         fread(data, filelen, 1, fileptr); // Read in the entire file
         fclose(fileptr); // Close the file
         test.PushBuffer(data, filelen);
         free(data);
         data = NULL;
         
      }

      if(count>20)
         break;
   }
   test.StopPipeline();
   return 0;
}