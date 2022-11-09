#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

/** Free cluster_address constant */
constexpr int32_t FAT_FREE = -1;
/** End of file constant */
constexpr int32_t FAT_EOF = -2;
/** Bad cluster_address constant */
constexpr int32_t FAT_BAD = -3;
/** 1024 Bytes = 1 KB */
constexpr int32_t KB = 1024;
/** 1024 KB = 1 MB */
constexpr int32_t MB = 1024 * KB;
/** 1024 MB = 1 GB */
constexpr int32_t GB = 1024 * MB;
/** Cluster size in bytes */
constexpr uint32_t DEFAULT_CLUSTER_SIZE = 1 * KB;
/** Default length of file name */
constexpr uint32_t DEFAULT_FILE_NAME_LENGTH = 12;
/** Default FILE NOT FOUND error message */
constexpr const char *FILE_NOT_FOUND = "ERROR: FILE NOT FOUND";
/** Default DIRECTORY NOT FOUND error message */
constexpr const char *DIRECTORY_NOT_FOUND = "ERROR: DIR NOT FOUND";
/** Default FILE ALREADY EXISTS error message */
constexpr const char *FILE_ALREADY_EXISTS = "ERROR: FILE ALREADY EXISTS";
/** Default DIRECTORY ALREADY EXISTS error message */
constexpr const char *DIRECTORY_ALREADY_EXISTS = "ERROR: DIR ALREADY EXISTS";
/** Default FILE IS DIRECTORY error message */
constexpr const char *FILE_IS_DIRECTORY = "ERROR: IS DIR";
/** Default DIRECTORY IS FILE error message */
constexpr const char *FILE_IS_NOT_DIRECTORY = "ERROR: IS NOT DIR";
/** Default DIRECTORY IS NOT EMPTY error message */
constexpr const char *DIRECTORY_IS_NOT_EMPTY = "ERROR: DIR IS NOT EMPTY";
/** Default NO SPACE error message */
constexpr const char *NO_SPACE = "ERROR: NO SPACE";
/** Default CANNOT REMOVE CURRENT DIRECTORY error message */
constexpr const char *CANNOT_REMOVE_CURR_DIR = "ERROR: CANNOT REMOVE CURRENT DIR";
/** Default PATH NOT FOUND error message */
constexpr const char *PATH_NOT_FOUND = "ERROR: PATH NOT FOUND";
/** Default OK message */
constexpr const char *OK = "OK";

/**
 * MetaData structure for the whole file system
 * Includes information about the file system
 */
struct MetaData {
    /** zapped99 + null terminator = 9 */
    char signature[9];
    /** Size of the file system in bytes */
    uint32_t disk_size;
    /** Size of a cluster_address in bytes */
    uint32_t cluster_size;
    /** Number of clusters in the file system */
    uint32_t cluster_count;
    /** Fat table offset in bytes */
    uint32_t fat_start_address;
    /** Fat table size */
    uint32_t fat_size;
    /** Root directory offset in bytes */
    uint32_t data_start_address;
};

/**
 * DirectoryEntry structure for a directory entry
 * Includes information about a file or directory
 */
struct DirectoryEntry {
    /** Name of the file or directory */
    char item_name[DEFAULT_FILE_NAME_LENGTH];
    /** Flag for if the entry is a file or directory */
    bool is_directory;
    /** Size of the file in bytes */
    uint32_t size;
    /** Index of the first data cluster_address */
    uint32_t start_cluster;
};

/**
 * Working directory structure
 * Includes information about the current working directory
 */
struct WorkingDirectory {
    /** Cluster address of the working directory */
    uint32_t cluster_address;
    /** Path of the working directory */
    std::string path;
    /** Directory entries of the working directory */
    std::vector<DirectoryEntry> entries;
};

/**
 * Class representing a pseudo FAT file system
 * The file system is stored in a file on the disk
 */
class PseudoFS {
private:
    /** Typedef for a map of command functions */
    typedef bool (PseudoFS::*command)(const std::vector<std::string> &);
    /** Typedef for a map of command functions */
    typedef std::map<std::string, command> command_map;
    /** Map of command functions */
    command_map commands;
    /** File system file */
    std::string file_system_filepath;
    /** File system stream */
    std::fstream file_system;
    /** Meta data for the file system */
    struct MetaData meta_data;
    /** Working directory */
    struct WorkingDirectory working_directory;
    /** Working directory representing the root directory */
    struct WorkingDirectory ROOT_DIRECTORY;
    /** String representing empty cluster (zeroes) */
    std::string EMPTY_CLUSTER;

    /**
     * Initializes the command map
     * Maps commands to functions
     * Is always called in the constructor
     */
    void initialize_command_map();

    /**
     * Transforms FAT cluster index to cluster address offset in bytes in the file system
     * @param cluster_index Index of the cluster in the FAT table (cluster index)
     * @return Offset of the cluster in bytes (cluster address)
     */
    uint32_t get_cluster_address(uint32_t cluster_index) const;

    /**
     * Transforms cluster address offset in bytes in the file system to FAT cluster index
     * @param cluster_address Offset of the cluster in bytes (cluster address)
     * @return Index of the cluster in the FAT table (cluster index)
     */
    uint32_t get_cluster_index(uint32_t cluster_address) const;

    /**
    * Gets the first free cluster_address address in the FAT table
    * @return Index of the first free cluster_address (or 0 if there are no free clusters)
    */
    uint32_t find_free_cluster();

    /**
     * Redas the data from the cluster
     * @param cluster_address Address of the cluster in bytes
     * @param size Size of the data to read in bytes
     * @return Data read from the cluster
     */
    std::string read_from_cluster(uint32_t cluster_address, int size);

    /**
     * Writes the data to the cluster
     * @param cluster_address Address of the cluster in bytes
     * @param data Data to write to the cluster
     * @param size Size of the data in bytes
     */
    void write_to_cluster(uint32_t cluster_address, const std::string &data, int size);

    /**
     * Reads the value from the FAT table
     * @param cluster_index Index of the cluster in the FAT table
     * @return Value of the cluster in the FAT table
     */
    uint32_t read_from_fat(uint32_t cluster_index);

    /**
     * Writes the value to the FAT table at the given index
     * @param cluster_index Index of the cluster in the FAT table
     * @param value Value to write to the FAT table
     */
    void write_to_fat(uint32_t cluster_index, uint32_t value);

    /**
     * Gets the directory entries of a directory given by it's cluster_address index
     * @param cluster Cluster index of the directory
     * @return Vector of directory entries
     */
    std::vector<DirectoryEntry> get_directory_entries(uint32_t cluster);

    /**
     * Writes the DirectoryEntry to the given cluster_address (directory)
     * @param cluster_address Cluster address of the directory
     * @param entry Entry to be written
     */
    void write_directory_entry(uint32_t cluster_address, const DirectoryEntry &entry);

    /**
     * Removes the DirectoryEntry from the given cluster_address (directory)
     * @param cluster_address Cluster address of the directory
     * @param entry Entry to be removed
     */
    void remove_directory_entry(uint32_t cluster_address, const DirectoryEntry &entry);

    /**
     * Checks if the entry given by the name exists in the current working directory
     * @param name Name of the entry
     * @param entry Entry to be found
     * @return True if the entry exists, false otherwise
     */
    bool does_entry_exist(const std::string &name, DirectoryEntry &entry);

    /**
     * Changes current working directory to the given entry within the current working directory
     * @param dir_name Name of the directory to change to (not a path)
     * @return True if the directory was changed, false otherwise
     */
    bool change_directory(const std::string &dir_name);

    /**
     * Help function to list all commands
     * Callable by using the 'help' command
     * @param args This function takes no arguments (only for genericity)
     * @return Always returns true (only for genericity)
     */
    bool help(const std::vector<std::string> &args);

    /**
     * Meta function prints the meta data of the file system
     * Callable by using the 'meta' command
     * @param args This function takes no arguments (only for genericity)
     * @return Always returns true (only for genericity)
     */
    bool meta(const std::vector<std::string> &args);

    /**
     * Fat function prints the FAT table of the file system
     * Callable by using the 'fat' command
     * @param args This function takes no arguments (only for genericity)
     * @return Always returns true (only for genericity)
     */
    bool fat(const std::vector<std::string> &args);

    /**
     * Copy function copies a file from the <src> to the <dst>
     * Callable by using the 'copy' command with the <src> and <dst> arguments
     * @param args <src> and <dst> filepaths to copy from and to are expected
     * @return True if the copy was successful, false otherwise
     */
    bool cp(const std::vector<std::string> &args);

    /**
     * Move function moves a file from the <src> to the <dst>
     * Callable by using the 'move' command with the <src> and <dst> arguments
     * @param args <src> and <dst> filepaths to move from and to are expected
     * @return True if the move was successful, false otherwise
     */
    bool mv(const std::vector<std::string> &args);

    /**
     * Remove function removes a file from the file system
     * Callable by using the 'remove' command with the <filepath> argument
     * @param args <filepath> to be removed is expected
     * @return True if the remove was successful, false otherwise
     */
    bool rm(const std::vector<std::string> &args);

    /**
     * Make directory function creates a directory in the file system
     * Callable by using the 'mkdir' command with the <dirpath> argument
     * @param args <dirpath> to be created is expected
     * @return True if the directory was created, false otherwise
     */
    bool mkdir(const std::vector<std::string> &args);

    /**
     * Remove directory function removes a directory from the file system
     * Callable by using the 'rmdir' command with the <dirpath> argument
     * @param args <dirpath> to be removed is expected
     * @return True if the directory was removed, false otherwise
     */
    bool rmdir(const std::vector<std::string> &args);

    /**
     * List function lists the contents of a directory
     * Callable by using the 'ls' command with the <dirpath> argument (or no argument for current <dir>)
     * @param args <dirpath> to be listed is expected (or no argument for current <dir>)
     * @return True if the directory was listed, false otherwise
     */
    bool ls(const std::vector<std::string> &args);

    /**
     * Cat function prints the contents of a file
     * Callable by using the 'cat' command with the <filepath> argument
     * @param args <filepath> to be printed is expected
     * @return True if the file was printed, false otherwise
     */
    bool cat(const std::vector<std::string> &args);

    /**
     * Change directory function changes the current directory
     * Callable by using the 'cd' command with the <dirpath> argument
     * @param args <dirpath> to be changed to is expected
     * @return True if the directory was changed, false otherwise
     */
    bool cd(const std::vector<std::string> &args);

    /**
     * Print working directory function prints the current directory
     * Callable by using the 'pwd' command
     * @param args This function takes no arguments (only for genericity)
     * @return Always returns true (only for genericity)
     */
    bool pwd(const std::vector<std::string> &args);

    /**
     * Info function prints information about a file or directory
     * Callable by using the 'info' command with the <filepath> argument
     * @param args <filepath> to be informed about is expected
     * @return True if the information was printed, false otherwise
     */
    bool info(const std::vector<std::string> &args);

    /**
     * In copy function copies a file from the <src> on disk to the <dst> on the file system
     * Callable by using the 'incp' command with the <src> and <dst> arguments
     * @param args <src> and <dst> filepaths to copy from and to are expected
     *            (<src> is on the disk, <dst> is on the file system)
     * @return True if the copy was successful, false otherwise
     */
    bool incp(const std::vector<std::string> &args);

    /**
     * Out copy function copies a file from the <src> on the file system to the <dst> on disk
     * Callable by using the 'outcp' command with the <src> and <dst> arguments
     * @param args <src> and <dst> filepaths to copy from and to are expected
     *            (<src> is on the file system, <dst> is on the disk)
     * @return True if the copy was successful, false otherwise
     */
    bool outcp(const std::vector<std::string> &args);

    /**
     * Load function loads a file from the disk and reads it line by line and executes the commands on the lines
     * Callable by using the 'load' command with the <filepath> argument
     * @param args <filepath> to be loaded is expected
     *            (<filepath> is on the disk)
     * @return True if the load was successful, false otherwise
     */
    bool load(const std::vector<std::string> &args);

    /**
     * Format function formats the file system to the given size <size> in bytes
     * Callable by using the 'format' command with the <size> argument
     * @param args <size> of the file system is expected (KB, MB and GB are supported)
     *            (examples: 1024, 10KB, 200MB, 2GB...)
     * @return True if the format was successful, false otherwise
     */
    bool format(const std::vector<std::string> &args);

    /**
     * Defragmentation function defragments the given file <filepath>
     * Callable by using the 'defrag' command with the <filepath> argument
     * @param args <filepath> to be defragmented is expected
     * @return True if the defragmentation was successful, false otherwise
     */
    bool defrag(const std::vector<std::string> &args);

public:
    /**
     * Constructor
     * @param filepath Filepath of the file system file
     */
    explicit PseudoFS(const std::string &filepath);

    /**
     * Destructor
     */
    ~PseudoFS();

    /**
     * Calls the function mapped to the command with the given string
     * @param cmd String of the command to be executed
     * @param args Arguments of the command to be executed
     */
    void call_cmd(const std::string &cmd, const std::vector<std::string> &args);

    /**
     * Getter for the current directory of the file system
     * @return Current directory of the file system
     */
    std::string get_working_directory_path() const;
};
