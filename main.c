


int main(int argc, char **argv) {
    //pathname as arguments
    parseParameters(); //get filenames from parameter, check for errors

    detectOS(); //windows or linux + which version
    detectNumberOfCores();

    //set global values ?, output folder etc

    findAllWavFiles();

    //TODO Parallelization
    encodeAllWavFiles(); //loop over all found wav files
    encodeSingleWavFile(); //encode single file with lame


    return 0;
}