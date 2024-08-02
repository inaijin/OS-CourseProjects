#ifndef _FILTERS_HPP_
#define _FILTERS_HPP_

#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <math.h>

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

void flip(vector<vector<Pixel>>& pixels, int row, int col);
void kernel(vector<vector<Pixel>>& pixels, int row, int col);
void purpleHaze(vector<vector<Pixel>>& pixels, int rows, int cols);
void lining(vector<vector<Pixel>>& pixels, int rows, int cols);
void applyFilters(vector<vector<Pixel>>& pixels, int rows, int cols);

#endif