// WAV to MP3 Encoder
// Author: Ivar Wohlfeil
// Copyright (c) 2019 All Rights Reserved

#include "main.h"

int main(int argc, char **argv) {
#ifdef DEBUG
    printf("\targc: '%d'\n", argc);
    for (int i=0; i < argc; i=i+1) {
        printf("\targv[%d]: '%s'\n", i, argv[i]);
    }
    printf("\n");

    printf("sizeof(wav_header_t): %ld\n", sizeof(wav_header_t));
    printf("sizeof(wav_file_t): %ld\n", sizeof(wav_file_t));
#endif

    if ((argc != 2) || (argv[1] == NULL)) {
        printf ("Incorrect Usage\n");
        printf ("Usage: ./main.out <directory_path>\n");
        printf ("Example: ./main.out ./wavFiles/\n");
        return -1;
    }

    printf("START IMPORT\n");
    import_wav_file_path_list(argv[1]);

    printf("\n");
    printf("START ENCODE\n");
    
    if (pthread_mutex_init(&lock_encode_counter, NULL) != 0) {
#ifdef ERRORS
        printf("\n lock_encode_counter mutex init has failed\n");
#endif
        return -1;
    }
    if (pthread_mutex_init(&lock_wav_file_list, NULL) != 0) {
#ifdef ERRORS
        printf("\n lock_wav_file_list mutex init has failed\n");
#endif
        return -1;
    }

    int num_cores = get_number_of_processors();

#ifdef REDUCE_LAG
    unsigned int num_threads = num_cores-1; //leave 1 core free to prevent lag in terminal output etc, and use only one thread per core
#else
    unsigned int num_threads = num_cores*THREADS_PER_CORE;
#endif

    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    printf("Number of Cores: %d\n", num_cores); 
    printf("Number of Threads: %d\n", num_threads);

    //thread start parameters
    void *(*functionPtr) (void *);
    functionPtr = &encoding_thread_function;
    void *data_unused = NULL;
    //encoding_thread_parameters_t thread_params; //unused

    //start all threads
    for (int i=0; i<num_threads; i=i+1) {
        //int pthread_create_error = pthread_create(&(threads[i]), NULL, functionPtr, (void*)&(thread_params)); //thread_params not used
        int pthread_create_error = pthread_create(&(threads[i]), NULL, functionPtr, data_unused);
        if(pthread_create_error != 0) {
#ifdef ERRORS
            printf("\nThread %d can't be created :[%s]\n", i, strerror(pthread_create_error));
#endif
        }
    }
    //wait for all threads to end
    for (int i=0; i<num_threads; i=i+1) {
        int pthread_join_error = pthread_join(threads[i], NULL);
        if(pthread_join_error != 0) {
#ifdef ERRORS
            printf("\nThread %d can't be joined :[%s]\n", i, strerror(pthread_join_error));
#endif
        }
    }
    printf(ANSI_COLOR_YELLOW "\n'%d' WAV files encoded to MP3.\n" ANSI_COLOR_RESET, encode_counter);

    pthread_mutex_destroy(&lock_encode_counter);
    pthread_mutex_destroy(&lock_wav_file_list);
    
    free(threads);
    deallocate_wav_file_list();

    printf("Exit.\n");
    return 0;
}

//single-thread import, dirent.h readdir() causes certain bugs when multithreaded, even with locks
int import_wav_file_path_list(const char *dir_path) {
    //parameter must be correctly formatted and existing folder path
    char directory_path[MAX_PATH_LENGTH];
    strncpy(directory_path, dir_path, MAX_PATH_LENGTH);

    //append slash after directory so that later concatenation with filename is possible
    if(directory_path[strnlen(directory_path, MAX_PATH_LENGTH) - 1] != '/') {
        strncat(directory_path, "/", MAX_PATH_LENGTH);
    }

    DIR *pDir = opendir(directory_path);
    if (pDir == NULL) {
#ifdef ERRORS
        printf("Cannot open directory '%s'\n", directory_path);
        printf("errno %d: %s\n", errno, strerror(errno));
#endif
        return -1;
    }

    struct dirent *pDirent;
    unsigned int file_counter = 0;
    for (int i = 0; (pDirent = readdir(pDir)) != NULL; i=i+1) { //iterate through files
        wav_file_t *new_wav_file = malloc(sizeof(wav_file_t)); //gets deallocated in deallocate_wav_file_list() at end of program
        strncpy(new_wav_file->file_path, directory_path, MAX_PATH_LENGTH); //store file_path string directly in new node
        strncat(new_wav_file->file_path, pDirent->d_name, MAX_PATH_LENGTH);
#ifdef DEBUG
        printf("%d. import\n", file_counter);
#endif
        file_counter += 1;
        if(is_wav_file(new_wav_file->file_path)) {
            new_wav_file->next = NULL;
            if(current_wav_file == NULL) { //begin list
                current_wav_file = new_wav_file;
                start_wav_file = new_wav_file;
            } else { //append to list
                current_wav_file->next = new_wav_file;
                current_wav_file = new_wav_file;
            }
            wav_file_counter += 1;
        } else {
            free(new_wav_file);
            continue;
        }
    }

    closedir(pDir);
    current_wav_file = start_wav_file; //reset to start for reading
    printf(ANSI_COLOR_YELLOW "\n'%d' WAV files found / imported.\n" ANSI_COLOR_RESET, wav_file_counter);
    return 0;
}


int is_wav_file(const char *file_path) {
#ifdef DEBUG
    printf("\tfile_path: '%.256s'\n", file_path); 
    printf("\tstrnlen(file_path): %ld\n", strnlen(file_path, MAX_PATH_LENGTH));
#endif

    FILE *current_file = fopen(file_path, "rb");
    //determine file size
    fseek(current_file, 0, SEEK_END);
    long file_size = ftell(current_file);
    fseek(current_file, 0, SEEK_SET);
#ifdef DEBUG
    printf("\tfile_size: %ld\n", file_size);
#endif

    int last4chars_correct = 0;
    int header_correct = 0;
    wav_header_t current_header;

    //filter out "." and "..", anything that can't be ".wav" // TODO move this into is_wav_file()

    //TODO make properly case insensitive with toFloor() etc, not really necessary since combinations like ".wAv" are rarely used
    last4chars_correct = strnlen(file_path, MAX_PATH_LENGTH) >= 4 &&
                        (0 == strncmp(".wav", (file_path + strnlen(file_path, MAX_PATH_LENGTH) - 4), 4) ||
                        0 == strncmp(".WAV", (file_path + strnlen(file_path, MAX_PATH_LENGTH) - 4), 4));
    if(!last4chars_correct) {
        return 0;
    }
    
    if(file_size > sizeof(wav_header_t)) {
        int header = read_header(current_file, &current_header);
        if (!header) {
#ifdef ERRORS
            printf(ANSI_COLOR_RED "'%.256s:'Error while reading wav header\n" ANSI_COLOR_RESET, file_path);
#endif
            return 0; //bad file header
        } else {
            header_correct = (0 == strncmp("WAVE", current_header.wave_header, 4)) && (0 == strncmp("RIFF", current_header.riff_header, 4));
        }
    } else {
        return 0; //file too small to contain a wav_header
    }
#ifdef DEBUG
    printf("\tlast4chars_correct: %d header_correct: %d\n", last4chars_correct, header_correct);
#endif
    fclose(current_file);
    return (last4chars_correct && header_correct);
}

char *get_next_wav_file_path() {
    if(current_wav_file == NULL) {
        return NULL;
    }
    char *output = current_wav_file->file_path;
    current_wav_file = current_wav_file->next;
    return output;
}

void *encoding_thread_function(void *data_unused) {
    //printf("encode thread function on thread %ld\n", (long) pthread_self());
    int i = 0; //per-thread local encode counter

    while(1) { //infinite loop that polls for wav Files to be encoded
        pthread_mutex_lock(&lock_wav_file_list);
        char *wav_file_path = get_next_wav_file_path();
        pthread_mutex_unlock(&lock_wav_file_list);

        if (wav_file_path == NULL) { //end of directory reached
            printf("thread %ld local encode counter: %d\n", (long) pthread_self(), i);
            return NULL;
        } else {
            int encode_successful = encodeWavFile(wav_file_path);
            if (encode_successful) {
                pthread_mutex_lock(&lock_encode_counter);
                encode_counter += 1; //TODO only do this once at the end, for performance, aggregate all local thread counters, prevent locks
                pthread_mutex_unlock(&lock_encode_counter);
            } else {
                printf("encode '%.256s' failed on thread: %ld\n", wav_file_path, (long) pthread_self());
            }
            i=i+1;
        }

    } //end while(1)

    //return NULL;
    return 0;
}

int read_header(FILE *current_file, wav_header_t *current_header) {
    int riff_header_read      =  fread(current_header->riff_header, 1, 4, current_file); //4 chars
    int wav_size_read         =  fread(&(current_header->wav_size), 4, 1, current_file); //unsigned int, 4 bytes / 32bits
    int wave_header_read      =  fread(current_header->wave_header, 1, 4, current_file); //4 chars
    int fmt_header_read       =  fread(current_header->fmt_header, 1, 4, current_file); //4 chars
    int fmt_chunksize_read    =  fread(&(current_header->fmt_chunk_size), 4, 1, current_file); //unsigned int, 4 bytes / 32bits
    int audio_format_read     =  fread(&(current_header->audio_format), 2, 1, current_file); //unsigned short, 2 bytes
    int num_channels_read     =  fread(&(current_header->num_channels), 2, 1, current_file); //unsigned short, 2 bytes
    int sample_rate_read      =  fread(&(current_header->sample_rate), 4, 1, current_file); //unsigned int, 4 bytes
    int byte_rate_read        =  fread(&(current_header->byte_rate), 4, 1, current_file); //unsigned int, 4 bytes
    int sample_alignment_read =  fread(&(current_header->sample_alignment), 2, 1, current_file); //unsigned short, 2 bytes
    int bit_depth_read        =  fread(&(current_header->bit_depth), 2, 1, current_file); //unsigned short, 2 bytes
    //int dump_read = 0;        
    if(current_header->fmt_chunk_size > 16) {
        //read additional, optional bytes
        //TODO use the additional bytes somehow ?
        //int diff  = (current_header->fmt_chunk_size) - 16; //number of additional bytes in header
        //char *dump = malloc(diff);
        //dump_read = fread(dump, 1, diff, current_file);
        //free(dump);
        //or just increment the file offset instead
        //fseek(current_file, diff, SEEK_CUR);

#ifdef ERRORS
        printf(ANSI_COLOR_RED "Non-PCM WAV file not supported.\n" ANSI_COLOR_RESET);
#endif
        return 0;
    }
    int data_header_read      =  fread(current_header->data_header, 1, 4, current_file); //4 chars
    int data_bytes_read       =  fread(&(current_header->data_bytes), 4, 1, current_file); //unsigned int, 4 bytes

    int output =    (riff_header_read == 4) && (wav_size_read == 1) && (wave_header_read == 4) &&
                    (fmt_header_read == 4) && (fmt_chunksize_read == 1) && (audio_format_read == 1) &&
                    (num_channels_read == 1) && (sample_rate_read == 1) && (byte_rate_read == 1) &&
                    (sample_alignment_read == 1) && (bit_depth_read == 1) && (data_header_read == 4) &&
                    (data_bytes_read == 1);

    return output;
}

int encodeWavFile(const char *file_path) {
    wav_header_t current_header;
    FILE *wav_file = fopen(file_path, "rb");
    
    //determine file size
    fseek(wav_file, 0, SEEK_END);
    long file_size = ftell(wav_file);
    fseek(wav_file, 0, SEEK_SET);

#ifdef DEBUG
    printf("%d. encode\n", encode_counter);
    printf("\twav file_path: '%.256s'\n", file_path);
    printf("\twav file_size: %ld\n", file_size);
#endif
    short int *pcm_data = malloc(file_size - sizeof(wav_header_t));
    if(file_size > sizeof(wav_header_t)) {
        int header = read_header(wav_file, &current_header);
        if (!header) {
#ifdef ERRORS
            printf(ANSI_COLOR_RED "'%.256s:'Error while reading wav header\n" ANSI_COLOR_RESET, file_path);
#endif
            return 0; //bad header
        }
    } else {
        return 0; //file too small to contain a wav_header
    }

#ifdef DEBUG
    printWavHeader(&current_header);
#endif
    int wav_header_size = sizeof(wav_header_t) + (current_header.fmt_chunk_size - 16);
    int read_pcm = fread(pcm_data, 1, (file_size - wav_header_size), wav_file);
#ifdef DEBUG
    printf("read_pcm: %d\n", read_pcm);
#endif
    if (read_pcm != (file_size - wav_header_size)) {
#ifdef ERRORS
        printf(ANSI_COLOR_RED "\nError while reading pcm data\n" ANSI_COLOR_RESET);
#endif
        return 0;
    }

    int num_samples = current_header.data_bytes / (current_header.num_channels * (current_header.bit_depth/8)); //number of samples per channel

    //LAME API, every file has different settings and needs its own gfp
    lame_global_flags *gfp;
    gfp = lame_init();

    lame_set_num_samples(gfp, num_samples);
    lame_set_num_channels(gfp, current_header.num_channels);
    lame_set_in_samplerate(gfp, current_header.sample_rate);

    //lame_set_out_samplerate(gfp, 0); //default = 0; lame picks best value
    lame_set_brate(gfp, 192);
    lame_set_quality(gfp, 5);   // 5 = good quality, fast

    int ret_code = lame_init_params(gfp);

    if (ret_code < 0) {
#ifdef ERRORS
        printf(ANSI_COLOR_RED "\nlame_init_params() ret code error\n" ANSI_COLOR_RESET);
#endif
        return -1;
    }

    long int mp3_buffer_size = 1.25*num_samples + 7200; //worst case estimate according to lame/API
    unsigned char *mp3_buffer = malloc(mp3_buffer_size);

    int mp3_bytes = 0;
    if(current_header.num_channels == 1) {
        //lame_set_mode(gfp, MONO); //unnecessary, lame picks automatically based on num_channels
        mp3_bytes = lame_encode_buffer(gfp, pcm_data, NULL, num_samples, mp3_buffer, mp3_buffer_size);

    } else if(current_header.num_channels == 2) {
        mp3_bytes = lame_encode_buffer_interleaved(gfp, pcm_data, num_samples, mp3_buffer, mp3_buffer_size);
    } else {
        //more than two channels (5.1, 7.1) ? zero channels ?
        printf(ANSI_COLOR_RED "\nNumber of Channels (%d) not supported.\n" ANSI_COLOR_RESET, current_header.num_channels);
        return -1;
    }
    
    if (mp3_bytes < 0) {
#ifdef ERRORS
        printf(ANSI_COLOR_RED "\nlame encode error\n" ANSI_COLOR_RESET);
#endif
    }

    unsigned int mp3_flush_buffer_size = 7200;
    unsigned char *mp3_flush_buffer = malloc(mp3_flush_buffer_size);

    int mp3_flush_bytes = lame_encode_flush(gfp, mp3_flush_buffer, mp3_flush_buffer_size);

    char *mp3_file_path = malloc(MAX_PATH_LENGTH);
    strncpy(mp3_file_path, file_path, MAX_PATH_LENGTH);
    strncpy(mp3_file_path + strnlen(mp3_file_path, MAX_PATH_LENGTH) - 4, ".mp3", 4);

    FILE *mp3_file = fopen(mp3_file_path, "wb+");
#ifdef DEBUG
    printf("mp3_bytes %ld, mp3_file %ld\n", mp3_bytes, mp3_file);
#endif
    //printf("%d. WAV: \t'%s' encoded on thread %ld\n", encode_counter, file_path, (long) pthread_self());
    
    fwrite(mp3_buffer, 1, mp3_bytes, mp3_file);
    fwrite(mp3_flush_buffer, 1, mp3_flush_bytes, mp3_file);

    lame_mp3_tags_fid(gfp, mp3_file);

#ifdef DEBUG
    //determine mp3 file size
    fseek(mp3_file, 0, SEEK_END);
    long mp3_file_size = ftell(mp3_file);
    fseek(mp3_file, 0, SEEK_SET); //rewind(mp3_file);
    printf("\tmp3_file_path: '%s'\n", mp3_file_path);
    printf("\tmp3 file_size: %ld\n", mp3_file_size);
#endif

    fclose(wav_file);
    fclose(mp3_file);
    free(mp3_buffer);
    free(mp3_flush_buffer);
    free(pcm_data);
    free(mp3_file_path);
    lame_close(gfp);

    return 1;
}

#ifdef DEBUG
void printWavHeader(const wav_header_t *current_header) {
    //printf(ANSI_COLOR_YELLOW);
    printf("\nDEBUG wav header:"  "\n");
    printf("\tsizeof wav header: %ld\n", sizeof(wav_header_t));
    // RIFF Header
    printf("\triff_header: '%.4s'\n", current_header->riff_header);
    printf("\twav_size: '%d'\n", current_header->wav_size);
    printf("\twave_header: '%.4s'\n", current_header->wave_header);
    // Format Header
    printf("\tfmt_header: '%.4s' \n", current_header->fmt_header);
    printf("\tfmt_chunk_size: '%d'\n", current_header->fmt_chunk_size);
    printf("\taudio_format: '%d'\n", current_header->audio_format);
    printf("\tnum_channels: '%d'\n", current_header->num_channels);
    printf("\tsample_rate: '%d'\n", current_header->sample_rate);
    printf("\tbyte_rate: '%d'\n", current_header->byte_rate);
    printf("\tsample_alignment: '%d'\n", current_header->sample_alignment);
    printf("\tbit_depth: '%d'\n", current_header->bit_depth);
    //DATA
    printf("\tdata_header: '%.4s' \n", current_header->data_header);
    printf("\tdata_bytes: '%d'\n", current_header->data_bytes);
    printf("DEBUG wav header end\n\n");
    //printf(ANSI_COLOR_RESET);
}
#endif

int deallocate_wav_file_list() {
    current_wav_file = start_wav_file;
    int i=0;
    for(i=0; current_wav_file != NULL; i+=1) { //iterate through list
        wav_file_t *next_wav_file = current_wav_file->next;
        free(current_wav_file);
        current_wav_file = next_wav_file;
    }
#ifdef DEBUG
    printf("'%d' WAV file list entries deallocated\n", i);
#endif
    return 0;
}

#ifdef WIN32
int get_number_of_processors() {
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
}
#else
int get_number_of_processors() {
    return get_nprocs();
}
#endif