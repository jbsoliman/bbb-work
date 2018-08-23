#include "beatPlayer.h"
#include "audioMixer.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <alsa/asoundlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

//BPM & DEFAULT SETTINGS
#define DEFAULT_VOLUME 80
#define BEATS_PER_MINUTE_DEFAULT 120
#define MAX_BEATS_PER_MINUTE 300
#define MIN_BEATS_PER_MINUTE 40

//TIMINGS
#define NUM_SEC_IN_MIN 60
#define BEATS_PER_FULL_BEAT 2 //Eighth note 

//DRUM SOUNDS
#define SPLASH "wave-files/100060__menegass__gui-drum-splash-hard.wav"
#define TOM_DRUM "wave-files/100066__menegass__gui-drum-tom-mid-hard.wav"
#define SNARE "wave-files/100058__menegass__gui-drum-snare-hard.wav"
#define HIHAT_OPEN "wave-files/100055__menegass__gui-drum-co.wav"
#define HIHAT_CLOSE "wave-files/100054__menegass__gui-drum-ch.wav"
#define BASS_DRUM "wave-files/100051__menegass__gui-drum-bd-hard.wav"
#define CRASH "wave-files/100056__menegass__gui-drum-cyn-hard.wav"
#define SYMBOL "wave-files/100057__menegass__gui-drum-cyn-soft.wav"

static wavedata_t drum_splash;
static wavedata_t drum_toms;
static wavedata_t drum_snare;
static wavedata_t drum_hihat_open;
static wavedata_t drum_hihat_close;
static wavedata_t drum_bass;
static wavedata_t drum_crash;
static wavedata_t drum_symbol;

//static int bpm = BEATS_PER_MINUTE_DEFAULT;
static _Bool playerFlag = true;
static pthread_t beatPlayerThread;
static int bpm = BEATS_PER_MINUTE_DEFAULT;
static int beatPlayerMode = 1;

//https://stackoverflow.com/questions/1157209/is-there-an-alternative-sleep-function-in-c-to-milliseconds
void sleepInMs(int ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);

}

int Beatplayer_getBPM() {
    return bpm;
}

void Beatplayer_setBPM(int beatsPerMin){
    if(beatsPerMin > MAX_BEATS_PER_MINUTE || beatsPerMin < MIN_BEATS_PER_MINUTE) {
        printf("ERROR: Did not set beats per minute, must be within range of 60-300\n");
    } else {
        bpm = beatsPerMin;
    }
};

void Beatplayer_tempoControl() {
    int curBpm = Beatplayer_getBPM();
    double speed = (double) ((double) NUM_SEC_IN_MIN / curBpm / BEATS_PER_FULL_BEAT) * (double) 1000;
    sleepInMs((int)speed);
}

//This beat handler Queues the audio of your choice for beats 1 to 4
void beatHandler(wavedata_t *sound, int beat, int counter) {
    if(beat > 4 || beat < 0) {
        printf("Beat must be within 0-4 range\n");
    } else {
        if(beat == 4) {
            beat = 0; //beat 0 is equivalent to beat 4 (because we count 1-4 in music but 0 to 3 index wise)
        }
        int oncePerBar = counter % 4;
        if (oncePerBar == beat) {
            AudioMixer_queueSound(sound);
        }
    }
}

//This beat handler Queues the audio of your choice for beats 1 to 8
void beatHandlerEighthNote(wavedata_t *sound, int beat, int counter) {
    if (beat < 0 || beat > 8) {
        printf("Beat must be within 0-4 range \n");
    } else {
        if(beat == 8) {
            beat = 0; //beat 0 is equivalent to half-beat 8 (because we count 1-8 in music but 0 to 7 index wise)
        }
        int oncePerBar = counter % 8;
        if(oncePerBar == beat) {
            AudioMixer_queueSound(sound);
        }
    }
}

void Beatplayer_noBeat(){
    Beatplayer_tempoControl();
};

//EACH INDIVIDUAL NUMBER IN THE FOLLOWING FUNCTIONS REPRESENT THE BEAT OR HALF BEAT THAT IT IS PLAYED ON
void Beatplayer_rockBeat() {
    int count = 1;
    while(playerFlag) {
        AudioMixer_queueSound(&drum_hihat_open);
        beatHandler(&drum_bass, 1, count);
        beatHandler(&drum_snare, 3, count);
        beatHandler(&drum_splash, 4, count);
        Beatplayer_tempoControl();
        count++;
    }

};

void Beatplayer_rockBeat2() {
    int count = 1;
    while(playerFlag) {
        beatHandler(&drum_hihat_close, 1, count);
        beatHandler(&drum_hihat_close, 2, count);
        beatHandler(&drum_hihat_close, 3, count);
        beatHandler(&drum_hihat_close, 4, count);
        beatHandler(&drum_bass, 1, count);
        beatHandler(&drum_bass, 2, count);
        beatHandler(&drum_snare, 3, count);
        Beatplayer_tempoControl();
        count++;
    }
};


void Beatplayer_rockBeat3() {
    int count = 1;
    while(playerFlag) {
        beatHandler(&drum_bass, 1, count);
        beatHandler(&drum_bass, 3, count);
        beatHandler(&drum_snare, 2, count);
        beatHandler(&drum_snare, 4, count);
        Beatplayer_tempoControl();
        count++;
    }
}

void Beatplayer_jazzBeat(){
    int count = 1;
    while(playerFlag) {
        count++;
        Beatplayer_tempoControl();
    }
}


void Beatplayer_init(){
    AudioMixer_readWaveFileIntoMemory(SPLASH, &drum_splash);
    AudioMixer_readWaveFileIntoMemory(TOM_DRUM, &drum_toms);
    AudioMixer_readWaveFileIntoMemory(SNARE, &drum_snare);
    AudioMixer_readWaveFileIntoMemory(HIHAT_OPEN, &drum_hihat_open);
    AudioMixer_readWaveFileIntoMemory(HIHAT_CLOSE, &drum_hihat_close);
    AudioMixer_readWaveFileIntoMemory(BASS_DRUM, &drum_bass);
    AudioMixer_readWaveFileIntoMemory(CRASH, &drum_crash);
    AudioMixer_readWaveFileIntoMemory(SYMBOL, &drum_symbol);
    AudioMixer_setVolume(DEFAULT_VOLUME);
    Beatplayer_setBPM(BEATS_PER_MINUTE_DEFAULT);
    AudioMixer_init();
    pthread_create(&beatPlayerThread, NULL, (void *) &Beatplayer_play, NULL);
};

void Beatplayer_play() {
    beatPlayerMode = 2;
    Beatplayer_setBPM(100);
    while(playerFlag) {
        if(beatPlayerMode == 0) {
            Beatplayer_noBeat();
        } else if(beatPlayerMode == 1) {
            Beatplayer_rockBeat();
        } else if(beatPlayerMode == 2) {
            Beatplayer_rockBeat2();
        } else if(beatPlayerMode == 3) {
            beatPlayerMode = 0; //RESET TO NONE
        }
    }
};

void Beatplayer_stop() {
    pthread_join(beatPlayerThread, NULL);
    AudioMixer_cleanup();
    AudioMixer_freeWaveFileData(&drum_splash);
    AudioMixer_freeWaveFileData(&drum_toms);
    AudioMixer_freeWaveFileData(&drum_snare);
    AudioMixer_freeWaveFileData(&drum_hihat_open);
    AudioMixer_freeWaveFileData(&drum_hihat_close);
    AudioMixer_freeWaveFileData(&drum_bass);
    AudioMixer_freeWaveFileData(&drum_crash);
    AudioMixer_freeWaveFileData(&drum_symbol);
};



