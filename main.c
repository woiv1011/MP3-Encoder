#include "main.h"

#ifdef DEBUG
void printWavHeader(const wav_header_t *current_header) {
    printf(ANSI_COLOR_YELLOW);
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
    printf(ANSI_COLOR_RESET);
}
#endif

void *encoding_thread_function(void *data) {
    char *file_path_local = ((encoding_thread_parameters_t*)data)->file_path;
    FILE *wav_file_local = ((encoding_thread_parameters_t*)data)->wav_file;
#ifdef DEBUG
    DEBUG_INFO;
    printf("file_path: %s length: %ld\n", file_path_local, strlen(file_path_local));
#endif
    printf ("%d. WAV In: \t'%s'\n", encode_counter, file_path_local);
    //printf("wav_file: %d\n", wav_file_local);
    int encode_successful = encodeWavFile(file_path_local, wav_file_local);
    if (encode_successful == 1) { //TODO
        //lock
        encode_counter += 1;
        //unlock
    }
}

int isWavFile(const char *file_path, FILE *current_file) {
    //TODO accepts files only based on last 4 chars as WAV files, not guaranteed to work
    //determine file size
    fseek(current_file, 0, SEEK_END);
    long file_size = ftell(current_file);
    fseek(current_file, 0, SEEK_SET); //rewind(wav_file);
#ifdef DEBUG
    printf("file_size: %ld  sizeof wav_header: %ld\n", file_size, sizeof(wav_header_t));
#endif

    int last4chars_correct = 0; //can not be -1, because that is boolean true

    int header_correct = 0;
    wav_header_t current_header;

    last4chars_correct = strncmp(".wav", (file_path + strlen(file_path) - 4), 4) == 0 || strncmp(".WAV", (file_path + strlen(file_path) - 4), 4) == 0;
    if(file_size > sizeof(wav_header_t)) {
        int read_header = fread(&current_header, sizeof(wav_header_t), 1, current_file);
        if (read_header != 1) { //exactly one element of size 44 bytes must be read
            printf(ANSI_COLOR_RED "\nError while reading wav header\n" ANSI_COLOR_RESET);
            return 0; //TODO, cannot be -1 because of connection to encodeCounter
        } else {
            //printf(ANSI_COLOR_YELLOW "\nNo Error while reading wav header\n" ANSI_COLOR_RESET);
            header_correct = strncmp("WAVE", current_header.wave_header, 4) == 0 && strncmp("RIFF", current_header.riff_header, 4) == 0;
        }
    }
#ifdef DEBUG
    printf("\t\tisWavFile()\n");
    printf("\t\tfile_path: %s length: %ld\n", file_path, strlen(file_path));
    printf("\t\tlast4chars_correct: %d header_correct: %d\n", last4chars_correct, header_correct);
#endif
    return last4chars_correct && header_correct;
}

int encodeWavFile(const char *file_path, FILE *wav_file) { //return number of successful encodes
    wav_header_t current_header; //TODO header gets read twice, for wav test and for encode
    
    //determine file size
    fseek(wav_file, 0, SEEK_END);
    long file_size = ftell(wav_file);
    fseek(wav_file, 0, SEEK_SET); //rewind(wav_file);

#ifdef DEBUG
    printf("file_size: %ld  sizeof wav_header: %ld\n", file_size, sizeof(wav_header_t));
#endif
    unsigned int pcm_data_malloc_size = (file_size - sizeof(wav_header_t));
    short int *pcm_data = malloc(pcm_data_malloc_size);
    if(file_size > sizeof(wav_header_t)) {
        int read_header = fread(&current_header, sizeof(wav_header_t), 1, wav_file); //TODO order middle two parameters
        if (read_header != 1) { //exactly one element of size 44 bytes must be read
            printf(ANSI_COLOR_RED "\nError while reading wav header\n" ANSI_COLOR_RESET);
            return 0; //TODO, cannot be -1 because of connection to encodeCounter
        }
    }
    else {
        return 0;
    }

#ifdef DEBUG
    printWavHeader(&current_header);
#endif

    int read_pcm = fread(pcm_data, 1, (file_size - sizeof(wav_header_t)), wav_file);
    if (read_pcm != (file_size - sizeof(wav_header_t))) {
        printf(ANSI_COLOR_RED "\nError while reading pcm data\n" ANSI_COLOR_RESET);
        return 0;
    }
    //char *pcm_buffer = malloc(file_size - sizeof(wav_header)); //wav header is 44 bytes long
    //remove header, 44 bytes, You can usually convert a .wav file into a .pcm file by simply stripping off the first 44 bytes which is just a header with sample rate info etc. They header length can vary, but I've only seen 44 byte headers.
    //TODO is the header guaranteed to be 44 bytes ? find better way to load pcm from wav

    //int num_samples = current_header.data_bytes / (current_header.num_channels * (current_header.bit_depth/8)); //number of samples per channel
    int num_samples = current_header.data_bytes / (2 * (current_header.bit_depth/8)); //must be 2 channels, because i'm using encode_interleaved

    //LAME API, every file has different settings, needs its own gfp
    lame_global_flags *gfp;
    gfp = lame_init();

    lame_set_num_samples(gfp, num_samples);
    lame_set_num_channels(gfp, current_header.num_channels);
    lame_set_in_samplerate(gfp, current_header.sample_rate);

    //lame_set_out_samplerate(gfp, 0); //default = 0; lame picks best value
    lame_set_brate(gfp, 192); //TODO scale of int value
    lame_set_quality(gfp, 5);   // 5 = good quality, fast

    int ret_code = lame_init_params(gfp); //TODO Check that ret_code >= 0.

    if (ret_code < 0) {
        printf(ANSI_COLOR_RED "\nlame_init_params() ret code error\n" ANSI_COLOR_RESET);
    }

    long int mp3_buffer_size = 1.25*num_samples + 7200; //worst case estimate according to lame/API
    unsigned int mp3_buffer_malloc_size = 0;
    unsigned char *mp3_buffer = malloc(mp3_buffer_malloc_size = mp3_buffer_size);

    int mp3_bytes = lame_encode_buffer_interleaved(gfp, pcm_data, num_samples, mp3_buffer, mp3_buffer_size); //TODO is this a problem with mono ?, there was an issue with num_samples formula when using num_channels==1

    if (mp3_bytes < 0) {
        printf(ANSI_COLOR_RED "\nlame encode error\n" ANSI_COLOR_RESET);
    }

    long int mp3_flush_buffer_size = 7200;
    unsigned int mp3_flush_buffer_malloc_size = 0;
    unsigned char *mp3_flush_buffer = malloc(mp3_flush_buffer_malloc_size = mp3_flush_buffer_size);

    int mp3_flush_bytes = lame_encode_flush(gfp, mp3_flush_buffer, mp3_flush_buffer_size);

    unsigned int mp3_file_path_malloc_size = 0;
    char *mp3_file_path = malloc(mp3_file_path_malloc_size = (strlen(file_path) + 1));
    strcpy(mp3_file_path, file_path);
    strncpy(mp3_file_path + strlen(mp3_file_path) - 4, ".mp3", 4);
#ifdef DEBUG
    printf("file_path: %s\n", file_path);
    printf("mp3_file_path: %s\n", mp3_file_path);
#endif

    FILE *mp3_file = fopen(mp3_file_path, "w+"); //TODO hats the difference when using wb+ ? or other mode, only difference on systems that differentiate binary and text files ?
#ifdef DEBUG
    printf("%d\n", mp3_file);
    //rewind(mp3_file); //TODO necessary ?
    printf("8\n");
    printf("mp3_buffer %ld, mp3_bytes %ld, mp3_file %ld\n", mp3_buffer, mp3_bytes, mp3_file);
#endif
    fwrite(mp3_buffer, 1, mp3_bytes, mp3_file);
#ifdef DEBUG
    printf("between mp3 writes\n");
#endif
    fwrite(mp3_flush_buffer, 1, mp3_flush_bytes, mp3_file);

#ifdef DEBUG
    printf("after mp3 writes\n");
#endif

    lame_mp3_tags_fid(gfp, mp3_file);

    fclose(mp3_file);

    free(mp3_buffer);
    free(mp3_flush_buffer);
    free(pcm_data);

    lame_close(gfp);

    return 1;
}

int main(int argc, char **argv) {
#ifdef DEBUG
    DEBUG_INFO;
    printf(ANSI_COLOR_YELLOW);
    //printf("main args:\n");
    printf("\targc: '%d'\n", argc);
    for (int i=0; i < argc; i=i+1) {
        printf("\targv[%d]: '%s'\n", i, argv[i]);
    }
    printf("DEBUG main args end\n\n");
    printf(ANSI_COLOR_RESET);
#endif

    if ((argc != 2) || (argv[1] == NULL)) {
        printf ("Incorrect Usage\n"); //TODO check if parameter is actually directory
        printf ("Usage: ./main.out <dirname>\n");
        printf ("Example: ./main.out ./wavFiles/\n");
        return -1;
    }

    struct dirent *pDirent;
    DIR *pDir;
    pDir = opendir (argv[1]);

    if (pDir == NULL) {
        printf ("Cannot open directory '%s'\n", argv[1]);
        printf("errno %d: %s\n", errno, strerror(errno));
        return 1;
    }
    
    //int encode_counter = 0;

    char *file_name = NULL;
    char *file_path = NULL;

    for (int i = 0; (pDirent = readdir(pDir)) != NULL; i=i+1) { //iterate through files
#ifdef DEBUG
        printf("start i: %d\n", i);
        printf ("'%s'\n", pDirent->d_name);
#endif

        unsigned int file_name_malloc_size = 0;
        file_name = malloc(file_name_malloc_size = (strlen(pDirent->d_name) + 1)); //TODO watch out for accidental pointer addition
        strcpy(file_name, pDirent->d_name);
#ifdef DEBUG
        printf("file_name: '%s' length: '%ld'\n", file_name, strlen(file_name));
#endif
        if(strlen(file_name) <= 4) { //filter out "." and "..", anything that can't be ".wav"
            free(file_name);
            continue;
        }

        unsigned int file_path_malloc_size = 0;
        file_path = malloc(file_path_malloc_size = (strlen(argv[1]) + strlen(file_name) + 1));
#ifdef DEBUG
        printf ("argv[1]: %s    file_name: %s\n", argv[1], file_name);
        printf ("strlen(argv[1]): %ld    strlen(file_name): %ld\n", strlen(argv[1]), strlen(file_name));
#endif
        strcpy(file_path, argv[1]);
        strcat(file_path, file_name);

        //TODO filepath is incorrect when concatenating to a folder path thats not ending in slash

        
        //printf("last char: '%c' int: %d\n", file_path[strlen(file_path)], file_path[strlen(file_path)]);

        
        
        int wav = 0;
        //TODO compare from file header as well
        //TODO make properly case sensitive
        FILE *wav_file = fopen(file_path, "r");
#ifdef DEBUG
        printf("wav_file: %d", (wav_file)); //TODO always the same pointer address ?
#endif
        wav = isWavFile(file_path, wav_file);
        rewind(wav_file);

        if (wav) {
            pthread_t thread_1;
            unsigned int num_cores = get_nprocs(); //1
            pthread_t threads[num_cores*2]; //2 threads per core
            //pthread_attr_t thread_1_attributes = NULL;
            //void (*functionPtr)(const char *);
            void *(*functionPtr) (void *);
            functionPtr = &encoding_thread_function;

            //pthread_create(&thread_1, 0, functionPtr, file_path);
            //pthread_create(&thread_1, thread_1_attributes, functionPtr, file_path); //thread attributes type problem

            encoding_thread_parameters_t temp;
            temp.file_path = file_path;
            temp.wav_file = wav_file;
            
            //encode_counter += 
            //TODO return argument from thread gets added to encode counter
#ifdef PRINT
            printf ("%d. WAV In: \t'%s'\n", encode_counter, file_path);
#endif
            pthread_create(&thread_1, NULL, functionPtr, (void*)&temp);
            pthread_join(thread_1, NULL);
            //int encode_successful = encodeWavFile(file_path, wav_file);
            
            //printf("%d. MP3 Out: \t'%s'\n", encode_counter);//, mp3_file_path); //segfault if there is no parameter for %s
        }
        fclose(wav_file);
        //printf("\tfile_path: %s length: %d\n", file_path, strlen(file_path));
        if (file_name != NULL) {
            free(file_name);
        }
        if (file_path != NULL) {
            free(file_path);
        }
#ifdef DEBUG
        printf("end i: %d\n", i);
#endif
    } //end for
    printf ("\n'%d' WAV files encoded to MP3.\n", encode_counter);

    closedir (pDir);
    


    //detectNumberOfCores();
    //verify parameters
    //correct number
    //parameter must be correctly formatted and existing folder path

    printf("Exit.\n");
    return 0; //if you return encode_counter its seen as an error/failure
}