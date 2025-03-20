#include "versionedFile.h"

int main()
{
    versionedFile file;
    int option;
    std::string data;

    while (true) {
        std::cout << "\n===================================\n";
        std::cout << "            MAIN MENU              \n";
        std::cout << "===================================\n";
        std::cout << "1. Create File\n";
        std::cout << "2. Open File\n";
        std::cout << "3. Read File\n";
        std::cout << "4. Write File\n";
        std::cout << "5. Close File\n";
        std::cout << "6. Exit\n";
        std::cout << "-----------------------------------\n";
        std::cout << "Choose an option: ";
        std::cin >> option;
        std::cout << "\n";

        switch (option) {
            case 1:
                std::cout << ">> Creating a new file...\n";
                file.create();
                break;

            case 2:
                std::cout << ">> Opening an existing file...\n";
                file.open();
                break;

            case 3:
                std::cout << ">> Reading from the opened file...\n";
                std::cout << file.read() << "\n";
                break;

            case 4:
                std::cout << ">> Enter data to write: ";
                std::cin.ignore();
                std::getline(std::cin, data);
                file.write(data);
                std::cout << ">> Data written successfully.\n";
                break;

            case 5:
                std::cout << ">> Closing the file...\n";
                file.close();
                std::cout << ">> File closed successfully.\n";
                break;

            case 6:
                std::cout << ">> Goodbye\n";
                return 0;

            default:
                std::cout << ">> Invalid option. Please try again.\n";
        }

        std::cout << "\n===================================\n";
    }
}