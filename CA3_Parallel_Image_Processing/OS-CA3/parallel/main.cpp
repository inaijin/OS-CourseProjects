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

void* getPixelsPartial(void* arguments) {

    struct ParallelArgs* args;
    args = (struct ParallelArgs*)arguments;
    int extra = args->extra;
    int count = args->count;
    int cols = args->cols;
    int row = args->row;
    int rowS = args->rowS;
    vector<vector<Pixel>>* image = args->image;

    for (int i = row; i < rowS; i++) {
        count -= extra;
        for (int j = cols - 1; j >= 0; j--) {
            for (int k = 0; k < 3; k++) {
                count--;
                auto c = (unsigned char)args->fileBuffer[count];
                switch (k) {
                case 0:
                    (*image)[i][j].red = c;
                    break;
                case 1:
                    (*image)[i][j].green = c;
                    break;
                case 2:
                    (*image)[i][j].blue = c;
                    break;
                }
            }
        }
    }

    return nullptr;

}

vector<vector<Pixel>> getPixelsFromBMP24(int end, int& rows, int& cols, char* fileReadBuffer, int threadCount) {

    int index = end;
    int extra = cols % 4;
    vector<vector<Pixel>> picture(rows, vector<Pixel>(cols));
    int padding = (3 * cols + extra) * rows;
    while (padding % threadCount != 0)
        threadCount--;

    int offset = end - padding, portion = rows / threadCount;
    pthread_t threads[threadCount];
    ParallelArgs args[threadCount];
    int st = 0;

    for (int i = 0; i < threadCount; i++) {
        args[i].col = 0;
        args[i].cols = cols;
        args[i].rows = rows;
        args[i].row = st;
        st += portion;
        args[i].rowS = (i == threadCount - 1) ? rows : st;
        args[i].count = index;
        args[i].extra = extra;
        args[i].image = &picture;
        args[i].fileBuffer = fileReadBuffer;

        pthread_create(&threads[i], nullptr, getPixelsPartial, &args[i]);
        index -= (3 * cols + extra) * portion;
    }

    for (int i = 0; i < threadCount; i++) {
        pthread_join(threads[i], nullptr);
    }

    return picture;

}

void* writeOutBmp24Partial(void* arguments) {

    struct ParallelArgs* args;
    args = (struct ParallelArgs*)arguments;
    int extra = args->extra;
    int count = args->count;
    int cols = args->cols;
    int row = args->row;
    int rowS = args->rowS;
    vector<vector<Pixel>>* image = args->image;

    for (int i = row; i < rowS; i++) {
        count -= extra;
        for (int j = cols - 1; j >= 0; j--) {
            for (int k = 0; k < 3; k++) {
                count--;
                switch (k) {
                case 0:
                    args->fileBuffer[count] = char(image->at(i)[j].red);
                    break;
                case 1:
                    args->fileBuffer[count] = char(image->at(i)[j].green);
                    break;
                case 2:
                    args->fileBuffer[count] = char(image->at(i)[j].blue);
                    break;
                }
            }
        }
    }
    return nullptr;

}

void writeOutBmp24(char* fileBuffer, const char* nameOfFileToCreate, int bufferSize, int& rows, int& cols, vector<vector<Pixel>>& image, int threadCount) {

    std::ofstream write(nameOfFileToCreate);
    if (!write) {
        cout << "Failed to write " << nameOfFileToCreate << endl;
        return;
    }

    int index = bufferSize;
    int extra = cols % 4;
    int dx = (3 * cols + extra) * rows;
    while (dx % threadCount != 0)
        threadCount--;

    int offset = bufferSize - dx, portion = rows / threadCount;
    pthread_t threads[threadCount];
    ParallelArgs args[threadCount];
    int st = 0;

    for (int i = 0; i < threadCount; i++) {
        args[i].threadcount = threadCount;
        args[i].col = 0;
        args[i].cols = cols;
        args[i].rows = rows;
        args[i].row = st;
        st += portion;
        args[i].rowS = (i == threadCount - 1) ? rows : st;
        args[i].count = index;
        args[i].extra = extra;
        args[i].image = &image;
        args[i].fileBuffer = fileBuffer;

        pthread_create(&threads[i], nullptr, writeOutBmp24Partial, &args[i]);
        index -= (3 * cols + extra) * portion;
    }

    for (int i = 0; i < threadCount; i++) {
        pthread_join(threads[i], nullptr);
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
    pixels = getPixelsFromBMP24(bufferSize, rows, cols, fileBuffer, 32);
    auto endReadRead = chrono::high_resolution_clock::now();
    chrono::duration<double> durationReadRead = endReadRead - startReadRead;
    cout << "Read: " << fixed << setprecision(3) << durationReadRead.count() * 1000.0 << " ms" << endl;

    applyFilters(pixels, rows, cols);

    writeOutBmp24(fileBuffer, "output.bmp", bufferSize, rows, cols, pixels, 32);

    delete[] fileBuffer;

    auto endReadTotal = chrono::high_resolution_clock::now();
    chrono::duration<double> durationReadTotal = endReadTotal - startReadTotal;
    cout << "Execution: " << fixed << setprecision(3) << durationReadTotal.count() * 1000.0 << " ms" << endl;

    return 0;

}