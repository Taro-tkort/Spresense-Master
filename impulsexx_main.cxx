// Included Files

//#include <nuttx/config.h>

//#include <cstdio>
//#include <debug.h>


//some standars includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <asmp/mpshm.h>
#include <arch/chip/pm.h>
#include <arch/board/board.h>
#include <sys/stat.h>

//math
#include <cmath>

//custom includes
#include "audio_player_tool.h"
#include "wawgen.h"


//audio
#include <audio/audio_high_level_api.h>
#include <audio/audio_player_api.h>
#include <memutils/simple_fifo/CMN_SimpleFifo.h>
#include <memutils/memory_manager/MemHandle.h>
#include <memutils/message/Message.h>
#include "include/msgq_id.h"
#include "include/mem_layout.h"
#include "include/memory_layout.h"
#include "include/msgq_pool.h"
#include "include/pool_layout.h"
#include "include/fixed_fence.h"
#include <audio/audio_message_types.h>

#include <nuttx/config.h>
#include <nuttx/compiler.h>



//defines

//***************************************************************************
// Public Functions
//***************************************************************************

/****************************************************************************
 * Name: helloxx_main
 ****************************************************************************/

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////



// The Native C part
extern "C" {
  //function prototypes
  bool uart_comms();

  int main(int argc, FAR char *argv[]){
    printf("Fucks sake, It works!!\n");

    if(!uart_comms()){
      printf("\nuart init failed or uart was exited...\n quitting program\n");
      return 0;
    } //END OF MAIN
  }


  /* FUNCTIONS */

  // char to string conversion //
  bool stringCompare(char *a, char *b){
    int sizea = sizeof(a);
    for(int i = 0; i <= sizea-1; i++){
      if(a[i] != b[i]){
        return false;
      }
    }
    return true;
  }

  //example of opening files
  bool file_config(){
    FILE *fp; //pointer to a new file
    printf("attempting to open file\n");
    fp = fopen("/mnt/sd0/AUDIO/test2.txt", "ab");
    if (fp == NULL){
      return 0;
    }
    fprintf(fp, "");

    fclose(fp);

    fp = fopen("/mnt/sd0/AUDIO/test2.txt", "rb");
    printf("attempting reading\n file content: ");
    int c;
    while(1) {
      c = fgetc(fp);
      if( feof(fp) ) { 
         break ;
      }
      printf("%c", c);
   }
    printf("\nclosing file...\n\n");
    fclose(fp);

    return 1;
  }

  //this only tests the generation of the waw file with the header
  bool waw_testbench(){
    audioData test;
    if(!generate_waw_file(test)){
      return 0;
    }
    printf("file should now be generated\nReturning to uart\n\n");
    return 1;
  }

  //this only tests the sine oscillator
  bool sine_oscillator_test(){
    float sample;
    float time = 0;
    audioData temp;
    temp.amplitude = 2;
    temp.angle = 0;
    temp.frequency = 1;
    temp.bitDepth = 8;
    temp.sampleRate = 50;
    temp.duration = 1;
    float dur = temp.duration;
    float SR = temp.sampleRate;
    float tau = 1/SR;
    printf("Starting sine wave generation of a wave with the following properties\n");
    printf("Ampliute: %f, Freq: %f, duration: %f, SR: %i Tau: %f\n", temp.amplitude, temp.frequency, dur, temp.sampleRate, tau);

    while(time <= dur){
      sample = sine_oscillator(temp, time);
      printf("Sample: %f, Time: %f\n", sample, time);
      time += tau;
    }
    return 1;
  }

  bool complete_waw_test(){
    float sample;
    float time = 0;
    audioData temp;
    temp.amplitude = 2;
    temp.angle = 0;
    temp.frequency = 1;
    temp.bitDepth = 8;
    temp.sampleRate = 50;
    temp.duration = 1;
    float dur = temp.duration;
    float SR = temp.sampleRate;
    float tau = 1/SR;
    printf("Starting sine wave generation of a wave with the following properties\n");
    printf("Ampliute: %f, Freq: %f, duration: %f, SR: %i Tau: %f\n", temp.amplitude, temp.frequency, dur, temp.sampleRate, tau);
  }

  //UART communications //
  bool uart_comms(){
    //test code for uart
    printf("initializing uart\n");
    int fd;
    char buffer;
    char buffer_aux[256] = {};
    char quit_buf[4] = {'q','u','i','t'};
    char play_buf[4] = {'p','l','a','y'};
    char file_buf[4] = {'f','i','l','e'};
    char help_buf[4] = {'h','e','l','p'};
    //test buffers
    char test_buf[4] = {'t','e','s','t'};
    char tsin_buf[4] = {'t','s','i','n'};
    char twaw_buf[4] = {'t','w','a','w'};
    int ret;
    int i = 0;

    fd = open("/dev/ttyS0", O_RDWR); //Opening the uart with read and write permission
    if (fd < 0) {
      printf("Error UART");
    }
    printf("Serial communication open\n\ntype help to get a list of availible commands\n\n");
    //This loop will check if there are any data available
    //That data it will be save in a buffer and when we detect a return
    //The data it will be send again
    while (1) {
      ret = read(fd, &buffer, sizeof(buffer));//It return only a char
      if (ret > 0) {
        buffer_aux[i] = buffer;//Saving in the auxilary buffer        
        i++;
        printf("%c", buffer);
        //printf("%i", buffer);

        if (buffer == 127) {
          buffer_aux[i] = NULL;
          buffer_aux[i-1] = NULL;
          i--;
        }
        //user presses enter
        if (buffer == '\r') {//If the character if a return the data will be send
          ret = write(fd, buffer_aux, sizeof(char) * i);//You can send in this case up to 256 character
          printf("\n");

          //help command
          if(stringCompare(help_buf, buffer_aux)){
            printf("\nWelcome to the shitty impulse response program\n");
            printf("Here are some useful commands\n");
            printf("> help - yeild this very window \n");
            printf("> play - launch the audio player example [curr broken :,( ]) \n");
            printf("> file - start the file generator \n");
            printf("> quit - exit the program\n");
            printf("\ntype a command to proceed\n");
          }

          //just a test of the uart
          if(stringCompare(test_buf, buffer_aux)){
            printf("well.. the uart works\n\n");
          }

          //testing the file creation...
          if(stringCompare(file_buf, buffer_aux)){
            printf("generating file test...\n");
            if(!file_config()){
              printf("file generation failed, exiting...\n\n");
              return 0;
            }
            return 1;
          }

          //call to test the sine wave generator
          if(stringCompare(tsin_buf, buffer_aux)){
            printf("Starting sine oscillator...\n\n");
            if(!(sine_oscillator_test())){
              printf("error: failed to run the sine oscillator test...\n\n");
            }
          }

          //call to test the .waw file generator
          if(stringCompare(twaw_buf, buffer_aux)){
            printf("Starting waw generator...\n\n");
            if(!waw_testbench()){
              printf("error: failed to generate waw file...\n\n");
            }
          }

          //termination of the program
          if (stringCompare(quit_buf, buffer_aux)) {
            printf("quitting the program... \n");
            return false;
          }

          //starting the play buffer
          if (stringCompare(play_buf, buffer_aux)) {
            printf("Launching Aduio Player... \n");
            if(!audio_Player_Call()){
              printf("audio player failed\n\n");
            }
          }

          if (ret > 0) {
            i = 0;
          }
        }
      }
    }
  }
} //END OF C CODE
