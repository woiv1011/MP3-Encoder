# MP3-Encoder

# ToDo
* create proper structure with header files etc
    * maybe for a small project like this its okay to just include everything
* create build system
    * makefile ?
    * bat / powershell ?
    * bash ?
    * python compile script ?
    * other build system probably not necessary
* Parallelization options
    * split multiple files onto multiple cores/threads
        - possibly unused cores, or different usage, because files have different size
    * split single file onto multiple threads
        - communication etc necessary
        - is that possible with lame ?
* Filepath parsing must be compatible with both linux and windows
    * Regex filename ending or check manually
        * what if filename doesn't have .wav ending (linux ?)
* Error safety checks, exception handling, etc



# ToDo End
* Clean up Code
    * names
    * formatting
    * comments
* Tests ?
* LAME License stuff



# MaybeDo
* implement general program structure in Python or other highlevel language and only computational hotspots in C/C++
    * python should be possible in most embedded environments
* performance tests of different versions with graph output


# STEPS

(evtl filehandling etc in Python)
1. Make single thread version for one OS (windows), without safety checks
    1. load lame
    2. load wav file into memory
2. add safety checks
3. add version for other OS
4. add multithreading


# TODO
+ more robust arg parsing
    + check that parameter is actually a directory path
+ more safety checks
    + put lame_encode_buffer etc in try-catch block or c equivalent to prevent segfault
    + check if it's a wav file by reading wav header instead/in addition to checking last 4 chars
    + replace all variants of string function with n variants, when possible
+ parallelize with pthreads
+ TEST application on windows and linux
    + MAYBE compare output with lame binary at equal settings ?
+ Clean up Code
+ MAYBE port to C++ for exception handling ?




