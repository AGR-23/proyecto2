#include "versionedFile.h"

versionedFile::versionedFile() : isOpen(false), selectedFile("") {}

PerformanceTracker performanceTracker;

const int MAX_VERSIONS = 5; // maximum number of versions to keep

// Garbage collector to remove old versions
void versionedFile::garbageCollector()
{
    if (versions.size() > MAX_VERSIONS)
    {
        std::string oldestVersion = versions.front().filename;
        if (std::filesystem::exists(oldestVersion))
        {
            std::filesystem::remove(oldestVersion);
            std::cout << "Deleted old version: " << oldestVersion << std::endl;
        }
        versions.erase(versions.begin());
    }
}

// Allows the user to create a file with a custom name.
bool versionedFile::create()
{
    performanceTracker.start();
    std::string userFilename;

    // Ask the user for the filename
    std::cout << "Enter the name of the file (without extension): ";
    std::cin >> userFilename;

    // Construct the initial versioned filename (_v0.txt)
    filename = userFilename;
    std::string initialVersion = filename + "_v0.txt";

    // Check if the file already exists
    if (std::filesystem::exists(initialVersion))
    {
        std::cout << "Error: A file with this name already exists.\n";
        return false;
    }

    // Create the new file
    std::ofstream file(initialVersion);

    if (!file)
    {
        std::cout << "Error: Could not create file.\n";
        return false;
    }

    file.close();
    versions.push_back({initialVersion, 0});
    std::cout << "File created: " << initialVersion << std::endl;
    performanceTracker.report("create");
    return true;
}

// Allows the user to open a versioned file by choosing a prefix and a version.
bool versionedFile::open()
{
    performanceTracker.start();
    std::unordered_map<std::string, std::vector<std::string>> filePrefixes;

    // Scan the current directory for versioned files (*_vN.txt)
    for (const auto &entry : std::filesystem::directory_iterator("."))
    {
        std::string name = entry.path().filename().string();
        size_t pos = name.find("_v");
        if (pos != std::string::npos)
        {
            std::string prefix = name.substr(0, pos);
            filePrefixes[prefix].push_back(name);
        }
    }

    // If no versioned files are found, exit
    if (filePrefixes.empty())
    {
        std::cout << "No versioned files found.\n";
        return false;
    }

    // Allow the user to select a file prefix if multiple exist
    std::cout << "Available file prefixes:\n";

    int index = 1;
    std::vector<std::string> prefixes;
    for (const auto &entry : filePrefixes)
    {
        std::cout << index << ". " << entry.first << std::endl;
        prefixes.push_back(entry.first);
        index++;
    }

    int choice;
    std::cout << "Enter the number of the prefix you want to open: ";
    std::cin >> choice;

    if (choice < 1 || choice > prefixes.size())
    {
        std::cout << "Invalid selection.\n";
        return false;
    }

    filename = prefixes[choice - 1];

    // Show available versions of the selected prefix
    std::cout << "Available versions for " << filename << ":\n";

    index = 1;
    for (const auto &file : filePrefixes[filename])
    {
        std::cout << index << ". " << file << std::endl;
        index++;
    }

    std::cout << "Enter the number of the version you want to open: ";
    std::cin >> choice;

    if (choice < 1 || choice > filePrefixes[filename].size())
    {
        std::cout << "Invalid selection.\n";
        return false;
    }

    // Set the selected file as the active file
    selectedFile = filePrefixes[filename][choice - 1];
    isOpen = true;
    std::cout << "Opened file: " << selectedFile << std::endl;
    performanceTracker.report("Open");
    return true;
}

// Reads from the currently opened file.
// Reads and reconstructs the latest version by applying all deltas
// Reads from the currently opened file.
std::string versionedFile::read()
{
    performanceTracker.start();
    if (!isOpen)
    {
        return "No file is open. Please open a file first.";
    }

    std::ifstream file(selectedFile);
    if (!file)
        return "Error reading file.";
    if (file.peek() == std::ifstream::traits_type::eof())
        return "File is empty.";

    performanceTracker.report("Read");
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

// Writes new data to a new version of the currently opened file.
// Saves only the changes (deltas) instead of copying the whole file
void versionedFile::write(const std::string &data)
{
    performanceTracker.start();
    if (!isOpen)
    {
        std::cout << "No file is open. Please open a file first.\n";
        return;
    }

    // Extract prefix from the selected file name
    size_t pos = selectedFile.find("_v");
    if (pos == std::string::npos)
    {
        std::cout << "Error: Invalid file name format.\n";
        return;
    }

    std::string prefix = selectedFile.substr(0, pos);
    int newVersionNumber = versions.size();
    std::string newVersion = prefix + "_v" + std::to_string(newVersionNumber) + ".txt";

    // Create and write to the new versioned file
    std::ofstream file(newVersion);
    if (!file)
    {
        std::cout << "Error creating file.\n";
        return;
    }

    file << data;
    file.close();
    versions.push_back({newVersion, data.size()});

    std::cout << "New version created: " << newVersion << std::endl;

    garbageCollector(); // Remove old versions
    performanceTracker.report("Write");
}

// Closes the currently opened file.
void versionedFile::close()
{
    performanceTracker.start();
    if (!isOpen)
    {
        std::cout << "No file is open.\n";
        return;
    }

    std::cout << "Closing file: " << selectedFile << std::endl;
    isOpen = false;
    performanceTracker.report("Close");
    selectedFile = "";
}