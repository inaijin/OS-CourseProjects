#include <iostream>
#include <filesystem>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fstream>
#include <string>
#include <cstring>
#include <fcntl.h>
#include <vector>

using namespace std;
namespace fs = filesystem;

int empty_file(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }
    if (fclose(file) != 0) {
        perror("Error closing file");
        return 2;
    }
    return 0;
}

void logg(const string& message) {
    ofstream outFile("logg.txt", ios::app);
    outFile << message;
    outFile.close();
}

void createNamedPipe(const string& houseName) {
    string pipeName = "pipe_" + houseName;

    if (access(pipeName.c_str(), F_OK) == 0) {
        if (unlink(pipeName.c_str()) != 0) {
            cerr << "Error removing existing named pipe for " << houseName << ": " << strerror(errno) << endl;
            exit(EXIT_FAILURE);
        }
    }

    if (mkfifo(pipeName.c_str(), 0666) == -1) {
        cerr << "Error creating named pipe for " << houseName << endl;
        exit(EXIT_FAILURE);
    }
}

pid_t processFacility(const vector<string>& fifos) {
    pid_t pid = fork();

    if (pid == -1) {
        cerr << "Error forking process for facility" << endl;
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        logg("Facility Registered\n");

        vector<const char*> args;
        args.push_back("./facility.out");
        for (const auto& fifo : fifos) {
            args.push_back(fifo.c_str());
        }
        args.push_back(nullptr);

        execvp(args[0], const_cast<char* const*>(args.data()));
        exit(EXIT_SUCCESS);
    } else {
        return pid;
    }
}

void processBuilding(const fs::directory_entry& entry) {
    string buildingName = entry.path().filename().string();
    createNamedPipe(buildingName);

    pid_t pid = fork();
    if (pid == -1) {
        cerr << "Error forking process for building " << buildingName << endl;
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        logg("Building " + buildingName + " Registered\n");
        char buildingNameCharArray[buildingName.size() + 1];
        strcpy(buildingNameCharArray, buildingName.c_str());
        const char* args[] = {"./houses.out", buildingNameCharArray, nullptr};
        execvp(args[0], const_cast<char* const*>(args));
        exit(EXIT_SUCCESS);
    }
}

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <buildings_directory>" << endl;
        return EXIT_FAILURE;
    }

    const char *log_filename = "logg.txt";
    empty_file(log_filename);

    vector<string> FIFOs;
    const string buildingsDirectory = argv[1];

    for (const auto& entry : fs::directory_iterator(buildingsDirectory)) {
        if (entry.is_directory()) {
            string buildingName = entry.path().filename().string();
            FIFOs.push_back(buildingName);
        }
    }

    pid_t facilityPID = processFacility(FIFOs);

    for (const auto& entry : fs::directory_iterator(buildingsDirectory)) {
        if (entry.is_directory()) {
            processBuilding(entry);
        }
    }

    int status;
    waitpid(facilityPID, &status, 0);

    return 0;
}
