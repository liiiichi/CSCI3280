/*

CSCI 3280, Introduction to Multimedia Systems
Spring 2023

Assignment 01 Skeleton

photomosaic.cpp

*/

#include "stdio.h"
#include <iostream>
#include <vector>
#include <string>
#include "malloc.h"
#include "memory.h"
#include "math.h"
#include "bmp.h"		//	Simple .bmp library
#include "list_files.h" //	Simple list file library

using namespace std;

#define SAFE_FREE(p)  { if(p){ free(p);  (p)=NULL;} }

Bitmap resizeImage(Bitmap& source_bitmap, int output_w, int output_h);
double calculateAverageBrightness(Bitmap& bmp, int cell_w, int cell_h, int start_x, int start_y, bool for_tiles);
int findNearestMatch(double region_bright, const vector<double>& tiles_bright);
void copyImageToOutput(Bitmap& photoTile, Bitmap& outputImage, int xOffset, int yOffset);
void convertToGrayscale(Bitmap& image);

int main(int argc, char** argv)
{
	// Parse output and cell shape specified in argv[3]
	int output_w, output_h, cell_w, cell_h;
	sscanf(argv[3], "%d,%d,%d,%d", &output_w, &output_h, &cell_w, &cell_h);

	// Read source bitmap from argv[1]
	Bitmap input_bitmap(argv[1]);
	int input_w = input_bitmap.getWidth();
	int input_h = input_bitmap.getHeight();

	// List .bmp files in argv[2] and do preprocessing
	vector<string> file_path_vector;
	list_files(argv[2], ".bmp", file_path_vector, false);

    // Resize source bitmap
    Bitmap output_bitmap = resizeImage(input_bitmap, output_w, output_h);

    // Preprocessing tiles
    vector<int> regionMatchedTiles;
    vector<Bitmap> resized_tiles;
    vector<double> tiles_bright;
    int num_tiles = file_path_vector.size();
    for (int i = 0; i < num_tiles; i++) {
        double temp_bright;
        Bitmap tiles, temp;
        tiles.create(file_path_vector[i].data());
        temp = resizeImage(tiles, cell_w, cell_h);
        // Resize tiles
        resized_tiles.push_back(temp);
        // Calculate average brightness of tile
        temp_bright = calculateAverageBrightness(temp, cell_w, cell_h, 0, 0, 1);
        tiles_bright.push_back(temp_bright);
    }
    for (int y = 0; y < output_h; y += cell_h) {
        for (int x = 0; x < output_w; x += cell_w) {
            // Calculate average brightness of region
            double temp_bright = calculateAverageBrightness(output_bitmap, cell_w, cell_h, x, y, 0);
            int temp_index = findNearestMatch(temp_bright, tiles_bright);
            regionMatchedTiles.push_back(temp_index);
        }
    }

	// Compose the output image as save to argv[4]
    int tiles_index = 0;
    for (int y = 0; y < output_h; y += cell_h) {
        for (int x = 0; x < output_w; x += cell_w) {
            copyImageToOutput(resized_tiles[regionMatchedTiles[tiles_index]], output_bitmap, x, y);
            tiles_index++;
        }
    }
    // Convert output image to grayscale
    convertToGrayscale(output_bitmap);
    //Save output image
    output_bitmap.save(argv[4]);
	return 0;
}

Bitmap resizeImage(Bitmap& source_bitmap, int output_w, int output_h) {
    int source_w = source_bitmap.getWidth();
    int source_h = source_bitmap.getHeight();
    Bitmap resized_bitmap(output_w, output_h);

    float x_mapping_func = (float)(source_w) / (output_w);
    float y_mapping_func = (float)(source_h) / (output_h);

    for (int y = 0; y < output_h; ++y) {
        for (int x = 0; x < output_w; ++x) {
            // Calculate corresponding location in source image
            float x_pixel = x * x_mapping_func;
            float y_pixel = y * y_mapping_func;

            // Find the four pixel surrounding to the point P in source image
            int x1 = floor(x_pixel);
            int y1 = floor(y_pixel);
            int x2 = x1 + 1;
            int y2 = y1 + 1;

            // Handle border cases the pixel P that has only 2 neighbors
            if (x2 >= source_w) x2 = x1;
            if (y2 >= source_h) y2 = y1;

            // Get colors of the four pixel
            Color q11, q12, q21, q22;
            source_bitmap.getColor(x1, y1, q11.R, q11.G, q11.B);
            source_bitmap.getColor(x1, y2, q12.R, q12.G, q12.B);
            source_bitmap.getColor(x2, y1, q21.R, q21.G, q21.B);
            source_bitmap.getColor(x2, y2, q22.R, q22.G, q22.B);

            // Calculation for bilinear interpolation
            float x_dist = x_pixel - x1;
            float y_dist = y_pixel - y1;

            Color r1, r2;
            r1.R = (1 - x_dist) * q21.R + x_dist * q11.R;
            r1.G = (1 - x_dist) * q21.G + x_dist * q11.G;
            r1.B = (1 - x_dist) * q21.B + x_dist * q11.B;

            r2.R = (1 - x_dist) * q22.R + x_dist * q12.R;
            r2.G = (1 - x_dist) * q22.G + x_dist * q12.G;
            r2.B = (1 - x_dist) * q22.B + x_dist * q12.B;

            Color p;
            p.R = ((1 - y_dist) * r2.R + y_dist * r1.R);
            p.G = ((1 - y_dist) * r2.G + y_dist * r1.G);
            p.B = ((1 - y_dist) * r2.B + y_dist * r1.B);

            resized_bitmap.setColor(x, y, p.R, p.G, p.B);
        }
    }

    return resized_bitmap;
}

double calculateAverageBrightness(Bitmap& bmp, int cell_w, int cell_h, const int start_x, const int start_y, bool for_tiles) {
    int total_pixels = 0;
    double total_brightness = 0;

    // Calculate tiles brightness
    if (for_tiles) {
        int tile_width = bmp.getWidth();
        int tile_height = bmp.getHeight();
        total_pixels = tile_width * tile_height;
        for (int i = 0; i < tile_width; i++) {
            for (int j = 0; j < tile_height; j++) {
                Color temp;
                bmp.getColor(i, j, temp.R, temp.G, temp.B);
                total_brightness += 0.299 * temp.R + 0.587 * temp.G + 0.114 * temp.B;
            }
        }
    }
    // Calculate source image region brightness
    else {
        total_pixels = cell_w * cell_h;
        for (int x = start_x; x < start_x + cell_w; x++) {
            for (int y = start_y; y < start_y + cell_h; y++) {
                Color temp;
                bmp.getColor(x, y, temp.R, temp.G, temp.B);
                total_brightness += 0.299 * temp.R + 0.587 * temp.G + 0.114 * temp.B;
                total_pixels++;
            }
        }
    }

    return total_brightness / total_pixels;
}

int findNearestMatch(double region_bright, const vector<double>& tiles_bright) {
    int best_match_index = 0;
    // Initial first smallest difference
    double smallest_diff = abs(region_bright - tiles_bright[0]);
    // Find any smaller difference
    for (int i = 1; i < tiles_bright.size(); i++) {
        double diff = abs(region_bright - tiles_bright[i]);
        if (diff < smallest_diff) {
            smallest_diff = diff;
            best_match_index = i;
        }
    }

    return best_match_index;
}

void copyImageToOutput(Bitmap& photoTile, Bitmap& outputImage, int x_Image, int y_Image) {
    // Get the dimensions of the photo tile and output image
    int tile_w = photoTile.getWidth();
    int tile_h = photoTile.getHeight();
    int output_w = outputImage.getWidth();
    int output_h = outputImage.getHeight();

    // Loop to copy every pixel in tile to specify region
    for (int y = 0; y < tile_h; y++) {
        for (int x = 0; x < tile_w; x++) {
            // Get the color of the pixel in tile
            Color tile;
            photoTile.getColor(x, y, tile.R, tile.G, tile.B);

            // Calculate the correct position in specify region
            int outputX = x + x_Image;
            int outputY = y + y_Image;

            // Border check
            if (outputX >= 0 && outputX < output_w && outputY >= 0 && outputY < output_h) {
                // Set the tile pixel color to region pixel
                outputImage.setColor(outputX, outputY, tile.R, tile.G, tile.B);
            }
        }
    }
}

void convertToGrayscale(Bitmap& image) {
    int width = image.getWidth();
    int height = image.getHeight();

    // Loop every pixel in image
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Get the color of the pixel
            Color pixel;
            image.getColor(x, y, pixel.R, pixel.G, pixel.B);

            // Calculate the grayscale RGB value
            unsigned char gray = 0.299 * pixel.R + 0.587 * pixel.G + 0.114 * pixel.B;

            // Set the pixel to the grayscale
            image.setColor(x, y, gray, gray, gray);
        }
    }
}
