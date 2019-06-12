#include "main.h"

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

/*struct dirent *get_next_directory_entry() {
    pthread_mutex_lock(&lock_pDir);  //lock pDir

    struct dirent *pDirent = NULL;
    struct dirent *output = malloc(sizeof(struct dirent));
    pDirent = readdir(pDir); //iterate through files in directory
    output->d_ino = pDirent->d_ino;
    //output.d_name = pDirent->d_name;
    strncpy((output->d_name), pDirent->d_name, MAX_PATH_LENGTH);
    output->d_off = pDirent->d_off;
    output->d_reclen = pDirent->d_reclen;
    output->d_type = pDirent->d_type;
    //memcpy(pDirent->d_name, temp->d_name, MAX_FILEPATH_LENGTH);
    //memcpy(pDirent, temp, sizeof(struct dirent)); //TODO does this work ?
    
    pthread_mutex_unlock(&lock_pDir);
    return output;
}*/

/*int print_dir() {
    struct dirent *pDirent;
    for (int i = 0; (pDirent = readdir(pDir)) != NULL; i=i+1) { //iterate through files
        printf("i: %d\t", i);
        printf("'%s'\n", pDirent->d_name);
    }
    return 0;
}*/

//single-thread import, dirent.h readdir() causes certain bugs when multithreaded, even with locks
int import_wav_file_path_list(const char *directory_path) {
    //parameter must be correctly formatted and existing folder path
    //directory_path = malloc(strlen(argv[1]) + 1); //TODO create maximum directory and filepath lengths
    //strcpy(directory_path, argv[1]);
    DIR *pDir = opendir(directory_path);
    if (pDir == NULL) {
#ifdef ERRORS
        printf("Cannot open directory '%s'\n", directory_path);
        printf("errno %d: %s\n", errno, strerror(errno));
#endif
        return -1;
    }

//TODO wird speicher für file_path beim allokieren der struct auch schon allokiert ?

    struct dirent *pDirent;
    unsigned int file_counter = 0;
    for (int i = 0; (pDirent = readdir(pDir)) != NULL; i=i+1) { //iterate through files
        char *file_path = malloc(MAX_PATH_LENGTH);
        strncpy(file_path, directory_path, MAX_PATH_LENGTH);
        strncat(file_path, pDirent->d_name, MAX_PATH_LENGTH);
#ifdef DEBUG
        printf("%d. import\n", file_counter); //TODO turn .256s into preprocessor macro with max path length
#endif
        file_counter += 1;
        if(isWavFile(file_path)) {
            wav_file_t *new_wav_file = malloc(sizeof(wav_file_t));
#ifdef DEBUG
        //printf("sizeof(wav_file_t): '%ld'\n", sizeof(wav_file_t));
#endif
            new_wav_file->next = NULL;
            strncpy(new_wav_file->file_path, file_path, MAX_PATH_LENGTH);
            if(current_wav_file == NULL) { //begin list
                current_wav_file = new_wav_file;
                start_wav_file = new_wav_file;
            } else { //append to list
                current_wav_file->next = new_wav_file;
                current_wav_file = new_wav_file;
            }
            wav_file_counter += 1;
        } else {
            continue; //TODO aktuell unnötig
        }
    }

    //unsigned int file_path_malloc_size = 0;
    //char *file_path = malloc(file_path_malloc_size = (strlen(argv[1]) + strlen(file_name) + 1));
    //char *file_path = malloc(file_path_malloc_size = (strlen(directory_path) + strlen(file_name) + 1));

    //strcpy(file_path, directory_path);
    //strcat(file_path, file_name);
    closedir(pDir);
    current_wav_file = start_wav_file; //reset to start for reading
    printf("%d WAV files found / imported\n", wav_file_counter);
    return 0;
}

/*int deallocate_wav_file_list() {
    for(;;) { //iterate through list
        fclose();
        next = current.next;
        free(current);
    } 
}*/



char *get_next_wav_file_path() {
    //FILE *wav_file = fopen(current_wav_file->file_path, "r");
    if(current_wav_file == NULL) {
        return NULL;
    }
    char *output = current_wav_file->file_path;
    current_wav_file = current_wav_file->next;
    return output;
}

void *encoding_thread_function(void *data_unused) {
    printf("encode thread function on thread %ld\n", (long) pthread_self());
    //struct dirent *pDirent = NULL;
    //FILE *wav_file = NULL;
    int i = 0; //per-thread local encode counter

    while(1) { //infinite loop that polls for wav Files to be encoded
        pthread_mutex_lock(&lock_pDir);
        //FILE *wav_file = fopen(get_next_wav_file_path(), "r");
        char *wav_file_path = get_next_wav_file_path();
        pthread_mutex_unlock(&lock_pDir);

        if (wav_file_path == NULL) { //end of directory reached
            printf("thread %ld local encode counter: %d\n", (long) pthread_self(), i);
            //pthread_exit(NULL);
            return NULL; //TODO is it a problem that this doesnt get called ? if pthread exit
        } else {
            //unsigned int file_name_malloc_size = 0; //malloc size variables for debug
            //char *file_name;// = malloc(file_name_malloc_size = (strlen(pDirent->d_name) + 1)); //TODO watch out for accidental pointer addition
            //file_name = malloc(MAX_PATH_LENGTH); //TODO watch out for accidental pointer addition
            //strcpy(file_name, pDirent->d_name);

            //printf("Thread ID is: %ld", (long) pthread_self());
            int encode_successful = encodeWavFile(wav_file_path);
            if (encode_successful) {
                pthread_mutex_lock(&lock_encode_counter);
                encode_counter += 1; //TODO only do this once at the end, for performance, aggregate all local thread counters

                pthread_mutex_unlock(&lock_encode_counter);
            } else {

            }
            i=i+1; //local, perthread encode counter

            //free(file_name); //TODO one large, permanent array for filename is better for performance
        }
        //fclose(wav_file);
    } //end while(1)

        


    //return NULL;
    return 0;
}

int isWavFile(const char *file_path) {
    //TODO filepath is incorrect when concatenating to a folder path thats not ending in slash
#ifdef DEBUG
    printf("\tfile_path: '%.256s'\n", file_path); 
    printf("\tstrlen(file_path): %ld\n", strlen(file_path));
    //printf ("directory_path: %s    file_name: %s\n", directory_path, file_name);
    //printf ("strlen(directory_path): %ld    strlen(file_name): %ld\n", strnlen(directory_path, MAX_PATH_LENGTH), strnlen(file_name, MAX_PATH_LENGTH)); //TODO does strnlen work ?
#endif

    FILE *current_file = fopen(file_path, "r");

    //determine file size
    fseek(current_file, 0, SEEK_END);
    long file_size = ftell(current_file);
    fseek(current_file, 0, SEEK_SET); //rewind(wav_file);
#ifdef DEBUG
    printf("\tfile_size: %ld\n", file_size);
#endif

    int last4chars_correct = 0; //can not be -1, because that is boolean true

    int header_correct = 0;
    wav_header_t current_header;

    //filter out "." and "..", anything that can't be ".wav" // TODO move this into isWavFile()

    //TODO make properly case insensitive
    last4chars_correct = strlen(file_path) >= 4 && (strncmp(".wav", (file_path + strlen(file_path) - 4), 4) == 0 || strncmp(".WAV", (file_path + strlen(file_path) - 4), 4) == 0);
    if(!last4chars_correct) {
        return 0;
    }
    
    if(file_size > sizeof(wav_header_t)) {
        int read_header = fread(&current_header, sizeof(wav_header_t), 1, current_file);
        if (read_header != 1) { //exactly one element of size 44 bytes must be read
#ifdef ERRORS
            printf(ANSI_COLOR_RED "Error while reading wav header\n" ANSI_COLOR_RESET);
#endif
            return 0; //TODO, cannot be -1 because of connection to encodeCounter
        } else {
            //printf(ANSI_COLOR_YELLOW "\nNo Error while reading wav header\n" ANSI_COLOR_RESET);
            header_correct = (0 == strncmp("WAVE", current_header.wave_header, 4)) && (0 == strncmp("RIFF", current_header.riff_header, 4));
        }
    }
#ifdef DEBUG
    printf("\tlast4chars_correct: %d header_correct: %d\n", last4chars_correct, header_correct);
#endif
    //rewind(current_file); //TODO incorrect, reset to position that it had at beginning of isWavfile instead of start of file
    fclose(current_file);
    return (last4chars_correct && header_correct); // ? 0 : 1;
}

int encodeWavFile(const char *file_path) { //return number of successful encodes
    wav_header_t current_header; //TODO header gets read twice, for wav test and for encode
    FILE *wav_file = fopen(file_path, "r");
    
    //determine file size
    fseek(wav_file, 0, SEEK_END);
    long file_size = ftell(wav_file);
    fseek(wav_file, 0, SEEK_SET); //rewind(wav_file);

#ifdef DEBUG
    printf("%d. encode\n", encode_counter);
    printf("\twav file_path: '%.256s'\n", file_path);
    printf("\twav file_size: %ld\n", file_size);
#endif
    unsigned int pcm_data_malloc_size = (file_size - sizeof(wav_header_t));
    short int *pcm_data = malloc(pcm_data_malloc_size);
    if(file_size > sizeof(wav_header_t)) {
        int read_header = fread(&current_header, sizeof(wav_header_t), 1, wav_file); //TODO order middle two parameters
        if (read_header != 1) { //exactly one header element of size 44 bytes must be read
#ifdef ERRORS
            printf(ANSI_COLOR_RED "\nError while reading wav header\n" ANSI_COLOR_RESET);
#endif
            return 0; //exit if wav header wasn't read correctly
        }
    } else {
        return 0; //exit if file is smaller than wav header
    }

#ifdef DEBUG
    //printWavHeader(&current_header);
#endif

    int read_pcm = fread(pcm_data, 1, (file_size - sizeof(wav_header_t)), wav_file);
    if (read_pcm != (file_size - sizeof(wav_header_t))) {
#ifdef ERRORS
        printf(ANSI_COLOR_RED "\nError while reading pcm data\n" ANSI_COLOR_RESET);
#endif
        return 0;
    }
    //char *pcm_buffer = malloc(file_size - sizeof(wav_header)); //wav header is 44 bytes long
    //remove header, 44 bytes, You can usually convert a .wav file into a .pcm file by simply stripping off the first 44 bytes which is just a header with sample rate info etc. They header length can vary, but I've only seen 44 byte headers.
    //TODO is the header guaranteed to be 44 bytes ? find better way to load pcm from wav

    //int num_samples = current_header.data_bytes / (current_header.num_channels * (current_header.bit_depth/8)); //number of samples per channel
    int num_samples = current_header.data_bytes / (2 * (current_header.bit_depth/8)); //must be 2 channels, because i'm using encode_interleaved

    //LAME API, every file has different settings and needs its own gfp
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
#ifdef ERRORS
        printf(ANSI_COLOR_RED "\nlame_init_params() ret code error\n" ANSI_COLOR_RESET);
#endif
    }

    long int mp3_buffer_size = 1.25*num_samples + 7200; //worst case estimate according to lame/API
    unsigned int mp3_buffer_malloc_size = 0;
    unsigned char *mp3_buffer = malloc(mp3_buffer_malloc_size = mp3_buffer_size);

    int mp3_bytes = lame_encode_buffer_interleaved(gfp, pcm_data, num_samples, mp3_buffer, mp3_buffer_size); //TODO is this a problem with mono ?, there was an issue with num_samples formula when using num_channels==1

    if (mp3_bytes < 0) {
#ifdef ERRORS
        printf(ANSI_COLOR_RED "\nlame encode error\n" ANSI_COLOR_RESET);
#endif
    }

    unsigned int mp3_flush_buffer_size = 7200;
    unsigned int mp3_flush_buffer_malloc_size = 0;
    unsigned char *mp3_flush_buffer = malloc(mp3_flush_buffer_malloc_size = mp3_flush_buffer_size);

    int mp3_flush_bytes = lame_encode_flush(gfp, mp3_flush_buffer, mp3_flush_buffer_size);

    unsigned int mp3_file_path_malloc_size = 0;
    char *mp3_file_path = malloc(mp3_file_path_malloc_size = (strlen(file_path) + 1));
    strcpy(mp3_file_path, file_path);
    strncpy(mp3_file_path + strlen(mp3_file_path) - 4, ".mp3", 4);


    FILE *mp3_file = fopen(mp3_file_path, "w+"); //TODO hats the difference when using wb+ ? or other mode, only difference on systems that differentiate binary and text files ?
#ifdef DEBUG
    //printf("%d\n", mp3_file);
    //rewind(mp3_file); //TODO necessary ?
    //printf("8\n");
    //printf("mp3_buffer %ld, mp3_bytes %ld, mp3_file %ld\n", mp3_buffer, mp3_bytes, mp3_file);
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

    printf("sizeof(wav_header_t): %ld\n", sizeof(wav_header_t));
    printf("sizeof(wav_file_t): %ld\n", sizeof(wav_file_t));
#endif

    if ((argc != 2) || (argv[1] == NULL)) {
        printf ("Incorrect Usage\n"); //TODO check if parameter is actually directory
        printf ("Usage: ./main.out <dirname>\n");
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
        return 1;
    }
    if (pthread_mutex_init(&lock_pDir, NULL) != 0) {
#ifdef ERRORS
        printf("\n lock_pDir mutex init has failed\n");
#endif
        return 1; 
    }

    unsigned int num_cores = get_nprocs(); //TODO robust enough ?
#ifdef PREVENT_LAG
    unsigned int num_threads = num_cores-2; // //leave 1 core free to prevent lag (terminal output etc) and use only one thread per core, in addition to main thread
#else
    unsigned int num_threads = num_cores*2; //2 threads per core
#endif

    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    printf("Number of Cores: %d\n", num_cores); 
    printf("Number of Threads: %d\n", num_threads); //TODO detect if there are two hardware threads per core or not, print out processor model etc

    //thread start parameters
    void *(*functionPtr) (void *);
    functionPtr = &encoding_thread_function;
    void *data_unused = NULL;
    //encoding_thread_parameters_t thread_params; //unused

    //start all threads
    for (int i=0; i<num_threads; i=i+1) {
        //int pthread_create_error = pthread_create(&(threads[i]), NULL, functionPtr, (void*)&(thread_params));
        int pthread_create_error = pthread_create(&(threads[i]), NULL, functionPtr, data_unused);
        if(pthread_create_error != 0) {
#ifdef ERRORS
            printf("\nThread %d can't be created :[%s]\n", i, strerror(pthread_create_error));
#endif
        }
    }
    //wait for all threads to end
    for (int i=0; i<num_threads; i=i+1) {
        int pthread_join_error = pthread_join(threads[i], NULL); //TODO rückgabewert ?
        if(pthread_join_error != 0) {
#ifdef ERRORS
            printf("\nThread %d can't be joined :[%s]\n", i, strerror(pthread_join_error));
#endif
        }
    }
    
    //TODO return argument from thread ?
    //TODO is close/free order important ?

    pthread_mutex_destroy(&lock_encode_counter);
    pthread_mutex_destroy(&lock_pDir);

    free(threads);

    printf ("\n'%d' WAV files encoded to MP3.\n", encode_counter);
    printf("Exit.\n");
    return 0;
}