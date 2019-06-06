#include "main.h"

void encodeFile(const char *file_path) {
    
}

int main(int argc, char **argv) {
#ifdef DEBUG
    printf(ANSI_COLOR_YELLOW);
    printf("\nDEBUG main args:\n");
    printf("\targc: '%d'\n", argc);
    for (int i=0; i < argc; i=i+1) {
        printf("\targv[%d]: '%s'\n", i, argv[i]);
    }
    printf("DEBUG main args end\n\n");
    printf(ANSI_COLOR_RESET);
#endif

    if ((argc != 2) || (argv[1] == NULL)) {
        printf ("Incorrect Usage\n"); //TODO check if parameter is actually directory
        printf ("Usage: main <dirname>\n");
        printf ("Example: main ./wavFiles/\n");
        return -1;
    }

    //listDirectory(argv[1]);


    struct dirent *pDirent;
    DIR *pDir;

    pDir = opendir (argv[1]);

    //TODO read number of channels etc from wav file


    if (pDir == NULL) {
        printf ("Cannot open directory '%s'\n", argv[1]);
        printf("errno %d: %s\n", errno, strerror(errno));
        return 1;
    }
    int encode_counter = 0;



    for (int i = 0; (pDirent = readdir(pDir)) != NULL; i=i+1) { //iterate through files
        //printf ("'%s'\n", pDirent->d_name);

        char *file_name = malloc(strlen(pDirent->d_name + 1));
        strcpy(file_name, pDirent->d_name);

        if(strlen(file_name) <= 4) { //filter out "." and ".." + anything that can't be ".wav"
            continue;
        }
#ifdef DEBUG
        printf ("'%s'\n", file_name);
#endif
        char *file_path = malloc(strlen(argv[1]) + strlen(file_name) + 1);
        strcpy(file_path, argv[1]);
        strcat(file_path, file_name);

        
        //printf("last char: '%c' int: %d\n", file_path[strlen(file_path)], file_path[strlen(file_path)]);

        //TODO accepts files only based on last 4 chars as WAV files, not guaranteed to work
        int wav = strncmp(".wav", (file_name + strlen(file_name) - 4), 4) == 0 || strncmp(".WAV", (file_name + strlen(file_name) - 4), 4) == 0;
        
        //TODO compare from file header as well
        //TODO make properly case sensitive

        if (wav) {
            printf ("%d. WAV In: \t'%s'\n", encode_counter, file_path);
            FILE *wav_file = fopen(file_path, "r");
            wav_header current_header; // = malloc(sizeof(wav_header));
            
            //determine file size
            fseek(wav_file, 0, SEEK_END);
            long file_size = ftell(wav_file);
            fseek(wav_file, 0, SEEK_SET); //rewind(wav_file);
            short int *pcm_data = malloc(file_size - sizeof(wav_header));

            int read_header = fread(&current_header, sizeof(wav_header), 1, wav_file);
            if (read_header != 1) { //exactly one element of size 44 bytes must be read
                printf(ANSI_COLOR_RED "\nError while reading wav header\n" ANSI_COLOR_RESET);
            }
            if (strncmp("WAVE", current_header.wave_header, 4) != 0 || strncmp("RIFF", current_header.riff_header, 4) != 0) {

            }

#ifdef DEBUG
            printf(ANSI_COLOR_YELLOW);
            printf("\nDEBUG wav header:"  "\n");
            printf("\tsizeof wav header: %ld\n", sizeof(wav_header));
            // RIFF Header
            printf("\triff_header: '%.4s'\n", current_header.riff_header);
            printf("\twav_size: '%d'\n", current_header.wav_size);
            printf("\twave_header: '%.4s'\n", current_header.wave_header);
            // Format Header
            printf("\tfmt_header: '%.4s' \n", current_header.fmt_header);
            printf("\tfmt_chunk_size: '%d'\n", current_header.fmt_chunk_size);
            printf("\taudio_format: '%d'\n", current_header.audio_format);
            printf("\tnum_channels: '%d'\n", current_header.num_channels);
            printf("\tsample_rate: '%d'\n", current_header.sample_rate);
            printf("\tbyte_rate: '%d'\n", current_header.byte_rate);
            printf("\tsample_alignment: '%d'\n", current_header.sample_alignment);
            printf("\tbit_depth: '%d'\n", current_header.bit_depth);
            //DATA
            printf("\tdata_header: '%.4s' \n", current_header.data_header);
            printf("\tdata_bytes: '%d'\n", current_header.data_bytes);
            printf("DEBUG wav header end\n\n");
            printf(ANSI_COLOR_RESET);
#endif
            int read_pcm = fread(pcm_data, 1, (file_size - sizeof(wav_header)), wav_file);
            if (read_pcm != (file_size - sizeof(wav_header))) {
                printf(ANSI_COLOR_RED "\nError while reading pcm data\n" ANSI_COLOR_RESET);
            }
            //char *pcm_buffer = malloc(file_size - sizeof(wav_header)); //wav header is 44 bytes long
            //remove header, 44 bytes, You can usually convert a .wav file into a .pcm file by simply stripping off the first 44 bytes which is just a header with sample rate info etc. They header length can vary, but I've only seen 44 byte headers.
            //TODO is the header guaranteed to be 44 bytes ? find better way to load pcm from wav

            //int num_samples = current_header.data_bytes / (current_header.num_channels * (current_header.bit_depth/8)); //number of samples per channel
            int num_samples = current_header.data_bytes / (2 * (current_header.bit_depth/8)); //must be 2 channels, because i'm using encode_interleaved

            //LAME API
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
            unsigned char *mp3_buffer = malloc(mp3_buffer_size);

            int mp3_bytes = lame_encode_buffer_interleaved(gfp, pcm_data, num_samples, mp3_buffer, mp3_buffer_size); //TODO is this a problem with mono ?, there was an issue with num_samples formula when using num_channels==1

            if (mp3_bytes < 0) {
                printf(ANSI_COLOR_RED "\nlame encode error\n" ANSI_COLOR_RESET);
            }

            long int mp3_flush_buffer_size = 7200;
            unsigned char *mp3_flush_buffer = malloc(mp3_flush_buffer_size);

            int mp3_flush_bytes = lame_encode_flush(gfp, mp3_flush_buffer, mp3_flush_buffer_size);

            char *mp3_file_path = malloc(strlen(file_path) + 1);
            strcpy(mp3_file_path, file_path);
            strncpy(mp3_file_path + strlen(mp3_file_path) - 4, ".mp3", 4);

            FILE *mp3_file = fopen(mp3_file_path, "w+"); //TODO hats the difference when using wb+ ? or other mode


            fwrite(mp3_buffer, 1, mp3_bytes, mp3_file);
            fwrite(mp3_flush_buffer, 1, mp3_flush_bytes, mp3_file);

            lame_mp3_tags_fid(gfp, mp3_file);
            fclose(mp3_file);

            free(mp3_buffer);
            free(mp3_flush_buffer);

            printf("%d. MP3 Out: \t'%s'\n", encode_counter, mp3_file_path);
            encode_counter += 1;

            fclose(wav_file);
            lame_close(gfp);
        }
        free(file_name);
        free(file_path);
    } //end for
    printf ("\n'%d' WAV files encoded to MP3.\n", encode_counter);

    closedir (pDir);
    


    //detectNumberOfCores();
    //verify parameters
    //correct number
    //parameter must be correctly formatted and existing folder path

    printf("Exit.\n");
    return 0;
}