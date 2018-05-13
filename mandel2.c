#include "mandel-gui.h"


void computing (struct Img * image, unsigned nb_iter, double x_min, double x_max, double y_min, double y_max) {
    //Compute values to scale computational region to window
    int row,col;
    double scale_real = (x_max - x_min) / image -> width;
    double scale_imaginary = (y_max - y_min) / image -> height;

    omp_set_num_threads(16);
    omp_set_nested(1);

    #pragma omp parallel
    {
    	    #pragma omp for nowait private(col)
	    for (row = 0; row < image->height; row++) {
		 #pragma omp parallel for
		for (col = 0; col < image->width; col++) {
			struct Complex z,c;
			double aux;
			z.real=0.0;
			z.imaginary=0.0;
			//Scale coordenates to actual region
			c.real=x_min + col*scale_real;
			c.imaginary=y_min + row*scale_imaginary;
		    int iter = 0;
		    double lenght=z.real*z.real + z.imaginary*z.imaginary;
		    //Computing z0, z1 and so on until divergence or maximum iterations

		    for(iter=0;  (lenght < 4.0 ) && ( iter < nb_iter ); iter++ ) {
			aux = z.real*z.real - z.imaginary*z.imaginary + c.real;
			z.imaginary = 2.0*z.real*z.imaginary + c.imaginary;
			z.real = aux;
			lenght=z.real*z.real + z.imaginary*z.imaginary;
		    }
		    if(iter == nb_iter) 
		    	iter = 0;
		    image->pixels[row*image->width + col] = iter;
		}
	    }
    }
}

int main(int argc, char*argv[]) {
    struct Img image;
    double start,end;
    //Default values
    int num_iter = 256;
    double x_min=-2.5, x_max=1.5,y_min=-2.0,y_max=2.0;
    image.height = 40;
    image.width = 80;
    
    for(int i=1;i<argc;i++){
    	if(strcmp(argv[i], "-xm")==0){
    		i++;
    		x_min=atof(argv[i]);
    	}
    	if(strcmp(argv[i], "-xM")==0){
    		i++;
    		x_max=atof(argv[i]);
    	}
    	if(strcmp(argv[i], "-ym")==0){
    		i++;
    		y_min=atof(argv[i]);
    	}
    	if(strcmp(argv[i], "-yM")==0){
    		i++;
    		y_max=atof(argv[i]);
    	}
    	if(strcmp(argv[i], "-iH")==0){
    		i++;
    		image.height=atoi(argv[i]);
    	}
    	if(strcmp(argv[i], "-iW")==0){
    		i++;
    		image.width=atoi(argv[i]);
    	}
    	if(strcmp(argv[i], "-ni")==0){
    		i++;
    		num_iter=atoi(argv[i]);
    	}
    }
    image.pixels = (int*)malloc(sizeof (int) * image.height*image.width);
    start=omp_get_wtime();

    computing(&image, num_iter, x_min, x_max, y_min, y_max);

    end=omp_get_wtime();
    printf("\nExecution time is %g\n", end-start);


    drawing_result(&image);
    free(image.pixels);
    return 0;
}
