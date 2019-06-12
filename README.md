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




vs code and terminal output lag


#TODO
+ mp3 files get sped up by factor of 2
    + probably num_channels problem, test with stereo file

+ directories ending in wav produce error message
+ create thread numbering ?
+ fix crash with directory path not ending in slash
+ prevent lag better


i tend to prefer long, readable names/identifiers instead of abbreviations that one has to be familiar with

2240 wav files
2691 printed out

495 wav files
738 printed out
7 other files in folder
3 other subdirectories
505 total elements

500 wav files
745 files output ?


encode_counter ist schon vor finalem output falsch
apparently after clean_mp3 ?

after make clean_mp3, 745 
HOW ? wtf ?
some filesystem problem

create version where single thread reads directory

# Known Issues
+ when running the multithreaded program the first time after "make clean_mp3", some files get encoded twice ?
    + presumably some issue with filesystem, readdir is not properly thread-safe, even though I use locks ?
    + TODO iterate through directory on single thread


TODO test with large wav files to check for memory problems, buffer size etc