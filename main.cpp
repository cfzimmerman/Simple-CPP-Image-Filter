#include <string.h>

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

// RGB_Triple is a frequently-used structure for storing data about a given color within an image.
class RGB_Triple {
private:
    // Commonly used variables. Instantiated here and reused to speed up by reducing the number of allocations needed in loops.
    int red;
    int green;
    int blue;
    int frequency;

public:
    RGB_Triple(int r = 0, int g = 0, int b = 0, int fr = 0) : red(r), green(g), blue(b), frequency(fr){};
    void mergeValue(int r, int g, int b) {
        if (frequency < 10) {
            red = ((frequency * red) + r) / (frequency + 1);
            green = ((frequency * green) + g) / (frequency + 1);
            blue = ((frequency * blue) + b) / (frequency + 1);
        } else {
            red = (red * 9 + r) / 10;
            green = (green * 9 + g) / 10;
            blue = (blue * 9 + b) / 10;
        }
        frequency++;
    }
    int getRed(void) { return red; }
    int getGreen(void) { return green; }
    int getBlue(void) { return blue; }
};

class ImageFilter {
private:
    int width;
    int height;
    CImg<unsigned char> image;
    // The filter averages the colors in different R, G, and B color categories and creates a palette with light and dark options for each.
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
    // for lighter and darker hues. Index 0 is for lighter (dominant value greater than one third), index 1 is for middle shades (dominant value in
    // middle third), and index 2 is for darker (dominant value less than half).
    std::vector<RGB_Triple> Rg = {vectorDefault, vectorDefault, vectorDefault}, Rb = {vectorDefault, vectorDefault, vectorDefault},
                            Gr = {vectorDefault, vectorDefault, vectorDefault}, Gb = {vectorDefault, vectorDefault, vectorDefault},
                            Br = {vectorDefault, vectorDefault, vectorDefault}, Bg = {vectorDefault, vectorDefault, vectorDefault},
                            noColor = {vectorDefault, vectorDefault, vectorDefault};

    void getColorPalette(void) {
        int r, g, b;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                r = image(x, y, 0);
                g = image(x, y, 1);
                b = image(x, y, 2);
                if (r == g && r == b) {
                    noColor[getLuminosityIndex(r)].mergeValue(r, g, b);
                } else if (r >= g && r >= b) {
                    if (g >= b) {
                        Rg[getLuminosityIndex(r)].mergeValue(r, g, b);
                    } else {
                        Rb[getLuminosityIndex(r)].mergeValue(r, g, b);
                    }
                } else if (g >= r && g >= b) {
                    if (r >= b) {
                        Gr[getLuminosityIndex(g)].mergeValue(r, g, b);
                    } else {
                        Gb[getLuminosityIndex(g)].mergeValue(r, g, b);
                    }
                } else if (b >= r && b >= g) {
                    if (r >= b) {
                        Br[getLuminosityIndex(b)].mergeValue(r, g, b);
                    } else {
                        Bg[getLuminosityIndex(b)].mergeValue(r, g, b);
                    }
                }
            }
        }
    }
    // Based on the RGB and luminosity alignment of an input color, return the closest color available within the pre-defined palette.
    RGB_Triple getPaletteHue(int r, int g, int b) {
        if (r == g && r == b) {
            return noColor[getLuminosityIndex(r)];
        } else if (r >= g && r >= b) {
            if (g >= b) {
                return Rg[getLuminosityIndex(r)];
            }
            return Rb[getLuminosityIndex(r)];
        } else if (g >= r && g >= b) {
            if (r >= b) {
                return Gr[getLuminosityIndex(g)];
            }
            return Gb[getLuminosityIndex(g)];
        }
        if (r >= g) {
            return Br[getLuminosityIndex(b)];
        }
        return Bg[getLuminosityIndex(b)];
    }
    // Renames filtered files and places them in an output folder.
    std::string getFileName(std::string uri) {
        int searcher, slash = 0;
        for (searcher = 0; searcher < uri.size(); searcher++) {
            if (uri[searcher] == '/') {
                slash = searcher;
                slash++;
            }
        }
        return ("output/filtered-" + uri.substr(slash, searcher));
    }

public:
    ImageFilter(std::string uri) {
        CImg<unsigned char> newImg(uri.c_str());
        image = newImg;
        width = image.width();
        height = image.height();
        getColorPalette();
    }
    void saveImageFile(std::string uri) { image.save(getFileName(uri).c_str()); }
    void applyFilter(void) {
        RGB_Triple newColor;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                newColor = getPaletteHue(image(x, y, 0), image(x, y, 1), image(x, y, 2));
                image(x, y, 0) = newColor.getRed();
                image(x, y, 1) = newColor.getGreen();
                image(x, y, 2) = newColor.getBlue();
            }
        }
    }
};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "Please provide a file name argument" << std::endl;
        return 1;
    }
    // Image file URL is passed as a CLI argument
    std::string uri = argv[1];
    ImageFilter newImage(uri.c_str());

    newImage.applyFilter();
    newImage.saveImageFile(uri);
    return 0;
}

/*
To compile: (deprecated flag needed as of Dec. 2022)
g++ -std=c++11 -Wno-deprecated -I/opt/X11/include -L/opt/X11/lib -lX11 -lpthread main.cpp -o main

To run:
./main input/img3.jpeg
*/
