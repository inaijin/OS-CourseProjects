#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>

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

string serializeUsageData(const vector<UsageData>& data) {
    ostringstream oss;
    for (const auto& entry : data) {
        oss << entry.year << ' ' << entry.month << ' ' << entry.day << ' ';
        for (const auto& value : entry.values) {
            oss << value << ' ';
        }
        oss << '\n';
    }
    return oss.str();
}

string processCSVFile(const string& filePath, const string& type, const string& houseName) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filePath << endl;
        exit(EXIT_FAILURE);
    }

    vector<UsageData> data;

    string header;
    getline(file, header);

    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        char comma;

        int year, month, day;
        vector<int> values;

        iss >> year >> comma >> month >> comma >> day;
        while (iss >> comma) {
            int value;
            if (iss >> value) {
                values.push_back(value);
            }
        }

        data.emplace_back(year, month, day, values);
    }

    int ans = 0;
    for (const auto& entry : data) {
        for (int value : entry.values) {
            ans += value;
        }
    }

    logg("Result for " + type + " in " + houseName + ": " + to_string(ans) + "\n");

    return serializeUsageData(data);
}

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <house_name> <contor_type>" << endl;
        return EXIT_FAILURE;
    }

    string houseName = argv[1];
    string type = argv[2];
    string filePath = "buildings/" + houseName + "/" + type + ".csv";

    string result = processCSVFile(filePath, type, houseName);
    string decoded = result + "/" + houseName + "." + type;

    ssize_t bytesWritten = write(STDOUT_FILENO, decoded.c_str(), decoded.size());
    if (bytesWritten == -1) {
        cerr << "Error writing to pipe" << endl;
        exit(EXIT_FAILURE);
    }

    close(STDOUT_FILENO);

    return 0;
}
