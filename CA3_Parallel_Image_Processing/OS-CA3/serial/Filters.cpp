#include "Filters.hpp"

int Kernel[3][3] = {
        {1, 2, 1},
        {2, 4, 2},
        {1, 2, 1}
};

void flip(vector<vector<Pixel>>& pixels, int row, int col) {

    for (int roww = 0; roww < row / 2; ++roww) {
        for (int coll = 0; coll < col; ++coll) {
            swap(pixels[roww][coll], pixels[row - 1 - roww][coll]);
        }
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

void kernel(vector<vector<Pixel>>& pixels, int rows, int cols) {

    vector<vector<Pixel>> temp(rows, vector<Pixel>(cols));

    for(int t=0; t<rows; t++) {
        temp[t][0] = pixels[t][0];
        temp[t][cols-1] = pixels[t][cols-1];
    }

    for(int t=0; t<cols; t++) {
        temp[0][t] = pixels[0][t];
        temp[rows-1][t] = pixels[rows-1][t];
    }

    for (int i = 1; i < rows - 1; i++)
        for (int j = 1; j < cols - 1; j++)
            temp[i][j] = multiply(pixels, i, j);

    pixels = temp;

}

void purpleHaze(vector<vector<Pixel>>& pixels, int rows, int cols){

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            Pixel temp = pixels[i][j];
            pixels[i][j].red = (0.5 * temp.red) + (0.3 * temp.green)  + (0.5 * temp.blue);
            pixels[i][j].red = min(255, pixels[i][j].red);
            pixels[i][j].green = (0.16 * temp.red) + (0.5 * temp.green)  + (0.16 * temp.blue);
            pixels[i][j].green = min(255, pixels[i][j].green);
            pixels[i][j].blue = (0.6 * temp.red) + (0.2 * temp.green)  + (0.8 * temp.blue);
            pixels[i][j].blue = min(255, pixels[i][j].blue);
        }
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

void lining(vector<vector<Pixel>>& pixels, int rows, int cols){

    int topX = rows / 2, topY = 0;
    int downX = rows / 2, downY = cols - 1;
    int rightX = rows - 1, rightY = cols / 2;
    int leftX = 0, leftY = cols / 2;

    drawLine(pixels, topX, topY, leftX, leftY, rows, cols);
    drawLine(pixels, rows - 1, 0, 0, cols - 1, rows, cols);
    drawLine(pixels, downX, downY, rightX, rightY, rows, cols);

}

void applyFilters(vector<vector<Pixel>>& pixels, int rows, int cols) {

    auto startReadFlip = chrono::high_resolution_clock::now();
    flip(pixels, rows, cols);
    auto endReadFlip = chrono::high_resolution_clock::now();
    chrono::duration<double> durationReadFlip = endReadFlip - startReadFlip;
    cout << "Flip: " << fixed << setprecision(3) << durationReadFlip.count() * 1000.0 << " ms" << endl;

    auto startReadBlur = chrono::high_resolution_clock::now();
    kernel(pixels, rows, cols);
    auto endReadBlur = chrono::high_resolution_clock::now();
    chrono::duration<double> durationReadBlur = endReadBlur - startReadBlur;
    cout << "Blur: " << fixed << setprecision(3) << durationReadBlur.count() * 1000.0 << " ms" << endl;

    auto startReadPurple = chrono::high_resolution_clock::now();
    purpleHaze(pixels, rows, cols);
    auto endReadPurple = chrono::high_resolution_clock::now();
    chrono::duration<double> durationReadPurple = endReadPurple - startReadPurple;
    cout << "Purple: " << fixed << setprecision(3) << durationReadPurple.count() * 1000.0 << " ms" << endl;

    auto startReadLine = chrono::high_resolution_clock::now();
    lining(pixels, rows, cols);
    auto endReadLine = chrono::high_resolution_clock::now();
    chrono::duration<double> durationReadLine = endReadLine - startReadLine;
    cout << "Lines: " << fixed << setprecision(3) << durationReadLine.count() * 1000.0 << " ms" << endl;

}