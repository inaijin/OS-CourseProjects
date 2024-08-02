#include <fstream>
#include <iostream>
#include <vector>
#include "Filters.hpp"

typedef int LONG;
typedef unsigned short WORD;
typedef unsigned int DWORD;

#pragma pack(push, 1)
typedef struct tagBITMAPFILEHEADER {

    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;

} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {

    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;

} BITMAPINFOHEADER, *PBITMAPINFOHEADER;
#pragma pack(pop)

int rows;
int cols;

bool fillAndAllocate(char*& buffer, const char* fileName, int& rows, int& cols, int& bufferSize) {

    std::ifstream file(fileName);
    if (!file) {
        std::cout << "File" << fileName << " doesn't exist!" << std::endl;
        return false;
    }

    file.seekg(0, std::ios::end);
    std::streampos length = file.tellg();
    file.seekg(0, std::ios::beg);

    buffer = new char[length];
    file.read(&buffer[0], length);

    PBITMAPFILEHEADER file_header;
    PBITMAPINFOHEADER info_header;

    file_header = (PBITMAPFILEHEADER)(&buffer[0]);
    info_header = (PBITMAPINFOHEADER)(&buffer[0] + sizeof(BITMAPFILEHEADER));
    rows = info_header->biHeight;
    cols = info_header->biWidth;
    bufferSize = file_header->bfSize;
    return true;

}

vector<vector<Pixel>> getPixlesFromBMP24(int end, int& rows, int& cols, char *fileReadBuffer) {

    int count = 1;
    int extra = cols % 4;
    vector <vector<Pixel>> pixels(rows, vector<Pixel>(cols));
    for (int i = 0; i < rows; i++) {
        count += extra;
        for (int j = cols - 1; j >= 0; j--) {
            for (int k = 0; k < 3; k++) {
                unsigned char channel = fileReadBuffer[end-count];
                switch (k) {
                    case 0:
                        pixels[i][j].red = int(channel);
                        break;
                    case 1:
                        pixels[i][j].green = int(channel);
                        break;
                    case 2:
                        pixels[i][j].blue = int(channel);
                        break;
                }
                count++;
            }
        }
    }
    return pixels;

}

void writeOutBmp24(char *fileBuffer, const char *nameOfFileToCreate, int bufferSize, int& rows, int& cols, vector<vector<Pixel>>& pixels) {

    std::ofstream write(nameOfFileToCreate);
    if (!write) {
        cout << "Failed to write " << nameOfFileToCreate << endl;
        return;
    }
    int count = 1;
    int extra = cols % 4;
    for (int i = 0; i < rows; i++) {
        count += extra;
        for (int j = cols - 1; j >= 0; j--) {
            for (int k = 0; k < 3; k++) {
                switch (k) {
                    case 0:
                        fileBuffer[bufferSize - count] = char(pixels[i][j].red);
                        break;
                    case 1:
                        fileBuffer[bufferSize - count] = char(pixels[i][j].green);
                        break;
                    case 2:
                        fileBuffer[bufferSize - count] = char(pixels[i][j].blue);
                        break;
                }
                count++;
            }
        }
    }
    write.write(fileBuffer, bufferSize);

}

int main(int argc, char* argv[]) {

    auto startReadTotal = chrono::high_resolution_clock::now();

    char* fileBuffer;
    int bufferSize;

    if (!fillAndAllocate(fileBuffer, argv[1], rows, cols, bufferSize)) {
        cout << "File read error" << endl;
        return 1;
    }

    vector<vector<Pixel>> pixels;

    auto startReadRead = chrono::high_resolution_clock::now();
    pixels = getPixlesFromBMP24(bufferSize, rows, cols, fileBuffer);
    auto endReadRead = chrono::high_resolution_clock::now();
    chrono::duration<double> durationReadRead = endReadRead - startReadRead;
    cout << "Read: " << fixed << setprecision(3) << durationReadRead.count() * 1000.0 << " ms" << endl;

    applyFilters(pixels, rows, cols);

    writeOutBmp24(fileBuffer, "output.bmp", bufferSize, rows, cols, pixels);

    delete[] fileBuffer;

    auto endReadTotal = chrono::high_resolution_clock::now();
    chrono::duration<double> durationReadTotal = endReadTotal - startReadTotal;
    cout << "Execution: " << fixed << setprecision(3) << durationReadTotal.count() * 1000.0 << " ms" << endl;

    return 0;

}