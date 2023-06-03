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
    int sampleRate, bitPerSample;
    float frequency, frequencyStop, amplitude, angle, duration;
};

struct header {
    char riff[4];
    int32_t riffLen;
    char wave[4];
    char fmt[4];
    int fmtLen;
    int16_t audioFormat;
    int16_t chanCnt;
    int32_t sampleRate;
    int32_t byteRate;
    int16_t blockAlign;
    int16_t bitPerSample;
    char data[4];
    int32_t dataSize;
};

bool generate_waw_file(audioData payload){ //generates an empty waw file
    FILE *fp;
    fp = fopen("/mnt/sd0/AUDIO/sineRiffWonky.wav", "wb"); //open in a RW binary mode

    header wavHdr; //create instance of waw header
    //assigning the standard values to the header

    //the strings have to be done this way...
    wavHdr.riff[0] = 'R';
    wavHdr.riff[1] = 'I';
    wavHdr.riff[2] = 'F';
    wavHdr.riff[3] = 'F';

    wavHdr.wave[0] = 'W';
    wavHdr.wave[1] = 'A';
    wavHdr.wave[2] = 'V';
    wavHdr.wave[3] = 'E';

    wavHdr.fmt[0] = 'f';
    wavHdr.fmt[1] = 'm';
    wavHdr.fmt[2] = 't';
    wavHdr.fmt[3] = ' ';

    wavHdr.fmtLen = 16;
    wavHdr.audioFormat = 1;
    wavHdr.chanCnt = 1;
    wavHdr.sampleRate = payload.sampleRate;
    wavHdr.byteRate = wavHdr.chanCnt * payload.sampleRate*(payload.bitPerSample/8);
    wavHdr.blockAlign = wavHdr.chanCnt * payload.bitPerSample/8;
    wavHdr.bitPerSample = payload.bitPerSample;

    wavHdr.data[0] = 'd';
    wavHdr.data[1] = 'a';
    wavHdr.data[2] = 't';
    wavHdr.data[3] = 'a';

    wavHdr.dataSize = 0;
    wavHdr.riffLen = 0;


    //filling size bytes
    wavHdr.dataSize = (payload.sampleRate * wavHdr.chanCnt * wavHdr.bitPerSample)/8 * payload.duration;
    wavHdr.riffLen = wavHdr.dataSize + 44 - 8;
    printf("dataSize is %ld and file size should be %ld\n", wavHdr.dataSize, wavHdr.riffLen);

    //writing of the waw file
    fwrite(&wavHdr, sizeof(wavHdr),1,fp);
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
    int16_t iSample = sample * (pow(2, payload.bitPerSample - 1) - 1); //convert to 16 bit
    return iSample;
}

int16_t exp_sine_oscillator(audioData payload, float time){ //exponential sine oscillator based on Novak2015
    float f1 = payload.frequency;
    float f2 = payload.frequencyStop;
    float T = payload.duration;
    float stg1 = (2*M_PI*f1*T)/log(f2/f1);
    float stg2 = exp((time/T)*log(f2/f1));
    float sample = payload.amplitude*sin(stg1*stg2);
    int16_t iSample = sample * (pow(2, payload.bitPerSample - 1) - 1);
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
    fp = fopen("/mnt/sd0/AUDIO/sineRiffWonky.wav", "ab"); //open in a RW binary mode

    //initial values for the sine oscillator
    float time = 0;
    float DUR = payload.duration;
    float SR = payload.sampleRate;
    float tau = 1/SR;
    //int p = 0;

    while (time <= DUR){
        int16_t sample = sine_oscillator(payload, time);
        fwrite(&sample, sizeof(sample), 1, fp);
        time += tau;
    }

    //closing protocol
    fclose(fp);
    return 1;
}
