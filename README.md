# PseudoFAT filesystem

## Description

Application takes one command line argument:

    fs_filepath - path to the filesystem file

Program represents a pseudoFAT filesystem, based on a real FAT.
PseudoFAT because it is simplified in these aspects:

    number of files per directory is limited (depends on cluster size)
    there is only one FAT table (no backup FAT)
    the right syntax is expected (no error handling)

## Usage

    ./pseudoFAT fs_filepath

### Build

Project is built using CMake. To build the project, run the following commands:

#### Windows

    mkdir build
    cd build
    cmake ..
    cmake --build .

#### Linux

    mkdir build
    cd build
    cmake ..
    make

## Input commands format

    command_name [arg1] [arg2] ...

### Commands

    help              | display this message
    exit              | exit the program
    meta              | display meta information about the file system
    fat               | display the FAT
    cp <src> <dst>    | copy file from <src> to <dst>
    mv <src> <dst>    | move file from <src> to <dst>
    rm <file>         | remove file <file>
    mkdir <dir>       | create directory <dir>
    rmdir <dir>       | remove directory <dir>
    ls <dir>          | list directory <dir> contents
    cat <file>        | display file <file> contents
    cd <dir>          | change current directory to <dir>
    pwd               | print working directory
    info <dir/file>   | display information about directory <dir> / file <file>
    incp <src> <dst>  | copy file from disk <src> to <dst> in the file system
    outcp <src> <dst> | copy file from <src> in the file system to disk <dst>
    load <file>       | load file <file> from disk and execute commands from it
    format <size>     | format the file system with size <size>
    defrag <file>     | defragment the file <file>

All commands are case sensitive and arguments are separated by spaces

All commands support both relative and absolute paths