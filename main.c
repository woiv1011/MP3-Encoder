#define DEBUG

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

void printListOfWavFiles() {
}

void encodeAllWavFiles() {
}

void encodeSingleWavFile() {
}

void printUsageHelp() {
    printf("WAV to MP3 Encoder\n");
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

int main(int argc, char **argv) {
    //path = C:\Users\Ivar\Desktop\MP3-Encoder
#ifdef DEBUG
    printf("argc: %d\n", argc);
    for (int i=0; i < argc; i=i+1) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }
#endif

    loadFile();


    //verify parameters
    //correct number
    //parameter must be correctly formatted and existing folder path


    parseParameters(); //get filenames from parameter, check for errors

    detectOS(); //windows or linux + which version
    detectNumberOfCores();

    

    //set global values ?, output folder etc

    findAllWavFiles();
    printListOfWavFiles();

    //TODO Parallelization
    encodeAllWavFiles(); //loop over all found wav files
    encodeSingleWavFile(); //encode single file with lame


    return 0;
}