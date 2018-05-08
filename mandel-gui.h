#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <omp.h>
struct Img {
    int height;
    int width;
    int *pixels;
};

struct Complex {
    double real, imaginary;
};

void drawing_result(struct Img *image) {
    const char symbols[20] = "-*?.,@#>$<&(+ox=qpg";
    int row,col, pixel_value, letter_index;
    char letter;
    for (row= 0; row < image->height; row++) {
        for (col = 0; col < image->width; col++) {
            pixel_value = *(image->pixels+row*image->width + col);
            if (pixel_value>0){
                letter_index=pixel_value % (sizeof(symbols)-1);
                letter = symbols[letter_index];
            }
            else
                letter = ' ';
            printf("%c", letter);
            if(col+1 == image->width) 
                printf("\n");
        }
    }
    printf("\n");
}

