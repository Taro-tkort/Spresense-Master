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
#include <cmath>

struct audioData {
    int sampleRate, bitDepth, duration;
    float frequency, amplitude, angle;
};

struct header {
    char riff[4], wave[4], fmt[4], data[4];
    uint32_t fileSize, fmtLen, sampleRate, byteRate, data_size;
    uint16_t format, chanCnt, blkAlign, bitPerSample;
};


FILE *generate_waw_file(audioData payload){ //generates an empty waw file
    FILE *fp;
    fp = fopen("/mnt/sd0/AUDIO/sine.waw", "rb+"); //open in a RW binary mode

    header wawHdr; //create instance of waw header 

    //assigning the standard values to the header

    //the strings have to be done this way...
    wawHdr.riff[0] = 'R';
    wawHdr.riff[1] = 'I';
    wawHdr.riff[2] = 'F';
    wawHdr.riff[3] = 'F';

    wawHdr.wave[0] = 'W';
    wawHdr.wave[1] = 'A';
    wawHdr.wave[2] = 'V';
    wawHdr.wave[3] = 'E';

    wawHdr.fmt[0] = 'f';
    wawHdr.fmt[1] = 'm';
    wawHdr.fmt[2] = 't';
    wawHdr.fmt[3] = ' ';

    wawHdr.wave[0] = 'W';
    wawHdr.wave[1] = 'A';
    wawHdr.wave[2] = 'V';
    wawHdr.wave[3] = 'E';

    wawHdr.data[0] = 'D';
    wawHdr.data[1] = 'A';
    wawHdr.data[2] = 'T';
    wawHdr.data[3] = 'A';

    wawHdr.fileSize = 16;
    wawHdr.fmtLen = 1;
    wawHdr.chanCnt = 2;
    wawHdr.sampleRate = payload.sampleRate;
    wawHdr.byteRate = (payload.sampleRate*payload.bitDepth*wawHdr.chanCnt)/8;
    wawHdr.blkAlign = (payload.bitDepth*wawHdr.chanCnt)/8;
    wawHdr.bitPerSample = 16;
    wawHdr.data_size = (payload.sampleRate*wawHdr.chanCnt*wawHdr.bitPerSample)/8 *payload.duration;
    
    //writing of the waw file
    fwrite(&wawHdr, sizeof(wawHdr),1,fp);
    return fp;
}

/* not currently needed
bool read_audioConfig(){ //extracts the data from prexisting audio config
    This function is intended to read a config file and extrac payload data from this file
    this will then be used to genereate the needed number of .waw files
}
*/

float sine_oscillator(audioData payload, float time){ //simple sine oscillator that can be called 
    float phase = 2*M_PI*payload.frequency*time;
    float sample = payload.amplitude*sin(phase);
    return sample;
}

bool wawgen(audioData payload){ //responsible for writing the waw file
    FILE *fp = generate_waw_file(payload);

    //initial values for the sine oscillator
    float time = 0;
    float tau = 1/payload.sampleRate;
    int duration = payload.duration;
    while (time <= duration){
        float sample = sine_oscillator(payload, time);
        fprintf(fp, "%f", sample);
        printf("%f", sample);
        time += tau;
    }
    fclose(fp);
    return 1;
}