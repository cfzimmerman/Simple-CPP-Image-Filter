#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>

#include "CImg.h"
#define cimg_use_png
#define cimg_use_jpeg
#define cimg_use_tiff
#define cimg_use_curl

using namespace cimg_library;

class RGB_Triple {
private:
    int red;
    int green;
    int blue;
    int frequency;

public:
    RGB_Triple(int r = 0, int g = 0, int b = 0, int fr = 0) : red(r), green(g), blue(b), frequency(fr){};
    void mergeValue(int r, int g, int b) {
        int safeFrequency = frequency < 8421500 ? frequency : 8421500;
        red = ((safeFrequency * red) + r) / (safeFrequency + 1);
        green = ((safeFrequency * green) + g) / (safeFrequency + 1);
        blue = ((safeFrequency * blue) + b) / (safeFrequency + 1);
        frequency++;
    }
};

class ImageFilter {
private:
    CImg<unsigned char> image;
    std::vector<int> colorPalette;
    int getLuminosityIndex(int intensity) { return (intensity > 128 ? 0 : 1); }
    void getColorPalette(void) {
        int r, g, b, insertIndex;
        // RGB
        RGB_Triple vectorDefault;
        std::vector<RGB_Triple> Rg = {vectorDefault, vectorDefault}, Rb = {vectorDefault, vectorDefault}, Gr = {vectorDefault, vectorDefault},
                                Gb = {vectorDefault, vectorDefault}, Br = {vectorDefault, vectorDefault}, Bg = {vectorDefault, vectorDefault};
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                r = image(x, y, 0);
                g = image(x, y, 1);
                b = image(x, y, 2);
                if (r >= g && r >= b) {
                    insertIndex = getLuminosityIndex(r);
                    if (g >= b) {
                        Rg[insertIndex].mergeValue(r, g, b);
                    } else {
                        Rb[insertIndex].mergeValue(r, g, b);
                    }
                } else if (g >= r && g >= b) {
                    insertIndex = getLuminosityIndex(g);
                    if (r >= b) {
                        Gr[insertIndex].mergeValue(r, g, b);
                    } else {
                        Gb[insertIndex].mergeValue(r, g, b);
                    }
                } else {
                    insertIndex = getLuminosityIndex(g);
                    if (r >= b) {
                        Br[insertIndex].mergeValue(r, g, b);
                    } else {
                        Bg[insertIndex].mergeValue(r, g, b);
                    }
                }
            }
        }
    }

    std::vector<int> formatPixelColors(int hexAsInt) {
        std::vector<int> result = {0, 0, 0};
        result[0] = hexAsInt >= 65536 ? (hexAsInt / 65536) : 0;
        hexAsInt -= result[0] * 65536;
        result[1] = hexAsInt >= 256 ? (hexAsInt / 256) : 0;
        hexAsInt -= result[1] * 256;
        result[2] = hexAsInt;
        return result;
    }

    /*
        std::vector<int> getPaletteHue(int originalColor, int minIndex, int maxIndex) {
        }
    */

public:
    int width;
    int height;
    ImageFilter(std::string uri) {
        CImg<unsigned char> newImg(uri.c_str());
        image = newImg;
        width = image.width();
        height = image.height();
        getColorPalette();
    }
    void printPalette(void) {
        for (int color : colorPalette) {
            std::cout << std::hex << color << std::endl;
        }
    }
    bool isValid(void) { return (colorPalette.size() > 2); }
    void saveImageFile(void) { image.save("output.jpeg"); }
    void applyFilter(void) {}
};

int main() {
    std::string uri = "targetImage.jpeg";
    ImageFilter filteredImage(uri.c_str());

    return 0;
}

/*
  int channels = img.spectrum();

  std::cout << "Dimensions: " << width << "x" << height << " " << channels <<
  "channels" << std::endl;

    for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {

      // start with trying to figure out what the type of img(x, y, z) is


      char hex[16];
      sprintf(hex, "#%02x%02x%02x", img(x, y, 0), img(x, y, 1), img(x, y, 2));
      std::cout << y << ", " << x << " " << hex << std::endl;
      // Grayscale
      int average = ((img(x, y, 0) + img(x, y, 1) + img(x, y, 2)) / 3);
      img(x, y, 0) = average;
      img(x, y, 1) = average;
      img(x, y, 2) = average;

    }
  }


  img.save("newlyCreated.jpeg");

*/

// clang++ main.cpp -o main -I/opt/X11/include -L/opt/X11/lib -lX11 -lpthread
// -std=c++11