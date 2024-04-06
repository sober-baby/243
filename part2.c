int pixel_buffer_start; // global variable

struct fb_t {
    unsigned short volatile pixels[256][512];
};

#define uint32 unsigned int
struct videoout_t {
    struct fb_t *volatile fbp; // front frame buffer
    struct fb_t *volatile bfbp; // back frame buffer
    uint32 volatile resolution; // resolution two fields of 16b each, packed into a 32b word
    uint32 volatile StatusControl;
};

struct videoout_t volatile *const vp = ((struct videoout_t *) 0xFF203020);

void swap(int *a, int *b);
int abs(int num);
void plot_pixel(int x, int y, short int line_color);
void clear_screen();
void draw_line(int x0, int y0, int x1, int y1, short int line_color);

int main(void)
{
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    /* Read location of the pixel buffer from the pixel buffer controller */
    pixel_buffer_start = *pixel_ctrl_ptr;

    clear_screen();


    while(1){
		for(int i=0; i<240; i++){
			draw_line(0, i, 319, i, 0x001F); // darw line
			wait_for_vsync();
			draw_line(0, i, 319, i, 0); // clear line
		}
		for(int j=239; j>=0; j--){
			draw_line(0, j, 319, j, 0x001F); // draw line
			wait_for_vsync();
			draw_line(0, j, 319, j, 0); // clear line
		}
	}
}

// code not shown for clear_screen() and draw_line() subroutines
int abs(int num) {
    return (num < 0) ? -num : num;
}

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void plot_pixel(int x, int y, short int line_color)
{
    volatile short int *one_pixel_address;

        one_pixel_address = pixel_buffer_start + (y << 10) + (x << 1);

        *one_pixel_address = line_color;
	
}

void clear_screen(){
    int xn = 320;
    int yn = 240;

    // make all white
    for (int x = 0; x < xn; x++){
        for (int y = 0; y < yn; y++) {
            plot_pixel (x, y, 0x0000);
        }
    }
}

void draw_line(int x0, int y0, int x1, int y1, short int line_color){
    int steep = 0;
	if(abs(y1-y0) > abs(x1-x0)){
		steep = 1;
	}

    int y_step;
    if (steep){ 
        swap(&x0, &y0);
        swap(&x1, &y1);
    }
    if (x0 > x1){
        swap(&x0, &x1);
        swap(&y0, &y1);
    }
    int deltax = x1 - x0;
    int deltay = abs(y1 - y0);
    int error = -(deltax / 2);
    int y = y0;
    if(y0 < y1){
        y_step = 1;
    } else {
        y_step = -1;
    }
    for (int x = x0; x < x1; x++){
        if (steep == 1){
            plot_pixel(y, x, line_color);
        }else{
            plot_pixel(x, y, line_color);
        }
        error = error + deltay;
        if (error > 0){
            y = y + y_step;
            error = error - deltax;
        }
    }
}

void wait_for_vsync(){
    vp->fbp = 1;    
    int status = vp->StatusControl; 
    while ((status & 0x01) != 0){
        status = vp->StatusControl; 
    }
}