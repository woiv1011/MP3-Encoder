#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <math.h>
#include <errno.h>

#include <pthread.h>
#include <lame/lame.h>

//#define DEBUG 1


//for color output, errors, etc
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"



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

#endif