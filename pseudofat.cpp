#include "pseudofat.h"

PseudoFS::PseudoFS(const std::string &filepath) : file_system_filepath{filepath}, meta_data{}, working_directory{} {
    file_system.open(filepath, std::ios::binary | std::ios::in | std::ios::out);
    if (!file_system.is_open())
        file_system.open(filepath, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
    else {
        file_system.read(reinterpret_cast<char *>(&meta_data), sizeof(MetaData));
        working_directory = WorkingDirectory {
            meta_data.data_start_address,
            "/",
            get_directory_entries(meta_data.data_start_address)
        };
    }
    if (!file_system.is_open())
        std::cerr << "Error opening file system file" << std::endl;
    initialize_command_map();
}

PseudoFS::~PseudoFS() {
    file_system.close();
}

void PseudoFS::initialize_command_map() {
    commands["help"] = &PseudoFS::help;
    commands["meta"] = &PseudoFS::meta;
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

std::vector<DirectoryEntry> PseudoFS::get_directory_entries(uint32_t cluster) {
    std::vector<DirectoryEntry> entries;
    file_system.seekp(cluster);
    for (int i = 0; i < DEFAULT_CLUSTER_SIZE / sizeof(DirectoryEntry); i++) {
        DirectoryEntry entry{};
        file_system.read(reinterpret_cast<char *>(&entry), sizeof(DirectoryEntry));
        if (entry.start_cluster != 0)
            entries.push_back(entry);
    }
    return entries;
}

uint32_t PseudoFS::find_free_cluster() {
    for (int i = 0; i < meta_data.cluster_count; i++) {
        uint32_t cluster{};
        file_system.seekp(meta_data.fat_start_address + i * sizeof(uint32_t));
        file_system.read(reinterpret_cast<char *>(&cluster), sizeof(uint32_t));
        if (cluster == FAT_FREE)
            return i;
    }
    return 0;
}

void PseudoFS::call_cmd(const std::string &cmd, const std::vector<std::string> &args) {
    if (commands.count(cmd))
        (this->*commands[cmd])(args);
    else {
        std::cerr << "Unknown command: " << cmd << std::endl;
        std::cerr << "Type 'help' for a list of commands" << std::endl;
    }
}

std::string PseudoFS::get_working_directory() const {
    return working_directory.path;
}


bool PseudoFS::help(const std::vector<std::string> &args) {
    std::cout << "-------------------------------------------------------------------------------" << std::endl;
    std::cout << "| help              | display this message                                    |" << std::endl;
    std::cout << "| exit              | exit the program                                        |" << std::endl;
    std::cout << "| meta              | display meta information about the file system          |" << std::endl;
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

bool PseudoFS::cp(const std::vector<std::string> &args) {
    return false;
}

bool PseudoFS::mv(const std::vector<std::string> &args) {
    return false;
}

bool PseudoFS::rm(const std::vector<std::string> &args) {
    return false;
}

bool PseudoFS::mkdir(const std::vector<std::string> &args) {
    // If argument is given, change working directory
    auto saved_working_directory = working_directory;
    std::string dir_name = args[1].substr(args[1].find_last_of('/') + 1);
    std::string dir_path = args[1].substr(0, args[1].find_last_of('/'));
    std::vector<std::string> cd_args = {"cd", dir_path};
    bool result_cd = this->cd(cd_args);

    // Error could have occurred while changing working directory
    if (!result_cd)
        return false;

    // Check if directory (or file) with the same name already exists
    for (const auto &entry : working_directory.entries) {
        if (entry.item_name == dir_name) {
            std::cerr << "ERROR: EXISTS" << std::endl;
            return false;
        }
    }

    // Find free cluster
    auto cluster_index = find_free_cluster();
    if (!cluster_index) {
        std::cerr << "ERROR: NO SPACE" << std::endl;
        return false;
    }
    // Calculate address of the cluster
    auto cluster_address = meta_data.data_start_address + cluster_index * meta_data.cluster_size;

    // Mark cluster as used in FAT table
    file_system.seekp(meta_data.fat_start_address + cluster_index * sizeof(uint32_t));
    file_system.write(reinterpret_cast<const char *>(&FAT_EOF), sizeof(uint32_t));

    // Create new directory entry
    DirectoryEntry entry{"", true, 0, cluster_address};
    for (int i = 0; i < DEFAULT_FILE_NAME_LENGTH - 1; i++)
        entry.item_name[i] = dir_name[i];
    entry.item_name[DEFAULT_FILE_NAME_LENGTH - 1] = '\0';
    DirectoryEntry this_entry{".", true, 0, cluster_address};
    DirectoryEntry parent_entry{"..", true, 0, working_directory.cluster_address};

    // Write current and parent directory entries to the cluster
    file_system.seekp(cluster_address);
    file_system.write(reinterpret_cast<const char *>(&this_entry), sizeof(DirectoryEntry));
    file_system.write(reinterpret_cast<const char *>(&parent_entry), sizeof(DirectoryEntry));

    // Add new directory entry to parent directory
    file_system.seekp(working_directory.cluster_address);
    for (int i = 0; i < DEFAULT_CLUSTER_SIZE / sizeof(DirectoryEntry); i++) {
        auto tmp = DirectoryEntry{};
        file_system.read(reinterpret_cast<char *>(&tmp), sizeof(DirectoryEntry));
        if (tmp.start_cluster != 0)
            continue;
        file_system.seekp(working_directory.cluster_address + i * sizeof(DirectoryEntry));
        file_system.write(reinterpret_cast<const char *>(&entry), sizeof(DirectoryEntry));
        break;
    }

    // Restore and update working directory
    working_directory = saved_working_directory;
    working_directory.entries = get_directory_entries(working_directory.cluster_address);

    std::cout << "OK" << std::endl;
    return true;
}

bool PseudoFS::rmdir(const std::vector<std::string> &args) {
    // If argument is given, change working directory
    auto saved_working_directory = working_directory;
    std::string dir_name = args[1].substr(args[1].find_last_of('/') + 1);
    std::string dir_path = args[1].substr(0, args[1].find_last_of('/'));
    std::vector<std::string> cd_args = {"cd", dir_path};
    bool result_cd = this->cd(cd_args);

    // Error could have occurred while changing working directory
    if (!result_cd)
        return false;

    // Check if directory with the given name exists
    auto entry = DirectoryEntry{};
    for (const auto &entry_for : working_directory.entries) {
        if (entry_for.item_name == dir_name) {
            entry = entry_for;
            break;
        }
    }
    if (entry.start_cluster == 0) {
        std::cerr << "ERROR: DIR NOT FOUND" << std::endl;
        return false;
    }

    // Check if directory is empty
    auto entries = get_directory_entries(entry.start_cluster);
    if (entries.size() > 2) {
        std::cerr << "ERROR: DIR NOT EMPTY" << std::endl;
        return false;
    }

    // Mark cluster as free in FAT table
    auto cluster_index = (entry.start_cluster - meta_data.data_start_address) / meta_data.cluster_size;
    file_system.seekp(meta_data.fat_start_address + cluster_index * sizeof(uint32_t));
    file_system.write(reinterpret_cast<const char *>(&FAT_FREE), sizeof(uint32_t));

    // Remove directory entry from parent directory
    for (int i = 0; i < DEFAULT_CLUSTER_SIZE / sizeof(DirectoryEntry); i++) {
        auto tmp = DirectoryEntry{};
        file_system.seekp(working_directory.cluster_address + i * sizeof(DirectoryEntry));
        file_system.read(reinterpret_cast<char *>(&tmp), sizeof(DirectoryEntry));
        if (tmp.start_cluster == entry.start_cluster) {
            file_system.seekp(working_directory.cluster_address + i * sizeof(DirectoryEntry));
            DirectoryEntry empty_entry{};
            file_system.write(reinterpret_cast<const char *>(&empty_entry), sizeof(DirectoryEntry));
            break;
        }
    }

    // Mark cluster as free by putting zeroes in it
    file_system.seekp(entry.start_cluster);
    for (int i = 0; i < DEFAULT_CLUSTER_SIZE; i++)
        file_system.write("\0", sizeof(char));

    // Restore and update working directory
    working_directory = saved_working_directory;
    working_directory.entries = get_directory_entries(working_directory.cluster_address);

    std::cout << "OK" << std::endl;
    return true;
}

bool PseudoFS::ls(const std::vector<std::string> &args) {
    // If argument is given, change working directory
    bool result_cd = true;
    auto saved_working_directory = working_directory;
    if (args.size() > 1)
        result_cd = this->cd(args);

    // Error could have occurred while changing working directory
    if (!result_cd)
        return false;

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
    return false;
}

bool PseudoFS::cd(const std::vector<std::string> &args) {
    // If no argument is given, go to root directory
    if (args.size() == 1) {
        working_directory = WorkingDirectory{
            meta_data.data_start_address,
            "/",
            get_directory_entries(meta_data.data_start_address)
        };
        std::cout << "OK" << std::endl;
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

    // If path starts with '/', go to root directory first
    if (path_tokenized[0].empty()) {
        working_directory = WorkingDirectory{
            meta_data.data_start_address,
            "/",
            get_directory_entries(meta_data.data_start_address)
        };
        path_tokenized.erase(path_tokenized.begin());
    }

    for (const auto &path : path_tokenized) {
        // Find directory entry with the given name
        auto entry = DirectoryEntry{};
        for (const auto &entry_for: working_directory.entries) {
            if (entry_for.item_name == path) {
                entry = entry_for;
                break;
            }
        }

        // Check if directory with the given name exists
        if (entry.start_cluster == 0) {
            std::cerr << "ERROR: PATH NOT FOUND" << std::endl;
            working_directory = saved_working_directory;
            return false;
        }

        // Go to the directory = update working directory
        if (entry.item_name[0] == '.') {
            // If argument is ".." go to parent directory
            if (entry.item_name[1] == '.') {
                working_directory.path = working_directory.path.substr(0, working_directory.path.find_last_of('/'));
                working_directory.path = working_directory.path.substr(0, working_directory.path.find_last_of('/'));
                working_directory.path += "/";
            }
                // If argument is "." do nothing
            else
                continue;
        }
            // If argument is a directory name, go to the directory
        else {
            working_directory.path += entry.item_name;
            working_directory.path += "/";
        }
        // Update working directory
        working_directory.cluster_address = entry.start_cluster;
        working_directory.entries = get_directory_entries(entry.start_cluster);
    }

    std::cout << "OK" << std::endl;
    return true;
}

bool PseudoFS::pwd(const std::vector<std::string> &args) {
    std::cout << working_directory.path << std::endl;
    return true;
}

bool PseudoFS::info(const std::vector<std::string> &args) {
    return false;
}

bool PseudoFS::incp(const std::vector<std::string> &args) {
    return false;
}

bool PseudoFS::outcp(const std::vector<std::string> &args) {
    return false;
}

bool PseudoFS::load(const std::vector<std::string> &args) {
    return false;
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
    auto root_dir = DirectoryEntry{
        ".",
        true,
        0,
        meta_data.data_start_address,
    };
    auto root_dir_parent = DirectoryEntry{
        "..",
        true,
        0,
        meta_data.data_start_address,
    };

    // Rewrite the file system file
    if (file_system.is_open()) file_system.close();
    file_system.open(file_system_filepath, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);

    // Write the meta data
    file_system.write(reinterpret_cast<const char *>(&meta_data), sizeof(struct MetaData));

    // Write the FAT table (all clusters are free)
    for (uint32_t i = 0; i < meta_data.cluster_count; i++)
        file_system.write(reinterpret_cast<const char *>(&FAT_FREE), sizeof(uint32_t));

    // Write the data (no data)
    for (uint32_t i = 0; i < meta_data.cluster_count * meta_data.cluster_size; i++)
        file_system.write("\0", sizeof(char));

    // Write the root directory to FAT table and data
    file_system.seekp(meta_data.fat_start_address);
    file_system.write(reinterpret_cast<const char *>(&FAT_EOF), sizeof(uint32_t));
    file_system.seekp(meta_data.data_start_address);
    file_system.write(reinterpret_cast<const char *>(&root_dir), sizeof(DirectoryEntry));
    file_system.write(reinterpret_cast<const char *>(&root_dir_parent), sizeof(DirectoryEntry));

    // Set the working directory to root
    working_directory = WorkingDirectory {
            meta_data.data_start_address,
            "/",
            get_directory_entries(meta_data.data_start_address)
    };

    std::cout << "OK" << std::endl;
    return true;
}

bool PseudoFS::defrag(const std::vector<std::string> &args) {
    return false;
}
