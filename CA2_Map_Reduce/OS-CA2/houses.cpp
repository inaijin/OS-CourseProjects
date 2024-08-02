#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <fcntl.h>

using namespace std;

void logg(const string& message) {
    ofstream outFile("logg.txt", ios::app);
    outFile << message;
    outFile.close();
}

void processHouse(const string& houseName) {
    string types[] = {"Electricity", "Gas", "Water"};
    string combinedResults;

    for (const string& type : types) {
        string filePath = "buildings/" + houseName + "/" + type + ".csv";

        int pipe_fd[2];
        if (pipe(pipe_fd) == -1) {
            cerr << "Error creating pipe for " << type << " data in " << houseName << endl;
            exit(EXIT_FAILURE);
        }

        pid_t pid = fork();
        if (pid == -1) {
            cerr << "Error forking process for " << type << " data in " << houseName << endl;
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            close(pipe_fd[0]);
            logg(type + " for house " + houseName + " is being calculated\n");

            dup2(pipe_fd[1], STDOUT_FILENO);
            close(pipe_fd[1]);

            execl("./contors.out", "./contors.out", houseName.c_str(), type.c_str(), nullptr);
            exit(EXIT_SUCCESS);
        } else {
            close(pipe_fd[1]);

            char result[11000];
            ssize_t bytesRead = read(pipe_fd[0], result, sizeof(result) - 1);
            if (bytesRead == -1) {
                cerr << "Error reading from pipe in house " << houseName << endl;
                exit(EXIT_FAILURE);
            }
            result[bytesRead] = '\0';
            close(pipe_fd[0]);

            combinedResults += string(result) + "\n";

            int status;
            waitpid(pid, &status, 0);
        }
    }

    int fifo_fd = open(("pipe_" + houseName).c_str(), O_WRONLY);
    if (fifo_fd == -1) {
        cerr << "Error opening named pipe for writing in " << houseName << endl;
        exit(EXIT_FAILURE);
    }

    if (write(fifo_fd, combinedResults.c_str(), combinedResults.size()) == -1) {
        cerr << "Error writing to named pipe in house " << houseName << endl;
        exit(EXIT_FAILURE);
    }
    close(fifo_fd);
}

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <house_name>" << endl;
        return EXIT_FAILURE;
    }

    const string houseName = argv[1];
    processHouse(houseName);

    return 0;
}
