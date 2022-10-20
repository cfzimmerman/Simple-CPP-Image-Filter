#include <iostream>
#include <cstdlib>
#include <unordered_map>
#include <map>
#include <functional>
#include <algorithm>
#include <vector>
#include "CImg.h"
#define cimg_use_png
#define cimg_use_jpeg
#define cimg_use_tiff
#define cimg_use_curl

using namespace cimg_library;

int main()
{
  CImg<unsigned char> img("targetImage.jpeg");

  int width = img.width();
  int height = img.height();
  int roundingFactor = 16, topColorLimit = 16;
  std::vector<int> colorPalette;

  // unordered_map <int colorHexValueAsDecimal, int frequencyOfColor>
  // Values are rounded to the nearest [roundingFactor]th value. Multiply by roundingFactor to get decimial version of hex.
  std::unordered_map<int, int> colorMap;
  // multimap <int frequencyOfColor, int colorHexValueAsDecimal>
  std::multimap<int, int, std::greater<int>> topColors;

  // Load all the colors into a map to count frequency
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      colorMap[((img(x, y, 0) * 65536) + (img(x, y, 1) * 256) + img(x, y, 2)) / roundingFactor]++;
    }
  }

  // Iterate through the map keeping track of our most frequent values
  // If top colors list is not yet full OR the frequency of the mapped color is greater than the smallest frequency in the topColors list, insert the new value. If applicable, remove the now-smallest value
  for (auto colorPair : colorMap)
  {
    if (topColors.size() < topColorLimit || colorPair.second > topColors.rbegin()->first)
    {
      topColors.insert(std::pair<int, int>(colorPair.second, colorPair.first));
    }
    if (topColors.size() > topColorLimit)
    {
      topColors.erase(topColors.rbegin()->first);
    }
  }

  // Add those values to a vector that now contains our color palette (of now-expanded hex color codes expressed in base 10)
  for (auto colorRank : topColors)
  {
    colorPalette.push_back(colorRank.second * roundingFactor);
  }

  // Sort the codes
  std::sort(colorPalette.begin(), colorPalette.end());

  for (int color : colorPalette)
  {
    std::cout << color << std::endl;
  }

  return 0;
}

/*
  int channels = img.spectrum();

  std::cout << "Dimensions: " << width << "x" << height << " " << channels << "channels" << std::endl;

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

  img.save("output.jpeg");

  img.save("newlyCreated.jpeg");

*/

// clang++ main.cpp -o main -I/opt/X11/include -L/opt/X11/lib -lX11 -lpthread -std=c++11