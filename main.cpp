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
        if (frequency < 10) {
            int safeFrequency = frequency < 8421500 ? frequency : 8421500;
            red = ((safeFrequency * red) + r) / (safeFrequency + 1);
            green = ((safeFrequency * green) + g) / (safeFrequency + 1);
            blue = ((safeFrequency * blue) + b) / (safeFrequency + 1);
        } else {
            red = (red * 9 + r) / 10;
            green = (green * 9 + g) / 10;
            blue = (blue * 9 + b) / 10;
        }
        /*
            int safeFrequency = frequency < 8421500 ? frequency : 8421500;
            red = (safeFrequency * red + r) / (safeFrequency + 1);
            green = (safeFrequency * green + g) / (safeFrequency + 1);
            blue = (safeFrequency * blue + b) / (safeFrequency + 1);
        */

        frequency++;
    }
    int getRed(void) { return red; }
    int getGreen(void) { return green; }
    int getBlue(void) { return blue; }
};

class ImageFilter {
private:
    CImg<unsigned char> image;
    std::vector<RGB_Triple> colorPalette;
    int getLuminosityIndex(int intensity) {
        if (intensity > 170) {
            return 0;
        } else if (intensity < 85) {
            return 2;
        }
        return 1;
    }
    int getColorIndex(int intensity) { return (intensity > 170 ? 0 : 1); }
    RGB_Triple vectorDefault;
    // Colors ranked by greatest to least pronounced color value. Rg is red >= green >= blue, Bg is blue >= green >= red, etc. Each set has a value
    // for lighter and darker hues. Index 0 is for lighter (dominant value greater than half), index 1 is for darker (dominant value less than half).
    std::vector<RGB_Triple> Rg = {vectorDefault, vectorDefault, vectorDefault}, Rb = {vectorDefault, vectorDefault, vectorDefault},
                            Gr = {vectorDefault, vectorDefault, vectorDefault}, Gb = {vectorDefault, vectorDefault, vectorDefault},
                            Br = {vectorDefault, vectorDefault, vectorDefault}, Bg = {vectorDefault, vectorDefault, vectorDefault},
                            noColor = {vectorDefault, vectorDefault, vectorDefault};

    void getColorPalette(void) {
        int r, g, b, insertIndex;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                r = image(x, y, 0);
                g = image(x, y, 1);
                b = image(x, y, 2);
                if (r == g && r == b) {
                    insertIndex = getLuminosityIndex(r);
                    noColor[insertIndex].mergeValue(r, g, b);
                } else if (r >= g && r >= b) {
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
                } else if (b >= r && b >= g) {
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

    RGB_Triple getPaletteHue(int r, int g, int b) {
        int brightnessIndex;
        if (r == g && r == b) {
            brightnessIndex = getLuminosityIndex(r);
            return noColor[brightnessIndex];
        } else if (r >= g && r >= b) {
            brightnessIndex = getLuminosityIndex(r);
            if (g >= b) {
                return Rg[brightnessIndex];
            }
            return Rb[brightnessIndex];
        } else if (g >= r && g >= b) {
            brightnessIndex = getLuminosityIndex(g);
            if (r >= b) {
                return Gr[brightnessIndex];
            }
            return Gb[brightnessIndex];
        }
        brightnessIndex = getLuminosityIndex(b);
        if (r >= g) {
            return Br[brightnessIndex];
        }
        return Bg[brightnessIndex];
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
        std::cout << "\nNo color: White, black, grey: " << std::endl;
        std::cout << "noColor[0]: " << noColor[0].getRed() << ", " << noColor[0].getGreen() << ", " << noColor[0].getBlue() << std::endl;
        std::cout << "noColor[1]: " << noColor[1].getRed() << ", " << noColor[1].getGreen() << ", " << noColor[1].getBlue() << std::endl;

        std::cout << "\nRed dominant: Red-Green, Red-Blue: " << std::endl;
        std::cout << "Rg[0]: " << Rg[0].getRed() << ", " << Rg[0].getGreen() << ", " << Rg[0].getBlue() << std::endl;
        std::cout << "Rg[1]: " << Rg[1].getRed() << ", " << Rg[1].getGreen() << ", " << Rg[1].getBlue() << std::endl;
        std::cout << "Rb[0]: " << Rb[0].getRed() << ", " << Rb[0].getGreen() << ", " << Rb[0].getBlue() << std::endl;
        std::cout << "Rb[1]: " << Rb[1].getRed() << ", " << Rb[1].getGreen() << ", " << Rb[1].getBlue() << std::endl;

        std::cout << "\nGreen dominant: Green-Red, Green-Blue: " << std::endl;
        std::cout << "Gr[0]: " << Gr[0].getRed() << ", " << Gr[0].getGreen() << ", " << Gr[0].getBlue() << std::endl;
        std::cout << "Gr[1]: " << Gr[1].getRed() << ", " << Gr[1].getGreen() << ", " << Gr[1].getBlue() << std::endl;
        std::cout << "Gb[0]: " << Gb[0].getRed() << ", " << Gb[0].getGreen() << ", " << Gb[0].getBlue() << std::endl;
        std::cout << "Gb[1]: " << Gb[1].getRed() << ", " << Gb[1].getGreen() << ", " << Gb[1].getBlue() << std::endl;

        std::cout << "\nBlue dominant: Blue-Red, Blue-Green: " << std::endl;
        std::cout << "Br[0]: " << Br[0].getRed() << ", " << Br[0].getGreen() << ", " << Br[0].getBlue() << std::endl;
        std::cout << "Br[1]: " << Br[1].getRed() << ", " << Br[1].getGreen() << ", " << Br[1].getBlue() << std::endl;
        std::cout << "Bg[0]: " << Bg[0].getRed() << ", " << Bg[0].getGreen() << ", " << Bg[0].getBlue() << std::endl;
        std::cout << "Bg[1]: " << Bg[1].getRed() << ", " << Bg[1].getGreen() << ", " << Bg[1].getBlue() << std::endl;
        std::cout << "\n";
    }

    bool isValid(void) { return (colorPalette.size() > 2); }
    void saveImageFile(void) { image.save("output.jpeg"); }
    void applyFilter(void) {
        RGB_Triple newColor;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                newColor = getPaletteHue(image(x, y, 0), image(x, y, 1), image(x, y, 2));
                image(x, y, 0) = newColor.getRed();
                image(x, y, 1) = newColor.getGreen();
                image(x, y, 2) = newColor.getBlue();
            }
        };
    }
};

int main() {
    std::string uri = "targetImage1.jpeg";
    ImageFilter filteredImage(uri.c_str());

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