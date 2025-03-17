#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Define the structure for an RGB pixel
typedef struct {
    unsigned char r, g, b;
} RGBPixel;

// Function to convert RGB to grayscale using luminance
void rgbToGrayscale(const RGBPixel *rgbImage, int width, int height, double *grayscaleImage) {
    for (int i = 0; i < width * height; i++) {
        grayscaleImage[i] = 0.2126 * rgbImage[i].r + 0.7152 * rgbImage[i].g + 0.0722 * rgbImage[i].b;
    }
}

// Function to perform the Discrete Fourier Transform (DFT)
void performDFT(const double *input, double *realOut, double *imagOut, int width, int height) {
    int N = width * height;
    for (int u = 0; u < height; u++) {
        for (int v = 0; v < width; v++) {
            double realSum = 0.0;
            double imagSum = 0.0;

            for (int x = 0; x < height; x++) {
                for (int y = 0; y < width; y++) {
                    double angle = 2.0 * M_PI * ((u * x / (double)height) + (v * y / (double)width));
                    realSum += input[x * width + y] * cos(angle);
                    imagSum -= input[x * width + y] * sin(angle);
                }
            }

            realOut[u * width + v] = realSum;
            imagOut[u * width + v] = imagSum;
        }
    }
}

// Main function to process an RGB image
void processImage(const RGBPixel *rgbImage, int width, int height) {
    // Step 1: Convert RGB to grayscale
    double *grayscaleImage = (double *)malloc(width * height * sizeof(double));
    if (!grayscaleImage) {
        fprintf(stderr, "Memory allocation failed for grayscale image.\n");
        return;
    }
    rgbToGrayscale(rgbImage, width, height, grayscaleImage);

    // Step 2: Allocate memory for Fourier Transform outputs
    double *realOut = (double *)malloc(width * height * sizeof(double));
    double *imagOut = (double *)malloc(width * height * sizeof(double));
    if (!realOut || !imagOut) {
        fprintf(stderr, "Memory allocation failed for Fourier transform outputs.\n");
        free(grayscaleImage);
        return;
    }

    // Step 3: Perform the Fourier Transform
    performDFT(grayscaleImage, realOut, imagOut, width, height);

    // Step 4: Print or process the results (here we just print the magnitude)
    printf("Fourier Transform Magnitude:\n");
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            double magnitude = sqrt(realOut[i * width + j] * realOut[i * width + j] +
                                    imagOut[i * width + j] * imagOut[i * width + j]);
            printf("%0.2f ", magnitude);
        }
        printf("\n");
    }

    // Free allocated memory
    free(grayscaleImage);
    free(realOut);
    free(imagOut);
}

int main() {
    // Example usage: Define a small 2x2 RGB image
    RGBPixel image[4] = {
        {255, 0, 0}, {0, 255, 0},
        {0, 0, 255}, {255, 255, 255}
    };

    int width = 2, height = 2;

    processImage(image, width, height);

    return 0;
}
