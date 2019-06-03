
#define DEBUG
#include "main.h"

/*short int *extract_PCM_from_WAV(const char * path) {
    //loads wav file and returns contained PCM
    FILE  *input = fopen(path, "r");
}*/

void loadFile() {
    //get File size
    //allocate memory
    //open file
    //read file into memory
    //close file

    //TODO safety checks
}

/*int listDirectory(char *path) {
    //int len;
    struct dirent *pDirent;
    DIR *pDir;

    pDir = opendir (path);

    if (pDir == NULL) {
        printf ("Cannot open directory '%s'\n", path);
        printf("errno %d: %s\n", errno, strerror(errno));
        return 1;
    }

    while ((pDirent = readdir(pDir)) != NULL) {
        printf ("'%s'\n", pDirent->d_name);
    }
    closedir (pDir);

    return 0;
}*/


int main(int argc, char **argv) {
#if defined(DEBUG) && defined(DEBUG_VERBOSE)
    printf("argc: '%d'\n", argc);
    for (int i=0; i < argc; i=i+1) {
        printf("argv[%d]: '%s'\n", i, argv[i]);
    }
    printf("\n");
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

    //char *current_file = malloc(strlen("s"));
    //FILE  *current = fopen(file_1, "r");

    //TODO read number of channels etc from wav file


    if (pDir == NULL) {
        printf ("Cannot open directory '%s'\n", argv[1]);
        printf("errno %d: %s\n", errno, strerror(errno));
        return 1;
    }

    for (int i = 0; (pDirent = readdir(pDir)) != NULL; i=i+1) { //iterate through files
        //printf ("'%s'\n", pDirent->d_name);

        char *file_name = malloc(strlen(pDirent->d_name + 1));
        strcpy(file_name, pDirent->d_name);
#ifdef DEBUG
        printf ("'%s'\n", file_name);
#endif
        char *file_path = malloc(strlen(argv[1]) + strlen(file_name) + 1);
        strcpy(file_path, argv[1]);
        strcat(file_path, file_name);
        //strcpy(file_path + strlen(argv[1]), file_name); //TODO does strpcy automatically add "\0" after the copied string in dest ?

        printf ("Full Path: '%s'\n", file_path);
        //printf("last char: '%c' int: %d\n", file_path[strlen(file_path)], file_path[strlen(file_path)]);

        char *last4chars = file_name + strlen(file_name) - 4; //TODO or 5, 4 because string end doesnt get counted

#ifdef DEBUG
        //printf ("'%s'\n", last4chars);
#endif

        int wav = strcmp(".wav", last4chars) == 0 || strcmp(".WAV", last4chars) == 0; //TODO accepts files only based on last 4 chars as WAV files, not guaranteed to work
        
        //TODO compare from file header as well
        //TODO make properly case sensitive



        if (wav) {
            FILE *current_file = fopen(file_path, "r");
            wav_header current_header; // = malloc(sizeof(wav_header));
            

            //determine file size
            fseek(current_file, 0, SEEK_END);
            long file_size = ftell(current_file);
            fseek(current_file, 0, SEEK_SET); //rewind(current_file);
            char *pcm_data = malloc(file_size - sizeof(wav_header));

            int read_header = fread(&current_header, sizeof(wav_header), 1, current_file);

#ifdef DEBUG
            printf("sizeof wav header: %ld\n", sizeof(wav_header));
            // RIFF Header
            printf("riff_header: '%.4s'\n", current_header.riff_header);
            printf("riff_header: '%c''%c''%c''%c'\n", current_header.riff_header[0],current_header.riff_header[1],current_header.riff_header[2],current_header.riff_header[3]);
            printf("wav_size: %d\n", current_header.wav_size);
            printf("wave_header: %.4s \n", current_header.wave_header);

            // Format Header
            printf("fmt_header: %.4s \n", current_header.fmt_header);
            printf("fmt_chunk_size: %d\n", current_header.fmt_chunk_size);
            printf("audio_format: %d\n", current_header.audio_format);
            printf("num_channels: %d\n", current_header.num_channels);
            printf("sample_rate: %d\n", current_header.sample_rate);
            printf("byte_rate: %d\n", current_header.byte_rate);
            printf("sample_alignment: %d\n", current_header.sample_alignment);
            printf("bit_depth: %d\n", current_header.bit_depth);

            //DATA
            printf("data_header: %.4s \n", current_header.data_header);
            printf("data_bytes: %d\n", current_header.data_bytes);
            /*for (int j=0; j<= 43; j+=1) {
                printf("temp: '%44c' \n", current_header.temp);
            }*/
            
#endif

            int read_pcm = fread(pcm_data, 1, (file_size - sizeof(wav_header)), current_file);

            char *file_buffer = malloc(file_size - sizeof(wav_header)); //wav header is 44 bytes long

            //remove header, 44 bytes, You can usually convert a .wav file into a .pcm file by simply stripping off the first 44 bytes which is just a header with sample rate info etc. They header length can vary, but I've only seen 44 byte headers.
            //TODO is the header guaranteed to be 44 bytes ? find better way to load pcm from wav

            //loadAndEncode(file_path);
            //current_file = 
            //open and load file
            //encode file

            int num_samples = current_header.data_bytes / (current_header.num_channels * (current_header.bit_depth/8)); //number of samples per channel
            char *mp3_buffer = malloc(1000000);
            int mp3_buffer_size = 1000000;

            lame_global_flags *gfp;
            int mp3_bytes = lame_encode_buffer_interleaved(gfp, pcm_data, num_samples, mp3_buffer, mp3_buffer_size);
            fclose(current_file);
            free(mp3_buffer);
#ifdef DEBUG
            break;
#endif
        }

    } //end for
    closedir (pDir);
    

    //LAME API
#ifdef DEBUG
    //printf("%s\n", get_lame_version());
#endif
    




    //TODO encodeFile()
    //3. Initialize the encoder.  sets default for all encoder parameters.
    
    //gfp = lame_init();

    //3.1 more settings, siehe lame API bzw. lame.h



    //4. Set more internal configuration based on data provided above,
    //as well as checking for problems.  Check that ret_code >= 0.
    //int ret_code = lame_init_params(gfp);

    //loads wav file and returns contained PCM
    

    //short int *left_pcm = NULL;
    //short int *right_pcm = NULL;
    /*short int *right_pcm = NULL;
    

    //5. Encode some data
    /*int mp3_bytes = lame_encode_buffer(gfp,
        left_pcm,
        right_pcm,
        num_samples,
        mp3buffer,
        mp3buffer_size);*/




    //int CDECL lame_encode_buffer (
    //    lame_global_flags*  gfp,           /* global context handle         */
    //    const short int     buffer_l [],   /* PCM data for left channel     */
    //    const short int     buffer_r [],   /* PCM data for right channel    */
    //    const int           nsamples,      /* number of samples per channel */
    //    unsigned char*      mp3buf,        /* pointer to encoded MP3 stream */
    //    const int           mp3buf_size ); /* number of valid octets in this
    //                                          stream                        */

    /*if (mp3_bytes < 0) {
        printf("Lame encoding error");
    }*/

    //There are also routines for various types of input  
    //(float, long, interleaved, etc).  See lame.h for details.

    //6. 
    /*int mp3_buffer_size = 7200;
    char *mp3_buffer = malloc(mp3_buffer_size);*/

    /*
    flush the buffers and may return a final few mp3 frames
    buffer should be at least 7200 bytes
    return code = number of bytes output to mp3buffer (can be 0)
    */
    /*int flushed_mp3_bytes = lame_encode_flush(gfp, mp3_buffer, mp3_buffer_size);*/


    //7.  Write the Xing VBR/INFO tag to mp3 file.  
    /*
    This adds a valid mp3 frame which contains information about the
    bitstream some players may find usefull.  It is used for CBR,ABR and
    VBR.  The routine will attempt to rewind the output stream to the
    beginning.  If this is not possible, (for example, you are encoding to
    stdout) you should specifically disable the tag by calling
    lame_set_bWriteVbrTag(gfp,0) in step 3 above, and call
    lame_mp3_tags_fid() with fid=NULL.  If the rewind fails and
    the tag was not disabled, the first mp3 frame in the bitstream
    will be all 0's.
    */
    /*void lame_mp3_tags_fid(lame_global_flags *,FILE* fid);*/





    //8. free the internal data structures.

    //lame_close(gfp); //void lame_close(lame_global_flags *);










    //detectNumberOfCores() {





    //loadFile();


    //verify parameters
    //correct number
    //parameter must be correctly formatted and existing folder path


    //parseParameters(); //get filenames from parameter, check for errors

    //detectOS(); //windows or linux + which version
    //detectNumberOfCores();

    

    //set global values ?, output folder etc

    //findAllWavFiles();
    //printListOfWavFiles();

    //TODO Parallelization
    //encodeAllWavFiles(); //loop over all found wav files
    //encodeSingleWavFile(); //encode single file with lame


    return 0;
}