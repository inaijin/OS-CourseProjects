#ifndef _FILTERS_HPP_
#define _FILTERS_HPP_

#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <pthread.h>
#include <algorithm>

using namespace std;

typedef struct Pixel{
    int red;
    int green;
    int blue;
    Pixel(int r, int g, int b){
        red = max(0, min(255,r));
        green = max(0, min(255,g));
        blue = max(0, min(255,b));
    }
    Pixel(){
        red = 0;
        green = 0;
        blue = 0;
    }
} Pixel;

struct ParallelArgs{
    int col;
    int cols;
    int colS;
    int row;
    int rows;
    int rowS;
    int count;
    int extra;
    int threadcount;
    vector<vector<Pixel>>* image;
    vector<vector<Pixel>>* temp;
    char* fileBuffer;
};

void flip(vector<vector<Pixel>>& pixels, int row, int col, int threadCount);
void kernel(vector<vector<Pixel>>& pixels, int row, int col, int threadCount);
void purpleHaze(vector<vector<Pixel>>& pixels, int rows, int cols, int threadCount);
void lining(vector<vector<Pixel>>& pixels, int rows, int cols, int threadCount);
void applyFilters(vector<vector<Pixel>>& pixels, int rows, int cols);

#endif