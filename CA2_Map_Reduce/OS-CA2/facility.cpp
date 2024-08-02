#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <unistd.h>
#include <filesystem>
#include <sys/wait.h>
#include <cstring>
#include <fcntl.h>
#include <unordered_map>

using namespace std;

void logg(const string& message) {
    ofstream outFile("logg.txt", ios::app);
    outFile << message;
    outFile.close();
}

struct UsageData {
    int year, month, day;
    vector<int> values;

    UsageData(int y, int m, int d, const vector<int>& v) : year(y), month(m), day(d), values(v) {}
};

std::unordered_map<std::string, std::vector<int>> parseCSV(const std::string& filename) {
    std::unordered_map<std::string, std::vector<int>> dataMap;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return dataMap;
    }

    std::getline(file, line);

    while (std::getline(file, line)) {
        std::istringstream lineStream(line);
        std::string year, month, water, gas, electricity;

        std::getline(lineStream, year, ',');
        std::getline(lineStream, month, ',');
        std::getline(lineStream, water, ',');
        std::getline(lineStream, gas, ',');
        std::getline(lineStream, electricity, ',');

        std::string key = year + "-" + month;
        dataMap[key] = {std::stoi(water), std::stoi(gas), std::stoi(electricity)};
    }

    file.close();
    return dataMap;
}

void processResultString(const std::string& resultStr, const std::unordered_map<std::string, std::vector<int>>& dataMap) {
    std::istringstream ss(resultStr);
    std::string line;

    while (std::getline(ss, line)) {
        std::istringstream lineStream(line);
        std::string year, month, day;
        int water1, water2, gas1, gas2, electricity1, electricity2;

        lineStream >> year >> month >> day >> water1 >> water2 >> gas1 >> gas2 >> electricity1 >> electricity2;

        std::string key = year + "-" + month;

        if (dataMap.find(key) != dataMap.end()) {
            const std::vector<int>& values = dataMap.at(key);
            int waterResult = (water1 + water2) * values[0];
            int gasResult = (gas1 + gas2) * values[1];
            int electricityResult = (electricity1 + electricity2) * values[2];

            std::cout << "Date: " << year << "-" << month << "-" << day << std::endl;
            std::cout << "Water Result: " << waterResult << std::endl;
            std::cout << "Gas Result: " << gasResult << std::endl;
            std::cout << "Electricity Result: " << electricityResult << std::endl;
            std::cout << std::endl;
        }
    }
}

vector<UsageData> deserializeUsageData(const string& str) {
    istringstream iss(str);
    vector<UsageData> result;
    int year, month, day;
    while (iss >> year >> month >> day) {
        vector<int> values;
        int value;
        while (iss >> value) {
            values.push_back(value);
        }
        result.emplace_back(year, month, day, values);
    }
    return result;
}

void processUsageData(const string& result, const string& houseName, const string& type) {
    vector<UsageData> datas = deserializeUsageData(result);
    cout << "House: " << houseName << " Type: " << type << endl;
    for (const auto& entry : datas) {
        cout << entry.year << '-' << entry.month << '-' << entry.day << " : ";
        for (const auto& value : entry.values) {
            cout << value << ' ';
        }
        cout << endl;
    }
}

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <house_names...>" << endl;
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; i++) {
        string houseName = argv[i];
        string pipeName = "pipe_" + houseName;

        int fifo_fd = open(pipeName.c_str(), O_RDONLY);
        if (fifo_fd == -1) {
            cerr << "Error opening named pipe for reading in facility" << endl;
            exit(EXIT_FAILURE);
        }

        char result[11000];
        ssize_t bytesRead = read(fifo_fd, result, sizeof(result) - 1);
        if (bytesRead == -1) {
            cerr << "Error reading from named pipe in facility" << endl;
            exit(EXIT_FAILURE);
        }
        result[bytesRead] = '\0';
        close(fifo_fd);

        string resultStr(result);

        string csvFilename = "buildings/bills.csv";

        std::unordered_map<std::string, std::vector<int>> dataMap = parseCSV(csvFilename);
        processResultString(resultStr, dataMap);
    }

    return 0;
}
