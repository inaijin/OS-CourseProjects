#include "Filters.hpp"

int Kernel[3][3] = {
        {1, 2, 1},
        {2, 4, 2},
        {1, 2, 1}
};

void* flipPartial(void* arguments) {

    struct ParallelArgs *args;
    args = (struct ParallelArgs *) arguments;

    for (int roww = 0; roww < args->row; ++roww) {
        for (int coll = args->colS; coll < args->col; ++coll) {
            Pixel temp = (*args->image)[roww][coll];
            (*args->image)[roww][coll] = (*args->image)[args->rows - roww -1][coll];
            (*args->image)[args->rows - roww -1][coll] = temp;
        }
    }

    return nullptr;

}

void flip(vector<vector<Pixel>>& pixels, int row, int col, int threadCount) {

    pthread_t threads[threadCount];
    ParallelArgs args[threadCount];

    int ratio = col / threadCount;

    for (int i = 0; i < threadCount; i++) {
        args[i].image = &pixels;
        args[i].rows = row;
        args[i].row = row / 2;
        args[i].colS = ratio * i;
        args[i].col = (i == threadCount - 1) ? col : ratio * (i + 1);

        pthread_create(&threads[i], nullptr, flipPartial, &args[i]);
    }

    for (int i = 0; i < threadCount; i++) {
        pthread_join(threads[i], nullptr);
    }

}


Pixel multiply(vector<vector<Pixel>>& image, int row, int col){

    int r=0, g=0, b=0;
    for(int i=-1; i<=1; i++){
        for(int j=-1; j<=1; j++){
            r += image[row+i][col+j].red * Kernel[i+1][j+1];
            g += image[row+i][col+j].green * Kernel[i+1][j+1];
            b += image[row+i][col+j].blue * Kernel[i+1][j+1];
        }
    }

    r = r / 16;
    g = g / 16;
    b = b / 16;

    r = min(255, r);
    g = min(255, g);
    b = min(255, b);

    return {r, g, b};

}

void* kernelPartial(void* arguments) {

    struct ParallelArgs *args;
    args = (struct ParallelArgs *) arguments;

    for(int i = args->rowS; i < args->row; i++) {
        if(i == 0) continue;
        for(int j = 1; j < args->cols - 1; j++) {
            (*args->temp)[i][j] = multiply(*args->image, i, j);
        }
    }

    return nullptr;

}

void kernel(vector<vector<Pixel>>& pixels, int rows, int cols, int threadCount) {

    vector<vector<Pixel>> temp(rows, vector<Pixel>(cols));

    pthread_t threads[threadCount];
    ParallelArgs args[threadCount];

    int ratio = rows / threadCount;

    for (int i = 0; i < threadCount; i++) {
        args[i].temp = &temp;
        args[i].image = &pixels;
        args[i].cols = cols;
        args[i].rowS = (i * ratio == 0) ? 1 : (i * ratio);
        args[i].row = (i == threadCount - 1) ? rows - 1 : ratio * (i + 1);;

        pthread_create(&threads[i], nullptr, kernelPartial, &args[i]);
    }

    for (int i = 0; i < threadCount; i++) {
        pthread_join(threads[i], nullptr);
    }

    pixels = temp;

}

void* purpleHazePartial(void* arguments) {

    struct ParallelArgs *args;
    args = (struct ParallelArgs *) arguments;

    for (int i = 0; i < args->rows; ++i) {
        for (int j = args->colS; j < args->col; ++j) {
            Pixel temp = (*args->image)[i][j];
            (*args->image)[i][j].red = (0.5 * temp.red) + (0.3 * temp.green)  + (0.5 * temp.blue);
            (*args->image)[i][j].red = min(255, (*args->image)[i][j].red);
            (*args->image)[i][j].green = (0.16 * temp.red) + (0.5 * temp.green)  + (0.16 * temp.blue);
            (*args->image)[i][j].green = min(255, (*args->image)[i][j].green);
            (*args->image)[i][j].blue = (0.6 * temp.red) + (0.2 * temp.green)  + (0.8 * temp.blue);
            (*args->image)[i][j].blue = min(255, (*args->image)[i][j].blue);
        }
    }

    return nullptr;

}

void purpleHaze(vector<vector<Pixel>>& pixels, int rows, int cols, int threadCount){

    pthread_t threads[threadCount];
    ParallelArgs args[threadCount];

    int ratio = cols / threadCount;

    for (int i = 0; i < threadCount; i++) {
        args[i].image = &pixels;
        args[i].rows = rows;
        args[i].colS = ratio * i;
        args[i].col = (i == threadCount - 1) ? cols : ratio * (i + 1);

        pthread_create(&threads[i], nullptr, purpleHazePartial, &args[i]);
    }

    for (int i = 0; i < threadCount; i++) {
        pthread_join(threads[i], nullptr);
    }

}

void drawLine(vector<vector<Pixel>>& pixels, int p1X, int p1Y, int p2X, int p2Y, int rows, int cols) {

    int dx = p2X - p1X;
    int dy = p2Y - p1Y;
    int steps = max(abs(dx), abs(dy));

    float xinc = dx / static_cast<float>(steps);
    float yinc = dy / static_cast<float>(steps);

    float x = p1X;
    float y = p1Y;

    for (int i = 0; i <= steps; ++i) {

        int roundX = static_cast<int>(round(x));
        int roundY = static_cast<int>(round(y));

        if (roundY < cols - 1 && roundX < rows - 1) {
            pixels[roundX][roundY].red = 255;
            pixels[roundX][roundY].green = 255;
            pixels[roundX][roundY].blue = 255;
        }

        x += xinc;
        y += yinc;
    }

}

void* liningPartial(void* arguments) {

    struct ParallelArgs *args;
    args = (struct ParallelArgs *) arguments;

    drawLine(*args->image, args->row, args->col, args->rows, args->cols, args->rowS, args->colS);

    return nullptr;

}

void lining(vector<vector<Pixel>>& pixels, int rows, int cols, int threadCount){

    vector<int> x = {rows / 2, 0, rows - 1, 0, rows / 2, rows - 1};
    vector<int> y = {0, cols / 2, 0, cols - 1, cols - 1, cols / 2};

    pthread_t threads[threadCount];
    ParallelArgs args[threadCount];

    for (int i = 0; i < threadCount; i++) {
        args[i].image = &pixels;
        args[i].row = x[2 * i];
        args[i].rows = x[2 * i + 1];
        args[i].cols = y[2 * i + 1];
        args[i].col = y[2 * i];
        args[i].colS = cols;
        args[i].rowS = rows;

        pthread_create(&threads[i], nullptr, liningPartial, &args[i]);
    }

    for (int i = 0; i < threadCount; i++) {
        pthread_join(threads[i], nullptr);
    }

}

void applyFilters(vector<vector<Pixel>>& pixels, int rows, int cols) {

    auto startReadFlip = chrono::high_resolution_clock::now();
    flip(pixels, rows, cols, 8);
    auto endReadFlip = chrono::high_resolution_clock::now();
    chrono::duration<double> durationReadFlip = endReadFlip - startReadFlip;
    cout << "Flip: " << fixed << setprecision(3) << durationReadFlip.count() * 1000.0 << " ms" << endl;

    auto startReadBlur = chrono::high_resolution_clock::now();
    kernel(pixels, rows, cols, 32);
    auto endReadBlur = chrono::high_resolution_clock::now();
    chrono::duration<double> durationReadBlur = endReadBlur - startReadBlur;
    cout << "Blur: " << fixed << setprecision(3) << durationReadBlur.count() * 1000.0 << " ms" << endl;

    auto startReadPurple = chrono::high_resolution_clock::now();
    purpleHaze(pixels, rows, cols, 16);
    auto endReadPurple = chrono::high_resolution_clock::now();
    chrono::duration<double> durationReadPurple = endReadPurple - startReadPurple;
    cout << "Purple: " << fixed << setprecision(3) << durationReadPurple.count() * 1000.0 << " ms" << endl;

    auto startReadLine = chrono::high_resolution_clock::now();
    lining(pixels, rows, cols, 3);
    auto endReadLine = chrono::high_resolution_clock::now();
    chrono::duration<double> durationReadLine = endReadLine - startReadLine;
    cout << "Lines: " << fixed << setprecision(3) << durationReadLine.count() * 1000.0 << " ms" << endl;

}