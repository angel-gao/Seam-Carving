#include "c_img.h"
//#include "c_img.c"
#include "seamcarving.h"
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
/*
void create_img(struct rgb_img **im, size_t height, size_t width){
    *im = (struct rgb_img *)malloc(sizeof(struct rgb_img));
    (*im)->height = height;
    (*im)->width = width;
    (*im)->raster = (uint8_t *)malloc(3 * height * width);
}


int read_2bytes(FILE *fp){
    uint8_t bytes[2];
    fread(bytes, sizeof(uint8_t), 1, fp);
    fread(bytes+1, sizeof(uint8_t), 1, fp);
    return (  ((int)bytes[0]) << 8)  + (int)bytes[1];
}

void write_2bytes(FILE *fp, int num){
    uint8_t bytes[2];
    bytes[0] = (uint8_t)((num & 0XFFFF) >> 8);
    bytes[1] = (uint8_t)(num & 0XFF);
    fwrite(bytes, 1, 1, fp);
    fwrite(bytes+1, 1, 1, fp);
}

void read_in_img(struct rgb_img **im, char *filename){
    FILE *fp = fopen(filename, "rb");
    size_t height = read_2bytes(fp);
    size_t width = read_2bytes(fp);
    create_img(im, height, width);
    fread((*im)->raster, 1, 3*width*height, fp);
    fclose(fp);
}

void write_img(struct rgb_img *im, char *filename){
    FILE *fp = fopen(filename, "wb");
    write_2bytes(fp, im->height);
    write_2bytes(fp, im->width);
    fwrite(im->raster, 1, im->height * im->width * 3, fp);
    fclose(fp);
}


//colour: 0 represnet R, 1 for G, 2 for B
uint8_t get_pixel(struct rgb_img *im, int y, int x, int col){
    return im->raster[3 * (y*(im->width) + x) + col];
}

void set_pixel(struct rgb_img *im, int y, int x, int r, int g, int b){
    im->raster[3 * (y*(im->width) + x) + 0] = r;
    im->raster[3 * (y*(im->width) + x) + 1] = g;
    im->raster[3 * (y*(im->width) + x) + 2] = b;
}



void destroy_image(struct rgb_img *im)
{
    free(im->raster);
    free(im);
}


void print_grad(struct rgb_img *grad){
    int height = grad->height;
    int width = grad->width;
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            printf("%d\t", get_pixel(grad, i, j, 0));
        }
    printf("\n");    
    }
}*/

void print_bestArr(double* bestArr, int width, int size){
    int counter = 0;
    for(int i = 0; i < size; i++){
        if(counter == width){
            printf("\n");
            counter = 0;
        }
        printf("%f   ", bestArr[i]);
        counter += 1;
    }
    printf("\n");
}

void print_recoverPath(int* path, int size){
    for(int i = 0; i < size; i++){
        printf("%d\n ", path[i]);
    }
    printf("\n");
}

/*
for a image, the pixal format is
(1,5,1), (1,4,6)
(1,2,3), (1,4,7)

row major order: (1,5,1), (1,4,6), (1,2,3), (1,4,7)     Stored in tuple?????? or in number????????
pixel at location (y,x): 3*(y*width + x) + colour (0:R, 1:G, 2:B)   <--- skip # of rows, go to column with index x, multiply all by 3 (bc we store 3 values in one tuple)
*/

/*
[[a,b,c,d],
[e,f,g,h],
[i,j,k,l]]

width: 4
Read out in row major order: a,b,c,d,e,f,g,h,i,j,k,l
g is at (1,2). in the raster, g is at 1(reach to row1)*width + 2
*/

void calc_energy(struct rgb_img *im, struct rgb_img **grad){
    int xFront, xBack, yUp, yDown, Rx, Gx, Bx, Ry, Gy, By, energy, pixalE;
    //Need to make sure grad is actually being created with dimension same as image
    create_img(grad, im->height, im->width);

    for(int y = 0; y < im->height; y++){
        //obtain 4 index that need to calculate difference in RGB from
        if(y - 1<0) {
            yUp = im->height-1;
        } else {yUp = y-1;}

        if(y + 1 > im->height-1){
            yDown = 0;
        } else {yDown = y+1;}


        for(int x = 0; x < im->width; x++){
            //obtain 4 index that need to calculate difference in RGB from
            if(x - 1 < 0){
                xFront = im->width-1;
            } else{ xFront = x-1;}

            if(x + 1 > im->width-1){
                xBack = 0;
            } else {xBack = x+1;}

            Rx = get_pixel(im, y, xFront, 0) - get_pixel(im, y, xBack, 0); 
            Gx = get_pixel(im, y, xFront, 1) - get_pixel(im, y, xBack, 1); 
            Bx = get_pixel(im, y, xFront, 2) - get_pixel(im, y, xBack, 2); 

            Ry = get_pixel(im, yUp, x, 0) - get_pixel(im, yDown, x, 0);
            Gy = get_pixel(im, yUp, x, 1) - get_pixel(im, yDown, x, 1);
            By = get_pixel(im, yUp, x, 2) - get_pixel(im, yDown, x, 2);

            energy = sqrt(pow(Rx,2) + pow(Gx,2) + pow(Bx,2) + pow(Ry,2) + pow(Gy,2) + pow(By,2));
            pixalE = (uint8_t)(energy/10);
            set_pixel(*grad, y, x, pixalE, pixalE, pixalE);

        }
    }
}

double min3(double a, double b, double c){
    if(a < b && a < c){
        return a;
    } else if(b < c){
        return b;
    } else{
        return c;
    }
}

double min2(double a, double b){
    if(a < b){
        return a;
    } else{
        return b;
    }
}




void dynamic_seam(struct rgb_img *grad, double **best_arr) {
    //allocate best_arr
    *best_arr = (double*)malloc(sizeof(double) * (grad->width) * (grad->height));
    //initializa first row of best arr = value of dual energy at row 0
    for(int i = 0; i < grad->width; i++){
        (*best_arr)[i] = get_pixel(grad, 0, i, 0);
    }

    //Logic: For each location, find the minimum amount required to reach it from the three adjacent locations in the previous row
    //we assume 3 adj locations already have best amount stored
    int mostL = 0, mostR = 0; 
    double topL, topS, topR; 
    for(int y = 1; y < grad->height; y++){
        for(int x = 0; x < grad->width; x++){
            if(x == 0){
                mostL = 1;
            } else if(x == grad->width-1){
                mostR = 1;
            }

            if(mostL){
                topS = (*best_arr)[(y-1) * grad->width + (x)];
                topR = (*best_arr)[(y-1) * grad->width + (x+1)];
                (*best_arr)[y * grad->width + x] = get_pixel(grad, y, x, 0) + min2(topR, topS);

            } else if(mostR){
                topL =  (*best_arr)[(y-1) * grad->width + (x-1)];
                topS = (*best_arr)[(y-1) * grad->width + (x)];
                (*best_arr)[y * grad->width + x] = get_pixel(grad, y, x, 0) + min2(topL, topS);
                
            } else{
                //reach from topLeft location, obtain value
                topL =  (*best_arr)[(y-1) * grad->width + (x-1)];
                //reach from topRight location
                topR = (*best_arr)[(y-1) * grad->width + (x+1)];
                //reach from top straight location
                topS = (*best_arr)[(y-1) * grad->width + (x)];

                //The minimum amount to reach cur location is the min of topL, topR, topS, plus the current val in location
                (*best_arr)[y * grad->width + x] = get_pixel(grad, y, x, 0) + min3(topL, topR, topS);
            }
            mostL = 0;
            mostR = 0;
        }
    }
}

void recover_path(double *best, int height, int width, int **path){
    int level = height-1; //because level represent index 0 to h-1 while height is 1 to h
    *path = (int*)malloc(sizeof(int)* level);
    //printf("%d", rowMinIndex(best, 4, width));
    

    //get min index for last row first, then only need to check previous 3 row indices
    int min = best[width*height-1]; //index of last element in best arr
    int minInx = width*height-1; 
    for(int i = 0; i < width; i++){ //loop through  last row 
        if (min > best[width* (height-1) + i]){
            min = best[width* (height-1) + i]; 
            minInx = width* (height-1) + i; 
        }
    }
    minInx -= width*(height-1); //convert best arr index into column number
    (*path)[level] = minInx; 
    level -= 1; 

    //Note: now the min indx only represent column, instead of actual index in best arr as we've already identified the min col index for last row, previous row can always this that as reference, either +1 or keep same or -1
    //Now only check three above 3 index
    for(level; level >= 0; level--){
        //if the current checking element is most left
        if(minInx == 0){
            min = min2(best[level*width + minInx + 1], best[level*width + minInx]);
            //the minimum of two option, the one to right is min, update minInx
            if(min == best[level*width + minInx + 1]){
                minInx += 1;
            }
        }
        //if the current checkng element is most right
        else if(minInx == width -1){
            min = min2(best[level*width + minInx - 1], best[level*width + minInx]);
            //given the min of two option, if the top left one is min, update minInx
            if(min == best[level*width + minInx - 1]){
                minInx -= 1;
            }

        }
        //if current checking element is in middle 
        else {
            min = min3(best[level*width + minInx - 1], best[level*width + minInx + 1], best[level*width + minInx]);
            if(min == best[level*width + minInx - 1]){
                minInx -= 1;
            } else if(min == best[level*width + minInx + 1]){
                minInx += 1; 
            }
        }
        (*path)[level] = minInx; 

    }


/*
    for(level; level >= 0; level--){
        int minInxEachRow = rowMinIndex(best, level, width);
        (*path)[level] = minInxEachRow;
    }*/
}


/*
int rowMinIndex(double* bestArr, int row, int width){
    //first get the index of last row, then we only need to check previous 3 row indices
    int min = bestArr[width* row];
    int minInx = width* row;
    for(int i = width*row + 1; i < width* (row+1); i++){
        if(min > bestArr[i]) {
            min = bestArr[i];
            minInx = i;
        }
    }
    return minInx - (width*row);

}*/










void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path) {
    create_img(dest, src->height, src->width-1);
    int destInx = 0;

    int srcX = 0;
    for(int y = 0; y < (*dest)->height; y++){
        int skipInx = path[y];
        for(int x = 0; x < (*dest)->width; x++){
            //if current x location is location need to skip
            if(srcX == skipInx){
                //update srcX, but keep current counting destX not changed (so they end together as destX has 1 less that srcX)
                srcX += 1;
                x -= 1; 
                continue; //skip this loop
            } 
            set_pixel(*dest, y, x, get_pixel(src, y, srcX, 0), get_pixel(src, y, srcX, 1), get_pixel(src, y, srcX, 2));
            srcX += 1; //manually updating srcX
        }
        srcX = 0; //when one row finish checking, manually set srcX = 0
        //printf("\n");
    }
}



/*
int main(){
   
    struct rgb_img *im;
    struct rgb_img *grad;
    double *bestArr;
    int* optPath; 
    struct rgb_img *dest;
    read_in_img(&im, "6x5.bin");
    calc_energy(im,  &grad);
  
    print_grad(grad);

    dynamic_seam(grad, &bestArr);

    print_bestArr(bestArr, grad->width, grad->width * grad->height);

    recover_path(bestArr, grad->height, grad->width, &optPath);

    print_recoverPath(optPath, grad->height);



    remove_seam(im, &dest, optPath);
   
    struct rgb_img *im;
    struct rgb_img *cur_im;
    struct rgb_img *grad;
    double *best;
    int *path;

    read_in_img(&im, "HJoceanSmall.bin");
    
    for(int i = 0; i < 507; i++){
        printf("i = %d\n", i);
        calc_energy(im,  &grad);


        


        dynamic_seam(grad, &best);
        recover_path(best, grad->height, grad->width, &path);



        //print_recoverPath(path, grad->height);





        remove_seam(im, &cur_im, path);

        char filename[200];
        sprintf(filename, "img%d.bin", i);
        write_img(cur_im, filename);


        destroy_image(im);
        destroy_image(grad);
        free(best);
        free(path);
        im = cur_im;
    }
    destroy_image(im);


    


    
    

    
    

}*/
