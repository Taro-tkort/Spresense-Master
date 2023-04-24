#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <asmp/mpshm.h>
#include <arch/chip/pm.h>
#include <arch/board/board.h>
#include <sys/stat.h>
#include <cmath>

struct audioData {
    int sampleRate, bitDepth, duration;
    float frequency, amplitude, angle;
};

struct header {
    char riff[4];
    int32_t fileSize;
    char wave[4];
    char fmt[4];
    int32_t fmtLen;
    int16_t format;
    int16_t chanCnt;
    int32_t sampleRate;
    int32_t byteRate;
    int16_t blkAlign;
    int16_t bitPerSample;
    char data[4];
    int32_t data_size;
};

bool generate_waw_file(audioData payload){ //generates an empty waw file
    FILE *fp;
    fp = fopen("/mnt/sd0/AUDIO/sine.wav", "wb"); //open in a RW binary mode

    header wawHdr; //create instance of waw header 

    //assigning the standard values to the header

    //the strings have to be done this way...
    wawHdr.riff[0] = 'R';
    wawHdr.riff[1] = 'I';
    wawHdr.riff[2] = 'F';
    wawHdr.riff[3] = 'F';

    wawHdr.fileSize = 16;

    wawHdr.wave[0] = 'W';
    wawHdr.wave[1] = 'A';
    wawHdr.wave[2] = 'V';
    wawHdr.wave[3] = 'E';

    wawHdr.fmt[0] = 'f';
    wawHdr.fmt[1] = 'm';
    wawHdr.fmt[2] = 't';
    wawHdr.fmt[3] = ' ';

    wawHdr.fmtLen = 16;
    wawHdr.format = 1;
    wawHdr.chanCnt = 2;
    wawHdr.sampleRate = payload.sampleRate;
    wawHdr.byteRate = (payload.sampleRate*payload.bitDepth*wawHdr.chanCnt)/8;
    wawHdr.blkAlign = (payload.bitDepth*wawHdr.chanCnt)/8;
    wawHdr.bitPerSample = 16;

    wawHdr.data[0] = 'd';
    wawHdr.data[1] = 'a';
    wawHdr.data[2] = 't';
    wawHdr.data[3] = 'a';

    wawHdr.data_size = (payload.sampleRate*wawHdr.chanCnt*wawHdr.bitPerSample)/8 * payload.duration;
    
    //writing of the waw file
    fwrite(&wawHdr, sizeof(wawHdr),1,fp);
    fclose(fp);


    //check if file exists CURRENTLY BROKEN
    /*
    const char *file = "/mnt/sd0/AUDIO/sine.waw";
    if(access(file, F_OK) != 0){
        printf("file creation failed..\n");
        return 0;
    }
    printf("file creation complete, file exists\n");
    */
    return 1;
}
/* not currently needed
bool read_audioConfig(){ //extracts the data from prexisting audio config
    This function is intended to read a config file and extrac payload data from this file
    this will then be used to genereate the needed number of .waw files
}
*/

int16_t sine_oscillator(audioData payload, float time){ //simple sine oscillator that can be called 
    float phase = 2*M_PI*payload.frequency*time;
    float sample = payload.amplitude*sin(phase);
    int16_t iSample = sample * (pow(2, payload.bitDepth - 1) - 1); //convert to 16 bit
    return iSample;
}

bool wawgen(audioData payload){ //responsible for writing the waw file
    if(!generate_waw_file(payload)){
        printf("waw generation failed.. \n");
        return 0;
    }
    printf("generated header file..\n\n Starting sine generation..\n\n");

    //open file
    FILE *fp;
    fp = fopen("/mnt/sd0/AUDIO/sine.wav", "ab"); //open in a RW binary mode

    //initial values for the sine oscillator
    float time = 0;
    float DUR = payload.duration;
    float SR = payload.sampleRate;
    float tau = 1/SR;

    while (time <= DUR){
        int16_t sample = sine_oscillator(payload, time);
        fprintf(fp, "%i", sample);
        //fprintf(fp, "%i", sample); //caused by channel cnt 2
        time += tau;
    }
    
//testspace////////////////////////////////////////

//testspace////////////////////////////////////////
    fclose(fp);
    return 1;
}
