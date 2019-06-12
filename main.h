#ifndef MAIN_H
#define MAIN_H


//INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <math.h>
#include <errno.h>

#include <pthread.h>
#include <lame/lame.h>

#include <sys/sysinfo.h> //for number of cores


//MACROS
//#define DEBUG 1
//#define PRINT 1
#define ERRORS 1
#define PREVENT_LAG 1 //leave one core free for better terminal output etc, and only one thread per core
#define MAX_PATH_LENGTH 256 //pdirent d_name is 256 chars long

#define ANSI_COLOR_RED     "\x1b[31m" //for color output, errors, etc
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define DEBUG_INFO printf ("DEBUG function: %s, file: \"%s\", line: %d \n", __func__, __FILE__, __LINE__) 

volatile unsigned int encode_counter = 0;
DIR *pDir = NULL; //TODO pointers cant be / dont need to be volatile ???
pthread_mutex_t lock_encode_counter;
pthread_mutex_t lock_pDir;
char *directory_path = NULL; //TODO make volatile ? thread safe ?, how does const work in this case ?

typedef struct encoding_thread_parameters_s {
    char *file_path;
    FILE *wav_file;
} encoding_thread_parameters_t;

#pragma pack(push, 1)
typedef struct wav_header {
    // RIFF Header
    char riff_header[4]; // Contains "RIFF"
    unsigned int wav_size; // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
    char wave_header[4]; // Contains "WAVE"
    
    // Format Header
    char fmt_header[4]; // Contains "fmt " (includes trailing space)
    unsigned int fmt_chunk_size; // Should be 16 for PCM
    unsigned short audio_format; // Should be 1 for PCM. 3 for IEEE Float
    unsigned short num_channels;
    unsigned int sample_rate;
    unsigned int byte_rate; // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
    unsigned short sample_alignment; // num_channels * Bytes Per Sample
    unsigned short bit_depth; // Number of bits per sample
    
    // Data
    unsigned char data_header[4]; // Contains "data"
    unsigned int data_bytes; // Number of bytes in data. Number of samples * num_channels * sample byte size
    // uint8_t bytes[]; // Remainder of wave file is bytes
} __attribute__((packed)) wav_header_t; //
#pragma pack(pop) //TODO doppelt notwendig/sinnvoll ?

int encodeWavFile(const char *file_path, FILE *wav_file);
void printWavHeader(const wav_header_t *current_header);
void *encoding_thread_function(void *data);
int isWavFile(const char *file_path, FILE *current_file);
int import_wav_file_list();

#endif

/*
//what do i want ?
int check_and_encode_wavFile(either FILE* or filepath) {
    return 1 if successful
}*/