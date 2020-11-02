#include <iostream>
#include "gstPipelineApp.h"
#include <unistd.h>

using namespace std;

void updateImageData(unsigned char * data, unsigned long len ){

   cout << "Writing nv12 to file\n";
   FILE *write_ptr;
   write_ptr = fopen("mytest.nv12","wb");  // w for write, b for binary
   fwrite(data,1,len,write_ptr); // write 10 bytes from our buffer
   fclose(write_ptr); // Close the file
  
   cout<< "Captured Data\n";  
}

// Main function for the program
int main() {
   int count = 0;
   unsigned char *data;
   FILE *fileptr;
   long filelen;
   GstreamerPipeline test;
   test.StartPipeline(&updateImageData);
   
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