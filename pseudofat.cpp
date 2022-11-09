#include "pseudofat.h"

PseudoFS::PseudoFS(const std::string &filepath) : file_system_filepath{filepath}, meta_data{}, working_directory{},
                                                  ROOT_DIRECTORY{} {
    // Open the file system file
    file_system.open(filepath, std::ios::binary | std::ios::in | std::ios::out);

    // If the file doesn't exist, create it
    if (!file_system.is_open())
        file_system.open(filepath, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);

        // If the file existed, read the metadata
    else {
        file_system.read(reinterpret_cast<char *>(&meta_data), sizeof(MetaData));
        ROOT_DIRECTORY = WorkingDirectory{
                meta_data.data_start_address,
                "/",
                get_directory_entries(meta_data.data_start_address)
        };
        working_directory = ROOT_DIRECTORY;
        EMPTY_CLUSTER = std::string(meta_data.cluster_size, '\0');
    }

    // If the file still isn't open, print an error
    if (!file_system.is_open())
        std::cerr << "Error opening file system file" << std::endl;

    // Initialize the command map for the shell
    initialize_command_map();
}

PseudoFS::~PseudoFS() {
    file_system.close();
}

void PseudoFS::initialize_command_map() {
    commands["help"] = &PseudoFS::help;
    commands["meta"] = &PseudoFS::meta;
    commands["fat"] = &PseudoFS::fat;
    commands["cp"] = &PseudoFS::cp;
    commands["mv"] = &PseudoFS::mv;
    commands["rm"] = &PseudoFS::rm;
    commands["mkdir"] = &PseudoFS::mkdir;
    commands["rmdir"] = &PseudoFS::rmdir;
    commands["ls"] = &PseudoFS::ls;
    commands["cat"] = &PseudoFS::cat;
    commands["cd"] = &PseudoFS::cd;
    commands["pwd"] = &PseudoFS::pwd;
    commands["info"] = &PseudoFS::info;
    commands["incp"] = &PseudoFS::incp;
    commands["outcp"] = &PseudoFS::outcp;
    commands["load"] = &PseudoFS::load;
    commands["format"] = &PseudoFS::format;
    commands["defrag"] = &PseudoFS::defrag;
}

uint32_t PseudoFS::get_cluster_address(uint32_t cluster_index) const {
    auto index = (cluster_index - meta_data.fat_start_address) / sizeof(uint32_t);
    return meta_data.data_start_address + index * meta_data.cluster_size;
}

uint32_t PseudoFS::get_cluster_index(uint32_t cluster_address) const {
    auto index = (cluster_address - meta_data.data_start_address) / meta_data.cluster_size;
    return meta_data.fat_start_address + index * sizeof(uint32_t);
}

uint32_t PseudoFS::find_free_cluster() {
    // Seek to the data start address and read the FAT, looking for a first free cluster
    for (int i = 0; i < meta_data.cluster_count; i++) {
        uint32_t cluster{};
        file_system.seekp(meta_data.fat_start_address + i * sizeof(uint32_t));
        file_system.read(reinterpret_cast<char *>(&cluster), sizeof(uint32_t));
        if (cluster == FAT_FREE)
            return i;
    }
    return 0;
}

std::string PseudoFS::read_from_cluster(uint32_t cluster_address, int size) {
    char buffer[size];
    file_system.seekp(cluster_address);
    file_system.read(buffer, size);
    return (buffer);
}

void PseudoFS::write_to_cluster(uint32_t cluster_address, const std::string &data, int size) {
    file_system.seekp(cluster_address);
    file_system.write(data.c_str(), size);
}

uint32_t PseudoFS::read_from_fat(uint32_t cluster_index) {
    uint32_t result;
    file_system.seekp(cluster_index);
    file_system.read(reinterpret_cast<char *>(&result), sizeof(uint32_t));
    return result;
}

void PseudoFS::write_to_fat(uint32_t cluster_index, uint32_t value) {
    file_system.seekp(cluster_index);
    file_system.write(reinterpret_cast<char *>(&value), sizeof(uint32_t));
}

std::vector<DirectoryEntry> PseudoFS::get_directory_entries(uint32_t cluster) {
    // Seek to the cluster in the file system and read the directory entries
    std::vector<DirectoryEntry> entries;
    file_system.seekp(cluster);
    for (int i = 0; i < meta_data.cluster_size / sizeof(DirectoryEntry); i++) {
        DirectoryEntry entry{};
        file_system.read(reinterpret_cast<char *>(&entry), sizeof(DirectoryEntry));
        // If the entry is not empty, add it to the vector
        if (entry.start_cluster != 0)
            entries.push_back(entry);
    }
    return entries;
}

void PseudoFS::write_directory_entry(uint32_t cluster_address, const DirectoryEntry &entry) {
    // Seek to the cluster and write to the first empty directory entry
    file_system.seekp(cluster_address);
    for (int i = 0; i < meta_data.cluster_size / sizeof(DirectoryEntry); i++) {
        auto tmp = DirectoryEntry{};
        file_system.read(reinterpret_cast<char *>(&tmp), sizeof(DirectoryEntry));
        if (tmp.start_cluster != 0)
            continue;
        file_system.seekp(cluster_address + i * sizeof(DirectoryEntry));
        file_system.write(reinterpret_cast<const char *>(&entry), sizeof(DirectoryEntry));
        break;
    }
}

void PseudoFS::remove_directory_entry(uint32_t cluster_address, const DirectoryEntry &entry) {
    // Seek to the cluster and write an empty directory entry to the one with the same starting cluster
    for (int i = 0; i < meta_data.cluster_size / sizeof(DirectoryEntry); i++) {
        auto tmp = DirectoryEntry{};
        file_system.seekp(cluster_address + i * sizeof(DirectoryEntry));
        file_system.read(reinterpret_cast<char *>(&tmp), sizeof(DirectoryEntry));
        if (tmp.start_cluster == entry.start_cluster) {
            file_system.seekp(cluster_address + i * sizeof(DirectoryEntry));
            DirectoryEntry empty_entry{};
            file_system.write(reinterpret_cast<const char *>(&empty_entry), sizeof(DirectoryEntry));
            break;
        }
    }
}

bool PseudoFS::does_entry_exist(const std::string &name, DirectoryEntry &entry) {
    for (const auto &entry_for: working_directory.entries) {
        if (entry_for.item_name == name) {
            entry = entry_for;
            break;
        }
    }
    if (!entry.start_cluster)
        return false;
    return true;
}

bool PseudoFS::change_directory(const std::string &dir_name) {
    // If the dir_name is empty or "/", change to the root directory
    if (dir_name.empty() || dir_name == "/") {
        working_directory = ROOT_DIRECTORY;
        return true;
    }

    // If the dir_name is ".", nothing needs to be done
    if (dir_name == ".")
        return true;

    // If the dir_name is "..", change path to the parent directory
    if (dir_name == "..") {
        working_directory.path = working_directory.path.substr(0, working_directory.path.find_last_of('/'));
        working_directory.path = working_directory.path.substr(0, working_directory.path.find_last_of('/'));
        working_directory.path += "/";
    }

    // Find the directory entry for the directory
    auto entries = get_directory_entries(working_directory.cluster_address);
    for (const auto &entry: entries) {
        // If the entry is a directory and the name matches, change the working directory
        if (entry.item_name == dir_name && entry.is_directory) {
            if (dir_name != "..")
                working_directory.path += dir_name + "/";
            working_directory.cluster_address = entry.start_cluster;
            working_directory.entries = get_directory_entries(entry.start_cluster);
            return true;
        }
    }

    // If the directory wasn't found, return false
    return false;
}

bool PseudoFS::is_file_defragmented(const DirectoryEntry &entry, std::vector<uint32_t> &clusters) {
    // Get the first cluster of the file
    auto cluster_address = entry.start_cluster;
    auto cluster_index = get_cluster_index(cluster_address);

    // Get all the clusters of the file
    while (cluster_address != FAT_EOF) {
        clusters.push_back((cluster_index - meta_data.fat_start_address) / sizeof(uint32_t));
        cluster_address = read_from_fat(cluster_index);
        cluster_index = get_cluster_index(cluster_address);
    }

    // If the clusters are consecutive in the file system, the file is defragmented
    for (int i = 0; i < clusters.size() - 1; i++) {
        if (clusters[i] + 1 != clusters[i + 1])
            return false;
    }

    return true;
}

void PseudoFS::call_cmd(const std::string &cmd, const std::vector<std::string> &args) {
    if (commands.count(cmd))
        (this->*commands[cmd])(args);
    else {
        std::cerr << "Unknown command: " << cmd << std::endl;
        std::cerr << "Type 'help' for a list of commands" << std::endl;
    }
}

std::string PseudoFS::get_working_directory_path() const {
    return working_directory.path;
}


bool PseudoFS::help(const std::vector<std::string> &args) {
    std::cout << "-------------------------------------------------------------------------------" << std::endl;
    std::cout << "| help              | display this message                                    |" << std::endl;
    std::cout << "| exit              | exit the program                                        |" << std::endl;
    std::cout << "| meta              | display meta information about the file system          |" << std::endl;
    std::cout << "| fat               | display the FAT                                         |" << std::endl;
    std::cout << "| cp <src> <dst>    | copy file from <src> to <dst>                           |" << std::endl;
    std::cout << "| mv <src> <dst>    | move file from <src> to <dst>                           |" << std::endl;
    std::cout << "| rm <file>         | remove file <file>                                      |" << std::endl;
    std::cout << "| mkdir <dir>       | create directory <dir>                                  |" << std::endl;
    std::cout << "| rmdir <dir>       | remove directory <dir>                                  |" << std::endl;
    std::cout << "| ls <dir>          | list directory <dir> contents                           |" << std::endl;
    std::cout << "| cat <file>        | display file <file> contents                            |" << std::endl;
    std::cout << "| cd <dir>          | change current directory to <dir>                       |" << std::endl;
    std::cout << "| pwd               | print working directory                                 |" << std::endl;
    std::cout << "| info <dir/file>   | display information about directory <dir> / file <file> |" << std::endl;
    std::cout << "| incp <src> <dst>  | copy file from disk <src> to <dst> in the file system   |" << std::endl;
    std::cout << "| outcp <src> <dst> | copy file from <src> in the file system to disk <dst>   |" << std::endl;
    std::cout << "| load <file>       | load file <file> from disk and execute commands from it |" << std::endl;
    std::cout << "| format <size>     | format the file system with size <size>                 |" << std::endl;
    std::cout << "| defrag <file>     | defragment the file <file>                              |" << std::endl;
    std::cout << "-------------------------------------------------------------------------------" << std::endl;
    return true;
}

bool PseudoFS::meta(const std::vector<std::string> &args) {
    std::cout << "-------------------------------------------------------------------------------" << std::endl;
    std::cout << "Signature:          " << meta_data.signature << std::endl;
    std::cout << "Disk size:          " << meta_data.disk_size << std::endl;
    std::cout << "Cluster size:       " << meta_data.cluster_size << std::endl;
    std::cout << "Cluster count:      " << meta_data.cluster_count << std::endl;
    std::cout << "Fat start address:  " << meta_data.fat_start_address << std::endl;
    std::cout << "Fat size:           " << meta_data.fat_size << std::endl;
    std::cout << "Data start address: " << meta_data.data_start_address << std::endl;
    std::cout << "-------------------------------------------------------------------------------" << std::endl;
    return true;
}

bool PseudoFS::fat(const std::vector<std::string> &args) {
    std::cout << "-------------------------------------------------------------------------------" << std::endl;
    file_system.seekp(meta_data.fat_start_address);
    for (int i = 0; i < meta_data.cluster_count; i++) {
        uint32_t cluster{};
        file_system.read(reinterpret_cast<char *>(&cluster), sizeof(uint32_t));
        if (cluster == FAT_FREE)
            std::cout << i << ": " << "FREE" << std::endl;
        else if (cluster == FAT_EOF)
            std::cout << i << ": " << "EOF" << std::endl;
        else if (cluster == FAT_BAD)
            std::cout << i << ": " << "BAD" << std::endl;
        else
            std::cout << i << ": " << cluster << std::endl;
    }
    std::cout << "-------------------------------------------------------------------------------" << std::endl;
    return true;
}

bool PseudoFS::cp(const std::vector<std::string> &args) {
    // First check if the source file exists
    auto saved_working_directory = working_directory;
    std::string file_name = args[1].substr(args[1].find_last_of('/') + 1, args[1].size());
    std::string file_path = args[1].substr(0, args[1].find_last_of('/'));
    std::vector<std::string> cd_args_file = {"cd", file_path, "don't print ok"};
    bool result_cd_file = true;
    // If directory path is not empty, change working directory
    if (args[1].find('/') != std::string::npos)
        result_cd_file = this->cd(cd_args_file);

    // Error could have occurred while changing working directory
    if (!result_cd_file) {
        working_directory = saved_working_directory;
        return false;
    }

    // Check if file with the given name exists
    auto source_entry = DirectoryEntry{};
    if (!does_entry_exist(file_name, source_entry)) {
        std::cerr << FILE_NOT_FOUND << std::endl;
        working_directory = saved_working_directory;
        return false;
    }

    // Check if the source file is a directory
    if (source_entry.is_directory) {
        std::cerr << FILE_IS_DIRECTORY << std::endl;
        working_directory = saved_working_directory;
        return false;
    }

    // Second check that the destination directory exists
    working_directory = saved_working_directory;

    std::string new_file_name = args[2].substr(args[2].find_last_of('/') + 1, args[2].size());
    std::string dir_path = args[2].substr(0, args[2].find_last_of('/'));
    std::vector<std::string> cd_args_dir = {"cd", dir_path, "don't print ok"};
    bool result_cd_dir = true;
    // If directory path is not empty, change working directory
    if (args[2].find('/') != std::string::npos)
        result_cd_dir = this->cd(cd_args_dir);

    // Error could have occurred while changing working directory
    if (!result_cd_dir) {
        working_directory = saved_working_directory;
        return false;
    }

    // Check if file with the given name exists
    auto existence_check = DirectoryEntry{};
    if (does_entry_exist(new_file_name, existence_check)) {
        std::cerr << FILE_ALREADY_EXISTS << std::endl;
        working_directory = saved_working_directory;
        return false;
    }

    // Everything is fine, copy the file now - create new entry
    auto new_entry = DirectoryEntry{
            "",
            false,
            source_entry.size,
            0
    };
    for (int i = 0; i < DEFAULT_FILE_NAME_LENGTH - 1; i++)
        new_entry.item_name[i] = new_file_name[i];
    new_entry.item_name[DEFAULT_FILE_NAME_LENGTH - 1] = '\0';

    // Find free cluster
    auto index = find_free_cluster();
    if (index == FAT_EOF) {
        std::cerr << NO_SPACE << std::endl;
        working_directory = saved_working_directory;
        return false;
    }
    auto write_cluster_address = meta_data.data_start_address + index * meta_data.cluster_size;
    auto write_cluster_index = get_cluster_index(write_cluster_address);
    new_entry.start_cluster = write_cluster_address;
    auto read_cluster_address = source_entry.start_cluster;
    auto read_cluster_index = get_cluster_index(read_cluster_address);
    write_to_fat(write_cluster_index, FAT_EOF); // Marking as used; EOF will do for now, later it will be changed
    uint32_t previous_cluster_index = 0;

    // Copy the file
    auto number_of_iterations = source_entry.size / meta_data.cluster_size;
    if (source_entry.size % meta_data.cluster_size)
        number_of_iterations++;

    for (int i = 0; i < number_of_iterations; i++) {
        // Write cluster address to FAT
        if (previous_cluster_index != 0)
            write_to_fat(previous_cluster_index, write_cluster_address);

        previous_cluster_index = write_cluster_index;

        // Read and write cluster
        if (i != number_of_iterations - 1) {
            std::string buffer = read_from_cluster(read_cluster_address, static_cast<int>(meta_data.cluster_size));
            write_to_cluster(write_cluster_address, buffer, static_cast<int>(meta_data.cluster_size));
        }
            // Last iteration
        else {
            std::string buffer = read_from_cluster(read_cluster_address,
                                                   static_cast<int>(source_entry.size % meta_data.cluster_size));
            write_to_cluster(write_cluster_address, buffer,
                             static_cast<int>(source_entry.size % meta_data.cluster_size));
            break;
        }

        // Find next free cluster
        index = find_free_cluster();
        if (!index) {
            std::cerr << NO_SPACE << std::endl;
            working_directory = saved_working_directory;
            return false;
        }
        write_cluster_index = meta_data.fat_start_address + index * sizeof(uint32_t);
        write_to_fat(write_cluster_index, FAT_EOF); // Marking as used; EOF will do for now, later it will be changed
        write_cluster_address = get_cluster_address(write_cluster_index);

        // Find next cluster to read
        read_cluster_address = read_from_fat(read_cluster_index);
        read_cluster_index = get_cluster_index(read_cluster_address);
    }

    // Write directory entry to directory
    write_directory_entry(working_directory.cluster_address, new_entry);

    // Restore and update working directory
    working_directory = saved_working_directory;
    working_directory.entries = get_directory_entries(working_directory.cluster_address);

    std::cout << OK << std::endl;
    return true;
}

bool PseudoFS::mv(const std::vector<std::string> &args) {
    // First check that the file exists
    auto saved_working_directory = working_directory;
    std::string file_name = args[1].substr(args[1].find_last_of('/') + 1, args[1].size());
    std::string file_path = args[1].substr(0, args[1].find_last_of('/'));
    std::vector<std::string> cd_args_file = {"cd", file_path, "don't print ok"};
    bool result_cd_file = true;
    // If directory path is not empty, change working directory
    if (args[1].find('/') != std::string::npos)
        result_cd_file = this->cd(cd_args_file);

    // Error could have occurred while changing working directory
    if (!result_cd_file) {
        working_directory = saved_working_directory;
        return false;
    }

    // Check if file with the given name exists
    auto entry = DirectoryEntry{};
    if (!does_entry_exist(file_name, entry)) {
        std::cerr << FILE_NOT_FOUND << std::endl;
        working_directory = saved_working_directory;
        return false;
    }

    // Second check that the destination directory exists
    working_directory = saved_working_directory;

    std::string new_file_name = args[2].substr(args[2].find_last_of('/') + 1, args[2].size());
    std::string dir_path = args[2].substr(0, args[2].find_last_of('/'));
    std::vector<std::string> cd_args_dir = {"cd", dir_path, "don't print ok"};
    bool result_cd_dir = true;
    // If directory path is not empty, change working directory
    if (args[2].find('/') != std::string::npos)
        result_cd_dir = this->cd(cd_args_dir);

    // Error could have occurred while changing working directory
    if (!result_cd_dir) {
        working_directory = saved_working_directory;
        return false;
    }

    // Check if file with the given name exists
    auto existence_check = DirectoryEntry{};
    if (does_entry_exist(new_file_name, existence_check)) {
        std::cerr << FILE_ALREADY_EXISTS << std::endl;
        working_directory = saved_working_directory;
        return false;
    }

    // Everything looks fine, let's move the file - remove the entry from the old path
    working_directory = saved_working_directory;
    if (args[1].find('/') != std::string::npos)
        cd(cd_args_file);
    remove_directory_entry(working_directory.cluster_address, entry);

    // Modify the file name
    for (int i = 0; i < DEFAULT_FILE_NAME_LENGTH - 1; i++)
        entry.item_name[i] = new_file_name[i];
    entry.item_name[DEFAULT_FILE_NAME_LENGTH - 1] = '\0';

    // Add the entry to the new path
    working_directory = saved_working_directory;
    if (args[2].find('/') != std::string::npos)
        cd(cd_args_dir);
    write_directory_entry(working_directory.cluster_address, entry);

    // Restore and update working directory
    working_directory = saved_working_directory;
    working_directory.entries = get_directory_entries(working_directory.cluster_address);

    std::cout << OK << std::endl;
    return true;
}

bool PseudoFS::rm(const std::vector<std::string> &args) {
    // Change working directory
    auto saved_working_directory = working_directory;
    std::string file_name = args[1].substr(args[1].find_last_of('/') + 1, args[1].size());
    std::string file_path = args[1].substr(0, args[1].find_last_of('/'));
    std::vector<std::string> cd_args = {"cd", file_path, "don't print ok"};
    bool result_cd = true;
    // If directory path is not empty, change working directory
    if (args[1].find('/') != std::string::npos)
        result_cd = this->cd(cd_args);

    // Error could have occurred while changing working directory
    if (!result_cd) {
        working_directory = saved_working_directory;
        return false;
    }

    // Check if file with the given name exists
    auto entry = DirectoryEntry{};
    if (!does_entry_exist(file_name, entry)) {
        std::cerr << FILE_NOT_FOUND << std::endl;
        working_directory = saved_working_directory;
        return false;
    }

    // Check if file is not a directory
    if (entry.is_directory) {
        std::cerr << FILE_IS_DIRECTORY << std::endl;
        working_directory = saved_working_directory;
        return false;
    }

    // Remove file
    auto cluster_address = entry.start_cluster;
    auto cluster_index = get_cluster_index(cluster_address);
    auto number_of_iterations = entry.size / meta_data.cluster_size;
    if (entry.size % meta_data.cluster_size)
        number_of_iterations++;

    for (int i = 0; i < number_of_iterations; i++) {
        write_to_cluster(cluster_address, EMPTY_CLUSTER, static_cast<int>(meta_data.cluster_size));

        auto next_cluster = read_from_fat(cluster_index);
        write_to_fat(cluster_index, FAT_FREE);

        cluster_address = next_cluster;
        cluster_index = get_cluster_index(cluster_address);
    }
    write_to_fat(cluster_index, FAT_FREE);

    // Remove entry from directory
    remove_directory_entry(working_directory.cluster_address, entry);

    // Restore and update working directory
    working_directory = saved_working_directory;
    working_directory.entries = get_directory_entries(working_directory.cluster_address);

    std::cout << OK << std::endl;
    return true;
}

bool PseudoFS::mkdir(const std::vector<std::string> &args) {
    // Change working directory
    auto saved_working_directory = working_directory;
    std::string dir_name = args[1].substr(args[1].find_last_of('/') + 1, args[1].size());
    std::string dir_path = args[1].substr(0, args[1].find_last_of('/'));
    std::vector<std::string> cd_args = {"cd", dir_path, "don't print ok"};
    bool result_cd = true;
    // If directory path is not empty, change working directory
    if (args[1].find('/') != std::string::npos)
        result_cd = this->cd(cd_args);

    // Error could have occurred while changing working directory
    if (!result_cd) {
        working_directory = saved_working_directory;
        return false;
    }

    // Check if directory (or file) with the same name already exists
    auto existance_check = DirectoryEntry{};
    if (does_entry_exist(dir_name, existance_check)) {
        std::cerr << DIRECTORY_ALREADY_EXISTS << std::endl;
        working_directory = saved_working_directory;
        return false;
    }

    // Find free cluster
    auto index = find_free_cluster();
    if (!index) {
        std::cerr << NO_SPACE << std::endl;
        working_directory = saved_working_directory;
        return false;
    }

    // Calculate address and index of the cluster
    auto cluster_address = meta_data.data_start_address + index * meta_data.cluster_size;
    auto cluster_index = get_cluster_index(cluster_address);

    // Mark cluster as used in FAT table
    write_to_fat(cluster_index, FAT_EOF);

    // Create new directory entry
    DirectoryEntry entry{"", true, 0, cluster_address};
    for (int i = 0; i < DEFAULT_FILE_NAME_LENGTH - 1; i++)
        entry.item_name[i] = dir_name[i];
    entry.item_name[DEFAULT_FILE_NAME_LENGTH - 1] = '\0';
    DirectoryEntry this_entry{".", true, 0, cluster_address};
    DirectoryEntry parent_entry{"..", true, 0, working_directory.cluster_address};

    // Write current and parent directory entries to the cluster
    write_directory_entry(cluster_address, this_entry);
    write_directory_entry(cluster_address, parent_entry);

    // Add new directory entry to parent directory
    write_directory_entry(working_directory.cluster_address, entry);

    // Restore and update working directory
    working_directory = saved_working_directory;
    working_directory.entries = get_directory_entries(working_directory.cluster_address);

    std::cout << OK << std::endl;
    return true;
}

bool PseudoFS::rmdir(const std::vector<std::string> &args) {
    // Change working directory
    auto saved_working_directory = working_directory;
    std::string dir_name = args[1].substr(args[1].find_last_of('/') + 1, args[1].size());
    if (dir_name == ".") {
        std::cerr << CANNOT_REMOVE_CURR_DIR << std::endl;
        return false;
    }
    std::string dir_path = args[1].substr(0, args[1].find_last_of('/'));
    std::vector<std::string> cd_args = {"cd", dir_path, "don't print ok"};
    bool result_cd = true;
    // If directory path is not empty, change working directory
    if (args[1].find('/') != std::string::npos)
        result_cd = this->cd(cd_args);

    // Error could have occurred while changing working directory
    if (!result_cd) {
        working_directory = saved_working_directory;
        return false;
    }

    // Check if directory with the given name exists
    auto entry = DirectoryEntry{};
    if (!does_entry_exist(dir_name, entry)) {
        std::cerr << DIRECTORY_NOT_FOUND << std::endl;
        working_directory = saved_working_directory;
        return false;
    }

    // Check if it actually is a directory
    if (!entry.is_directory) {
        std::cerr << FILE_IS_NOT_DIRECTORY << std::endl;
        working_directory = saved_working_directory;
        return false;
    }

    // Check if directory is empty
    auto entries = get_directory_entries(entry.start_cluster);
    if (entries.size() > 2) {
        std::cerr << DIRECTORY_IS_NOT_EMPTY << std::endl;
        working_directory = saved_working_directory;
        return false;
    }

    // Mark cluster as free in FAT table
    auto cluster_index = get_cluster_index(entry.start_cluster);
    write_to_fat(cluster_index, FAT_FREE);

    // Remove directory entry from parent directory
    remove_directory_entry(working_directory.cluster_address, entry);

    // Mark cluster as free by putting zeroes in it
    write_to_cluster(entry.start_cluster, EMPTY_CLUSTER, static_cast<int>(meta_data.cluster_size));

    // Restore and update working directory
    working_directory = saved_working_directory;
    working_directory.entries = get_directory_entries(working_directory.cluster_address);

    std::cout << OK << std::endl;
    return true;
}

bool PseudoFS::ls(const std::vector<std::string> &args) {
    // If argument is given, change working directory
    bool result_cd = true;
    auto saved_working_directory = working_directory;
    if (args.size() > 1) {
        std::vector<std::string> cd_args = {"cd", args[1], "don't print ok"};
        result_cd = this->cd(cd_args);
    }

    // Error could have occurred while changing working directory
    if (!result_cd) {
        working_directory = saved_working_directory;
        return false;
    }

    // List directory entries in working directory
    for (const auto &entry: working_directory.entries) {
        std::cout << entry.item_name << " ";
        if (entry.is_directory)
            std::cout << "<DIR> ";
        else
            std::cout << "<FILE> ";
        std::cout << entry.size << "B ";
        std::cout << entry.start_cluster << std::endl;
    }

    // Restore working directory
    working_directory = saved_working_directory;

    return true;
}

bool PseudoFS::cat(const std::vector<std::string> &args) {
    // Change working directory
    auto saved_working_directory = working_directory;
    std::string file_name = args[1].substr(args[1].find_last_of('/') + 1, args[1].size());
    std::string file_path = args[1].substr(0, args[1].find_last_of('/'));
    std::vector<std::string> cd_args = {"cd", file_path, "don't print ok"};
    bool result_cd = true;
    // If directory path is not empty, change working directory
    if (args[1].find('/') != std::string::npos)
        result_cd = this->cd(cd_args);

    // Error could have occurred while changing working directory
    if (!result_cd) {
        working_directory = saved_working_directory;
        return false;
    }

    // Check if file with the given name exists
    auto entry = DirectoryEntry{};
    if (!does_entry_exist(file_name, entry)) {
        std::cerr << FILE_NOT_FOUND << std::endl;
        working_directory = saved_working_directory;
        return false;
    }

    // Check if it actually is a file
    if (entry.is_directory) {
        std::cerr << FILE_IS_DIRECTORY << std::endl;
        working_directory = saved_working_directory;
        return false;
    }

    // Read file
    auto cluster_address = entry.start_cluster;
    auto cluster_index = get_cluster_index(cluster_address);
    auto number_of_iterations = entry.size / meta_data.cluster_size;
    if (entry.size % meta_data.cluster_size != 0)
        number_of_iterations++;

    for (int i = 0; i < number_of_iterations; i++) {
        if (i != number_of_iterations - 1) {
            auto bytes_to_read = meta_data.cluster_size;
            auto buffer = read_from_cluster(cluster_address, static_cast<int>(bytes_to_read));
            std::cout << buffer;
        }
            // Last iteration
        else {
            auto bytes_to_read = entry.size % meta_data.cluster_size;
            auto buffer = read_from_cluster(cluster_address, static_cast<int>(bytes_to_read));
            std::cout << buffer << std::endl;
        }

        cluster_address = read_from_fat(cluster_index);
        cluster_index = get_cluster_index(cluster_address);
    }

    // Restore working directory
    working_directory = saved_working_directory;

    return true;
}

bool PseudoFS::cd(const std::vector<std::string> &args) {
    // If no argument is given, go to root directory
    if (args.size() == 1 || args[1].empty() || args[1] == "/") {
        change_directory("");
        if (args.size() == 2) // When other functions use this function, they don't want to print OK
            std::cout << OK << std::endl;
        return true;
    }

    // Save current working directory
    auto saved_working_directory = working_directory;

    // If argument is given, tokenize the path and go to the directory
    std::stringstream ss(args[1]);
    std::string token;
    std::vector<std::string> path_tokenized;
    while (std::getline(ss, token, '/'))
        path_tokenized.push_back(token);

    // Go to the directory one by one
    for (const auto &path: path_tokenized)
        if (!change_directory(path)) {
            working_directory = saved_working_directory;
            std::cerr << PATH_NOT_FOUND << std::endl;
            return false;
        }

    if (args.size() == 2) // When other functions use this function, they don't want to print OK
        std::cout << OK << std::endl;
    return true;
}

bool PseudoFS::pwd(const std::vector<std::string> &args) {
    std::cout << working_directory.path << std::endl;
    return true;
}

bool PseudoFS::info(const std::vector<std::string> &args) {
    // Change working directory
    auto saved_working_directory = working_directory;
    std::string file_name = args[1].substr(args[1].find_last_of('/') + 1, args[1].size());
    std::string file_path = args[1].substr(0, args[1].find_last_of('/'));
    std::vector<std::string> cd_args = {"cd", file_path, "don't print ok"};
    bool result_cd = true;
    // If directory path is not empty, change working directory
    if (args[1].find('/') != std::string::npos)
        result_cd = this->cd(cd_args);

    // Error could have occurred while changing working directory
    if (!result_cd) {
        working_directory = saved_working_directory;
        return false;
    }

    // Check if file with the given name exists
    auto entry = DirectoryEntry{};
    if (!does_entry_exist(file_name, entry)) {
        std::cerr << FILE_NOT_FOUND << std::endl;
        working_directory = saved_working_directory;
        return false;
    }

    // Print file info
    std::cout << "File name: " << entry.item_name << std::endl;
    if (entry.is_directory)
        std::cout << "Type: directory" << std::endl;
    else
        std::cout << "Type: file" << std::endl;
    std::cout << "File size: " << entry.size << "B" << std::endl;
    std::cout << "File start cluster address: " << entry.start_cluster << std::endl;
    std::cout << "File clusters: ";
    auto cluster_address = entry.start_cluster;
    auto cluster_index = get_cluster_index(cluster_address);
    while (cluster_address != FAT_EOF) {
        std::cout << (cluster_index - meta_data.fat_start_address) / sizeof(uint32_t) << " ";
        cluster_address = read_from_fat(cluster_index);
        cluster_index = get_cluster_index(cluster_address);
    }
    std::cout << std::endl;

    // Restore working directory
    working_directory = saved_working_directory;

    return true;
}

bool PseudoFS::incp(const std::vector<std::string> &args) {
    // Change working directory
    auto saved_working_directory = working_directory;
    std::string file_name = args[2].substr(args[2].find_last_of('/') + 1, args[2].size());
    std::string file_path = args[2].substr(0, args[2].find_last_of('/'));
    std::vector<std::string> cd_args = {"cd", file_path, "don't print ok"};
    bool result_cd = true;
    // If directory path is not empty, change working directory
    if (args[2].find('/') != std::string::npos)
        result_cd = this->cd(cd_args);

    // Error could have occurred while changing working directory
    if (!result_cd) {
        working_directory = saved_working_directory;
        return false;
    }

    // Open source file from hard drive
    std::ifstream source_file(args[1], std::ios::binary | std::ios::in);
    if (!source_file.is_open()) {
        std::cerr << FILE_NOT_FOUND << std::endl;
        working_directory = saved_working_directory;
        return false;
    }

    // Check if file with the same name already exists
    auto existence_check = DirectoryEntry{};
    if (does_entry_exist(file_name, existence_check)) {
        std::cerr << FILE_ALREADY_EXISTS << std::endl;
        working_directory = saved_working_directory;
        return false;
    }

    // Get file size
    source_file.seekg(0, std::ios::end);
    uint32_t file_size = source_file.tellg();
    source_file.seekg(0, std::ios::beg);
    auto number_of_iterations = file_size / meta_data.cluster_size;
    if (file_size % meta_data.cluster_size != 0)
        number_of_iterations++;

    // Iterate over clusters and write data to them from source file (and write cluster addresses to FAT)
    auto index = find_free_cluster();
    if (!index) {
        std::cerr << NO_SPACE << std::endl;
        working_directory = saved_working_directory;
        return false;
    }
    auto current_cluster_index = meta_data.fat_start_address + index * sizeof(uint32_t);
    auto current_cluster_address = get_cluster_address(current_cluster_index);
    write_to_fat(current_cluster_index, FAT_EOF); // Marking as used; EOF will do for now, later it will be changed
    uint32_t previous_cluster_index = 0;

    // Create directory entry
    auto entry = DirectoryEntry{
            "",
            false,
            file_size,
            current_cluster_address,
    };
    for (int i = 0; i < DEFAULT_FILE_NAME_LENGTH - 1; i++)
        entry.item_name[i] = file_name[i];
    entry.item_name[DEFAULT_FILE_NAME_LENGTH - 1] = '\0';

    for (auto i = 0; i < number_of_iterations; i++) {
        // Write cluster address to FAT
        if (previous_cluster_index != 0)
            write_to_fat(previous_cluster_index, current_cluster_address);

        previous_cluster_index = current_cluster_index;

        // Write data to cluster
        if (i != number_of_iterations - 1) {
            char buffer[meta_data.cluster_size];
            source_file.seekg(i * meta_data.cluster_size);
            source_file.read(buffer, static_cast<int>(meta_data.cluster_size));
            write_to_cluster(current_cluster_address, buffer, static_cast<int>(meta_data.cluster_size));
        }
            // Last iteration
        else {
            char buffer[file_size % meta_data.cluster_size];
            source_file.seekg(i * meta_data.cluster_size);
            source_file.read(buffer, static_cast<int>(file_size % meta_data.cluster_size));
            write_to_cluster(current_cluster_address, buffer, static_cast<int>(file_size % meta_data.cluster_size));
            break;
        }

        // Find next free cluster
        index = find_free_cluster();
        if (!index) {
            std::cerr << NO_SPACE << std::endl;
            working_directory = saved_working_directory;
            return false;
        }
        current_cluster_index = meta_data.fat_start_address + index * sizeof(uint32_t);
        write_to_fat(current_cluster_index, FAT_EOF); // Marking as used; EOF will do for now, later it will be changed
        current_cluster_address = get_cluster_address(current_cluster_index);
    }
    // Write directory entry to directory
    write_directory_entry(working_directory.cluster_address, entry);

    // Close source file
    source_file.close();

    // Restore and update working directory
    working_directory = saved_working_directory;
    working_directory.entries = get_directory_entries(working_directory.cluster_address);

    std::cout << OK << std::endl;
    return true;
}

bool PseudoFS::outcp(const std::vector<std::string> &args) {
    // Change working directory
    auto saved_working_directory = working_directory;
    std::string file_name = args[1].substr(args[1].find_last_of('/') + 1, args[1].size());
    std::string file_path = args[1].substr(0, args[1].find_last_of('/'));
    std::vector<std::string> cd_args = {"cd", file_path, "don't print ok"};
    bool result_cd = true;
    // If directory path is not empty, change working directory
    if (args[1].find('/') != std::string::npos)
        result_cd = this->cd(cd_args);

    // Error could have occurred while changing working directory
    if (!result_cd) {
        working_directory = saved_working_directory;
        return false;
    }

    // Open destination file from hard drive
    std::ofstream destination_file(args[2], std::ios::binary | std::ios::out | std::ios::trunc);
    if (!destination_file.is_open()) {
        std::cerr << PATH_NOT_FOUND << std::endl;
        working_directory = saved_working_directory;
        return false;
    }

    // Find directory entry with the given name and check existence
    auto entry = DirectoryEntry{};
    if (!does_entry_exist(file_name, entry)) {
        std::cerr << FILE_NOT_FOUND << std::endl;
        working_directory = saved_working_directory;
        return false;
    }

    // Iterate over clusters and write data to destination file
    auto current_cluster_address = entry.start_cluster;
    auto current_cluster_index = get_cluster_index(current_cluster_address);
    auto number_of_iterations = entry.size / meta_data.cluster_size;
    if (entry.size % meta_data.cluster_size != 0)
        number_of_iterations++;

    for (auto i = 0; i < number_of_iterations; i++) {
        // Read data from cluster
        if (i != number_of_iterations - 1) {
            std::string buffer = read_from_cluster(current_cluster_address, static_cast<int>(meta_data.cluster_size));
            destination_file.write(buffer.c_str(), static_cast<int>(meta_data.cluster_size));
        }
            // Last iteration
        else {
            std::string buffer = read_from_cluster(current_cluster_address,
                                                   static_cast<int>(entry.size % meta_data.cluster_size));
            destination_file.write(buffer.c_str(), static_cast<int>(entry.size % meta_data.cluster_size));
            break;
        }

        // Find next cluster
        current_cluster_address = read_from_fat(current_cluster_index);
        current_cluster_index = get_cluster_index(current_cluster_address);
    }

    // Close destination file
    destination_file.close();

    // Restore working directory
    working_directory = saved_working_directory;

    std::cout << OK << std::endl;
    return true;
}

bool PseudoFS::load(const std::vector<std::string> &args) {
    // Open file from hard drive
    std::ifstream command_file(args[1]);
    if (!command_file.is_open()) {
        std::cerr << PATH_NOT_FOUND << std::endl;
        return false;
    }

    // Read commands from file
    std::string command;
    std::string token;
    std::vector<std::string> tokens;
    while (std::getline(command_file, command)) {
        std::stringstream ss(command);
        tokens.clear();
        while (std::getline(ss, token, ' '))
            tokens.push_back(token);

        std::cout << working_directory.path << "$ >" << command << std::endl;
        call_cmd(tokens[0], tokens);
    }

    std::cout << OK << std::endl;
    return true;
}

bool PseudoFS::format(const std::vector<std::string> &args) {
    // Get the user input for the disk size
    uint32_t disk_size = std::stoi(args[1]);
    if (args[1].find("KB") != std::string::npos)
        disk_size *= KB;
    else if (args[1].find("MB") != std::string::npos)
        disk_size *= MB;
    else if (args[1].find("GB") != std::string::npos)
        disk_size *= GB;

    // Calculate remaining size (size for FAT table and data)
    uint32_t remaining_size = disk_size - sizeof(MetaData);
    uint32_t num_blocks = remaining_size / (DEFAULT_CLUSTER_SIZE + sizeof(uint32_t));

    // Create the metadata for the file system
    meta_data = MetaData{
            "zapped99",
            disk_size,
            DEFAULT_CLUSTER_SIZE,
            (disk_size - (sizeof(MetaData) + num_blocks * sizeof(uint32_t))) / DEFAULT_CLUSTER_SIZE,
            sizeof(MetaData),
            num_blocks * sizeof(uint32_t),
            sizeof(MetaData) + num_blocks * sizeof(uint32_t)
    };
    // Create root directory
    auto root_dir_curr = DirectoryEntry{
            ".",
            true,
            0,
            meta_data.data_start_address
    };
    auto root_dir_parent = DirectoryEntry{
            "..",
            true,
            0,
            meta_data.data_start_address
    };

    // Rewrite the file system file
    if (file_system.is_open()) file_system.close();
    file_system.open(file_system_filepath, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);

    // Write the meta data
    file_system.write(reinterpret_cast<const char *>(&meta_data), sizeof(struct MetaData));

    // Write the FAT table (all clusters are free)
    for (uint32_t i = 0; i < meta_data.cluster_count; i++)
        write_to_fat(meta_data.fat_start_address + i * sizeof(uint32_t), FAT_FREE);

    // Write the data (no data)
    EMPTY_CLUSTER = std::string(meta_data.cluster_size, '\0');
    for (uint32_t i = 0; i < meta_data.cluster_count; i++)
        write_to_cluster(meta_data.data_start_address + i * meta_data.cluster_size, EMPTY_CLUSTER,
                         static_cast<int>(meta_data.cluster_size));

    // Write the root directory to FAT table and data
    write_to_fat(meta_data.fat_start_address, FAT_EOF);
    write_directory_entry(meta_data.data_start_address, root_dir_curr);
    write_directory_entry(meta_data.data_start_address, root_dir_parent);

    // Set the working directory to root
    ROOT_DIRECTORY = WorkingDirectory{
            meta_data.data_start_address,
            "/",
            get_directory_entries(meta_data.data_start_address)
    };
    working_directory = ROOT_DIRECTORY;

    std::cout << OK << std::endl;
    return true;
}

bool PseudoFS::defrag(const std::vector<std::string> &args) {
    // Check if the filepath is valid
    auto saved_working_directory = working_directory;
    std::string file_name = args[1].substr(args[1].find_last_of('/') + 1, args[1].size());
    std::string file_path = args[1].substr(0, args[1].find_last_of('/'));
    std::vector<std::string> cd_args_file = {"cd", file_path, "don't print ok"};
    bool result_cd_file = true;
    // If directory path is not empty, change working directory
    if (args[1].find('/') != std::string::npos)
        result_cd_file = this->cd(cd_args_file);

    // Error could have occurred while changing working directory
    if (!result_cd_file) {
        working_directory = saved_working_directory;
        return false;
    }

    // Check if file with the given name exists
    auto entry = DirectoryEntry{};
    if (!does_entry_exist(file_name, entry)) {
        std::cerr << FILE_NOT_FOUND << std::endl;
        working_directory = saved_working_directory;
        return false;
    }

    // Check if the source file is a directory
    if (entry.is_directory) {
        std::cerr << FILE_IS_DIRECTORY << std::endl;
        working_directory = saved_working_directory;
        return false;
    }

    // Check if the file is already defragmented
    std::vector<uint32_t> clusters;
    if (is_file_defragmented(entry, clusters)) {
        working_directory = saved_working_directory;
        return true;
    }

    // Find new clusters that are consecutive
    auto number_of_needed_consecutive_clusters = clusters.size();
    std::vector<uint32_t> new_clusters;
    for (;;) {
        bool found = true;
        auto index = find_free_cluster();
        new_clusters.push_back(index);
        write_to_fat(meta_data.fat_start_address + index * sizeof(uint32_t), FAT_EOF); // Mark the cluster as used
        if (new_clusters.size() == number_of_needed_consecutive_clusters) {
            for (int i = 0; i < number_of_needed_consecutive_clusters - 1; i++) {
                if (new_clusters[i] + 1 != new_clusters[i + 1]) {
                    write_to_fat(meta_data.fat_start_address + new_clusters[0] * sizeof(uint32_t),
                                 42); // Mark the cluster as "free"
                    new_clusters.erase(new_clusters.begin());
                    found = false;
                    break;
                }
            }
            if (found) break;
        }
    }
    // Truly free clusters that were markes as "free"
    for (int i = 0; i < meta_data.cluster_count; i++) {
        uint32_t fat_value = read_from_fat(meta_data.fat_start_address + i * sizeof(uint32_t));
        if (fat_value == 42)
            write_to_fat(meta_data.fat_start_address + i * sizeof(uint32_t), FAT_FREE);
    }

    // Copy the data from the old clusters to the new ones
    for (int i = 0; i < number_of_needed_consecutive_clusters; i++) {
        std::string data = read_from_cluster(meta_data.data_start_address + clusters[i] * meta_data.cluster_size,
                                             static_cast<int>(meta_data.cluster_size));
        write_to_cluster(meta_data.data_start_address + new_clusters[i] * meta_data.cluster_size, data,
                         static_cast<int>(meta_data.cluster_size));
    }

    // Update the FAT table
    for (int i = 0; i < number_of_needed_consecutive_clusters - 1; i++)
        write_to_fat(meta_data.fat_start_address + new_clusters[i] * sizeof(uint32_t),
                     meta_data.data_start_address + new_clusters[i + 1] * meta_data.cluster_size);

    // Free the old clusters
    for (int i = 0; i < number_of_needed_consecutive_clusters; i++) {
        write_to_fat(meta_data.fat_start_address + clusters[i] * sizeof(uint32_t), FAT_FREE);
        write_to_cluster(meta_data.data_start_address + clusters[i] * meta_data.cluster_size, EMPTY_CLUSTER,
                         static_cast<int>(meta_data.cluster_size));
    }

    // Update the file entry
    auto new_entry = DirectoryEntry{
            "",
            entry.is_directory,
            entry.size,
            meta_data.data_start_address + new_clusters[0] * meta_data.cluster_size
    };
    for (int i = 0; i < DEFAULT_FILE_NAME_LENGTH - 1; i++)
        new_entry.item_name[i] = entry.item_name[i];
    remove_directory_entry(working_directory.cluster_address, entry);
    write_directory_entry(working_directory.cluster_address, new_entry);

    // Update and restore working directory
    working_directory = saved_working_directory;
    working_directory.entries = get_directory_entries(working_directory.cluster_address);

    std::cout << OK << std::endl;
    return true;
}
