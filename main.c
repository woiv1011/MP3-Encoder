
#define DEBUG
#include "main.h"

void parseParameters() {
    //TODO check for errors, wrong usage etc
}

void printListOfWavFiles() { //list as pointer parameter
    printf("The following WAV files were found:\n");
    //loop
}

void detectOS() {
}

void detectNumberOfCores() {
}

void findAllWavFiles() {
}


void encodeAllWavFiles() {
}

void encodeSingleWavFile() {
}

void printUsageHelp() {
    printf("WAV to MP3 Encoder\n");
}

void loadFile() {
    //get File size
    //allocate memory
    //open file
    //read file into memory
    //close file

    //TODO safety checks
}

int listDirectory(char *path) {
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
        printf ("[%s]\n", pDirent->d_name);
    }
    closedir (pDir);

    return 0;
}

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

    listDirectory(argv[1]);

    

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