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

class ImageFilter {
private:
    CImg<unsigned char> image;
    int roundingFactor = 8;
    int topColorLimit = 8;
    int paletteMaxIndex;
    std::vector<int> colorPalette;

    void getColorPalette(void) {
        // If there are errors with the image, catch them here
        if (height == 0 || width == 0) {
            return;
        }

        /* Load all the colors into an unordered map to count hue frequency. Key is
         * minimized hex, value is frequency. */
        std::unordered_map<int, int> colorMap;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                colorMap[((image(x, y, 0) * 65536) + (image(x, y, 1) * 256) + image(x, y, 2)) / roundingFactor]++;
            }
        }

        /* Find the top [topColorLimit] number of colors by frequency. By completion, topColors has
         * all the most used hues sorted by highest to least frequency. */
        std::multimap<int, int, std::greater<int>> topColors;
        for (auto colorPair : colorMap) {
            if (topColors.size() < topColorLimit || colorPair.second > topColors.rbegin()->first) {
                topColors.insert(std::pair<int, int>(colorPair.second, colorPair.first));
            }
            if (topColors.size() > topColorLimit) {
                topColors.erase(topColors.rbegin()->first);
            }
        }

        // Add the colors to a vector that can be used as the final color palette.
        for (auto colorRank : topColors) {
            // Exclude black, which is used for drawing lines.
            if (colorRank.second != 0) {
                colorPalette.push_back(colorRank.second * roundingFactor);
            }
        }

        // Sort the color values (currently sorted by frequency) by hue, ascending.
        std::sort(colorPalette.begin(), colorPalette.end());
        paletteMaxIndex = colorPalette.size() - 1;
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

    std::vector<int> getPaletteHue(int originalColor, int minIndex, int maxIndex) {
        if (originalColor <= colorPalette[minIndex]) {
            return formatPixelColors(colorPalette[minIndex]);
        }
        if (originalColor >= colorPalette[maxIndex]) {
            return formatPixelColors(colorPalette[maxIndex]);
        }
        if (maxIndex == minIndex + 1) {
            return formatPixelColors(colorPalette[maxIndex]);
        }
        int midIndex = (maxIndex + minIndex) / 2;
        if (originalColor == colorPalette[midIndex]) {
            return formatPixelColors(colorPalette[midIndex]);
        }
        if (originalColor < colorPalette[midIndex]) {
            return getPaletteHue(originalColor, minIndex + 1, midIndex - 1);
        } else {
            return getPaletteHue(originalColor, midIndex + 1, maxIndex - 1);
        }
    }

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
    void applyFilter(void) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                std::vector<int> newColors = getPaletteHue(((image(x, y, 0) * 65536) + (image(x, y, 1) * 256) + image(x, y, 2)), 0, paletteMaxIndex);
                image(x, y, 0) = newColors[0];
                image(x, y, 1) = newColors[1];
                image(x, y, 2) = newColors[2];
            }
        }
    }
};

int main() {
    std::string uri = "targetImage2.jpeg";
    ImageFilter filteredImage(uri.c_str());

    if (!filteredImage.isValid()) {
        std::cout << "ERROR: Palette size less than 2.";
        return 1;
    }

    // filteredImage.printPalette();

    filteredImage.applyFilter();
    filteredImage.saveImageFile();

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