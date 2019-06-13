// WAV to MP3 Encoder
// Author: Ivar Wohlfeil
// Copyright (c) 2019 All Rights Reserved

#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <math.h>

#include <pthread.h>
#ifdef WIN32 //for number of cores
    #include <sysinfoapi.h>
#else
    #include <sys/sysinfo.h>
#endif

#include "./lame/include/lame.h"

//#define DEBUG 1
#define ERRORS 1

//#define REDUCE_LAG 1 //leave one core free for better terminal output etc, and only one thread per core
#define THREADS_PER_CORE 2
#define MAX_PATH_LENGTH 256 //pdirent d_name is max. 256 chars long

#ifdef WIN32
    #define ANSI_COLOR_RED      //no color output in cmd
    #define ANSI_COLOR_YELLOW  
    #define ANSI_COLOR_RESET   
#else
    #define ANSI_COLOR_RED     "\x1b[31m" //for color output, errors, etc
    #define ANSI_COLOR_YELLOW  "\x1b[33m"
    #define ANSI_COLOR_RESET   "\x1b[0m"
#endif

volatile unsigned int encode_counter = 0;
pthread_mutex_t lock_encode_counter;
pthread_mutex_t lock_wav_file_list;

/* //unused
typedef struct encoding_thread_parameters_s {
    char *file_path;
    FILE *wav_file;
} encoding_thread_parameters_t;
*/

typedef struct wav_file_s wav_file_t;
typedef struct wav_file_s{
    wav_file_t *next;
    char file_path[MAX_PATH_LENGTH];
} wav_file_t;

wav_file_t *current_wav_file = NULL;
wav_file_t *start_wav_file = NULL;
unsigned int wav_file_counter = 0; //singlethread access, non-volatile

#pragma pack(push, 1)
typedef struct wav_header {
    // RIFF Header
    char riff_header[4];            // Contains "RIFF"
    unsigned int wav_size;          // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
    char wave_header[4];            // Contains "WAVE"
    // Format Header
    char fmt_header[4];             // Contains "fmt " (includes trailing space)
    unsigned int fmt_chunk_size;    // Should be 16 for PCM
    unsigned short audio_format;    // Should be 1 for PCM. 3 for IEEE Float
    unsigned short num_channels;
    unsigned int sample_rate;
    unsigned int byte_rate;         // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
    unsigned short sample_alignment;// num_channels * Bytes Per Sample
    unsigned short bit_depth;       // Number of bits per sample
    //char *dump;                   //optional extra parameters, dumped in read_header() TODO use the parameters ? where to find specification ?
    // Data
    unsigned char data_header[4];   // Contains "data"
    unsigned int data_bytes;        // Number of bytes in data. Number of samples * num_channels * sample byte size
    // uint8_t bytes[];             // Remainder of wave file is bytes
} __attribute__((packed)) wav_header_t;
#pragma pack(pop)

int encodeWavFile(const char *file_path);
void printWavHeader(const wav_header_t *current_header);
void *encoding_thread_function(void *data_unused);
int is_wav_file(const char *file_path);
int import_wav_file_path_list();
int deallocate_wav_file_list();
int read_header(FILE *current_file, wav_header_t *current_header);
int get_number_of_processors();

#endif
