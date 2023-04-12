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
  bool audio_Player_Call();

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
    fp = fopen("/mnt/sd0/AUDIO/file.txt", "r");
    if (fp == NULL){
      return 0;
    }
    printf("attempting reading\n");
    int c;
    while(1) {
      c = fgetc(fp);
      if( feof(fp) ) { 
         break ;
      }
      printf("%c", c);
   }

    printf("closing file...\n\n");
    fclose(fp);

    return true;
  }

  bool waw_testbench(){
    audioData test;
    FILE *fp = generate_waw_file(test);
    fclose(fp);
    printf("file should now be generated");
  }

  bool sine_oscillator_test(){
    float sample;
    float time = 0;
    audioData temp;
    temp.amplitude = 2;
    temp.angle = 0;
    temp.frequency = 4;
    temp.bitDepth = 8;
    temp.sampleRate = 10;
    temp.duration = 4;
    float dur = temp.duration;
    while(time <= dur){
      sample = sine_oscillator(temp, time);
      printf("%f", sample);
      time += tau;
    }
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
    //arbitrary test buffer
    char test_buf[4] = {'t','e','s','t'};
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

          //test of the waw generator
          if(stringCompare(test_buf, buffer_aux)){
            printf("Starting waw generator...\n\n");

            if(!file_config()){
              printf("error: failed to open file...\n\n");
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

  bool audio_Player_Call(){
    //TESTING ENV
    //////////////////////////////////////////////////////////////////////
    //initialize functions

    /* Initialize clock mode.
     * Clock mode indicates whether the internal processing rate of
     * AudioSubSystem is Normal mode or Hi-Res mode. 
     * The sampling rate of the playback file determines which mode
     * will be taken. When playing a Hi-Res file,
     * please set player mode to Hi-Res mode with config.
     */

    int clk_mode = -1;

    printf("Start AudioPlayer with ObjIf example\n");

    /* First, initialize the shared memory and memory utility used by AudioSubSystem. */

    if (!app_init_libraries())
      {
        printf("Error: init_libraries() failure.\n");
        return 1;
      }

    /* Next, Create the features used by AudioSubSystem. */

    if (!app_create_audio_sub_system())
      {
        printf("Error: act_audiosubsystem() failure.\n");

        /* Abnormal termination processing */

        goto errout_act_audio_sub_system;
      }

    /* On and after this point, AudioSubSystem must be active.
     * Register the callback function to be notified when a problem occurs.
     */

    /* Open directory of play contents. */

    if (!app_open_contents_dir())
      {
        printf("Error: app_open_contents_dir() failure.\n");

        /* Abnormal termination processing */

        goto errout_open_contents_dir;
      }

    /* Initialize frequency lock parameter. */

    app_init_freq_lock();

    /* Lock cpu frequency to high. */

    app_freq_lock();

  #ifdef CONFIG_AUDIOUTILS_PLAYLIST
    /* Open playlist. */

    if (!app_open_playlist())
      {
        printf("Error: app_open_playlist() failure.\n");

        /* Abnormal termination processing */

        goto errout_open_playlist;
      }
  #endif

    /* Initialize simple fifo. */

    if (!app_init_simple_fifo())
      {
        printf("Error: app_init_simple_fifo() failure.\n");

        /* Abnormal termination processing */

        goto errout_init_simple_fifo;
      }

    /* Set the device to output the mixed audio. */

    if (!app_activate_baseband())
      {
        printf("Error: app_activate_baseband() failure.\n");

        /* Abnormal termination processing */

        goto errout_init_output_select;
      }

    /* Set player operation mode. */

    if (!app_activate_player_system())
      {
        printf("Error: app_activate_player_system() failure.\n");
        return 1;
      }

    for (int i = 0; i < PLAYER_PLAY_FILE_NUM; i++)
      {
        if (!app_open_next_play_file())
          {
            /* Abnormal termination processing */

            goto errout_open_next_play_file;
          }

        /* Get current clock mode.
         * If the sampling rate is less than 48 kHz,
         * it will be in Normal mode. Otherwise, Hi-Res mode is set.
         */

        int cur_clk_mode;
        if (s_player_info.file.track.sampling_rate <= AS_SAMPLINGRATE_48000)
          {
            cur_clk_mode = AS_CLKMODE_NORMAL;
          }
        else
          {
            cur_clk_mode = AS_CLKMODE_HIRES;
          }

        /* If clockmode is Hi-Res and player mode is not Hi-Res,
         * play the next file.
         */

  #ifdef CONFIG_EXAMPLES_AUDIO_PLAYER_OBJIF_MODE_NORMAL
        if (cur_clk_mode == AS_CLKMODE_HIRES)
          {
            printf("Hi-Res file is not supported.\n"
                   "Please change player mode to Hi-Res with config.\n");
            app_close_play_file();

            /* Play next file. */

            continue;
          }
  #endif

        /* If current clock mode is different from the previous clock mode,
         * perform initial setting.
         */

        if (clk_mode != cur_clk_mode)
          {
            /* Update clock mode. */

            clk_mode = cur_clk_mode;

            /* Since the initial setting is required to be in the Ready state,
             * if it is not in the Ready state, it is set to the Ready state.
             */

            if (board_external_amp_mute_control(true) != OK)
              {
                printf("Error: board_external_amp_mute_control(true) failuer.\n");

                /* Abnormal termination processing */

                goto errout_amp_mute_control;
              }

            if (!app_deactivate_baseband())
              {
                printf("Error: app_deactivate_baseband() failure.\n");

              }

            /* Set the clock mode of the output function. */

            if (!app_set_clkmode(clk_mode))
              {
                printf("Error: app_set_clkmode() failure.\n");

                /* Abnormal termination processing */

                goto errout_set_clkmode;
              }

            if (!app_activate_baseband())
              {
                printf("Error: app_activate_baseband() failure.\n");

                /* Abnormal termination processing */

                goto errout_init_output_select;
              }

             /* Cancel output mute. */

             app_set_volume(PLAYER_DEF_VOLUME);

            if (board_external_amp_mute_control(false) != OK)
              {
                printf("Error: board_external_amp_mute_control(false) failuer.\n");

                /* Abnormal termination processing */

                goto errout_amp_mute_control;
              }
          }

        /* Start player operation. */

        if (!app_start())
          {
            printf("Error: app_start_player() failure.\n");

            /* Abnormal termination processing */

            goto errout_start;
          }

        /* Running... */

        printf("Running time is %d sec\n", PLAYER_PLAY_TIME);

        app_play_process(PLAYER_PLAY_TIME);

        /* Stop player operation. */

        if (!app_stop())
          {
            printf("Error: app_stop() failure.\n");
            return 1;
          }

  #ifndef CONFIG_AUDIOUTILS_PLAYLIST
        break;
  #endif
      }

    /* Set output mute. */

  errout_start:

    if (board_external_amp_mute_control(true) != OK)
      {
        printf("Error: board_external_amp_mute_control(true) failuer.\n");
        return 1;
      }

  #ifdef CONFIG_AUDIOUTILS_PLAYLIST
  errout_open_playlist:
  #endif
  errout_amp_mute_control:
  errout_init_simple_fifo:
  errout_set_clkmode:
  errout_init_output_select:
  errout_open_next_play_file:

    /* Unlock cpu frequency. */

    app_freq_release();

    /* Close playlist. */

    if (!app_close_playlist())
      {
        printf("Error: app_close_playlist() failure.\n");
        return 1;
      }

    if (!app_close_contents_dir())
      {
        printf("Error: app_close_contents_dir() failure.\n");
        return 1;
      }

    /* Deactivate the features used by AudioSubSystem. */

    if (!app_deact_player_system())
      {
        printf("Error: app_deact_player_system() failure.\n");
        return 1;
      }

    /* Deactivate baseband */

    if (!app_deactivate_baseband())
      {
        printf("Error: app_deactivate_baseband() failure.\n");
  
      }

  errout_open_contents_dir:
    app_deact_audio_sub_system();

    /* finalize the shared memory and memory utility used by AudioSubSystem. */

  errout_act_audio_sub_system:
    if (!app_finalize_libraries())
      {
        printf("Error: finalize_libraries() failure.\n");
        return 1;
      }

    printf("Exit AudioPlayer example\n\n");

    return true;
    //////////////////////////////////////////////////////////////////////////
    // END OF TEST ENV

    }// END OF AUDIO_CALL
} //END OF C CODE
