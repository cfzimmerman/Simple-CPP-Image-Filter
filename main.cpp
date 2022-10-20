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
  int roundingFactor = 16;
  int topColorLimit = 16;
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
        colorMap[((image(x, y, 0) * 65536) + (image(x, y, 1) * 256) +
                  image(x, y, 2)) /
                 roundingFactor]++;
      }
    }

    /* Find the top [topColorLimit] number of colors by frequency. By
     * completion, topColors has all the most used hues sorted by highest to
     * least frequency. */
    std::multimap<int, int, std::greater<int>> topColors;
    for (auto colorPair : colorMap) {
      if (topColors.size() < topColorLimit ||
          colorPair.second > topColors.rbegin()->first) {
        topColors.insert(
            std::pair<int, int>(colorPair.second, colorPair.first));
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

  int getPaletteHue(int originalColor, int minIndex, int maxIndex) {
    if (originalColor <= colorPalette[minIndex]) {
      return colorPalette[minIndex];
    }
    if (originalColor >= colorPalette[maxIndex]) {
      return colorPalette[maxIndex];
    }
    if (maxIndex == minIndex + 1) {
      return colorPalette[maxIndex];
    }
    int midIndex = (maxIndex + minIndex) / 2;
    if (originalColor == colorPalette[midIndex]) {
      return colorPalette[midIndex];
    }
    if (originalColor < colorPalette[midIndex]) {
      return getPaletteHue(originalColor, minIndex + 1, midIndex - 1);
    } else {
      return getPaletteHue(originalColor, midIndex + 1, maxIndex - 1);
    }
  }
  void saveImageFile(void) { image.save("output.jpeg"); }

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
      std::cout << color << std::endl;
    }
  }
  bool isValid(void) { return (colorPalette.size() > 2); }
};

int main() {
  std::string uri = "targetImage.jpeg";
  ImageFilter filteredImage(uri.c_str());

  if (!filteredImage.isValid()) {
    std::cout << "ERROR: Palette size less than 2.";
    return 1;
  }

  // unordered_map <int colorHexValueAsDecimal, int frequencyOfColor>
  // Values are rounded to the nearest [roundingFactor]th value. Multiply by
  // roundingFactor to get decimial version of hex. multimap <int
  // frequencyOfColor, int colorHexValueAsDecimal>

  // Iterate through the map keeping track of our most frequent values
  // If top colors list is not yet full OR the frequency of the mapped color is
  // greater than the smallest frequency in the topColors list, insert the new
  // value. If applicable, remove the now-smallest value

  // Add those values to a vector that now contains our color palette (of
  // now-expanded hex color codes expressed in base 10)

  // Sort the codes

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