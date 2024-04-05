volatile int pixel_buffer_start; // global variable
short int Buffer1[240][512]; // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];

void swap(int *a, int *b);
int abs(int num);
void plot_pixel(int x, int y, short int line_color);
void clear_screen();
void draw_line(int x0, int y0, int x1, int y1, short int line_color);
void draw_square(int x, int y, short int line_color);
void wait_for_vsync();

int main(void)
{
   volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
	// declare other variables(not shown)
	// initialize location and direction of rectangles(not shown)
	/* set front pixel buffer to Buffer 1 */
	
	*(pixel_ctrl_ptr + 1) = (int) &Buffer1; // first store the address in theback buffer
	/* now, swap the front/back buffers, to set the front buffer location */
	wait_for_vsync();
	/* initialize a pointer to the pixel buffer, used by drawing functions */
	pixel_buffer_start = *pixel_ctrl_ptr;
	clear_screen(); // pixel_buffer_start points to the pixel buffer
	/* set back pixel buffer to Buffer 2 */
	*(pixel_ctrl_ptr + 1) = (int) &Buffer2;
	pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
	clear_screen(); // pixel_buffer_start points to the pixel buffer

    int x[8];
    int y[8];
    int x_dir[8];
    int y_dir[8];
	int color_box[8];
    int base[10] = {0xFFFF, 0xFFE0,  0xF800,  0x07E0, 0x001F, 0x07FF, 0xF81F, 0xC618, 0xFC18, 0xFC00}; 
    for(int i = 0; i < 8; i++){
        x[i] = rand()%318;
        y[i] = rand()%238;
    }

    for(int i = 0; i < 8; i++){
        x_dir[i] = (rand()%2)*2-1;
        y_dir[i] = (rand()%2)*2-1;
		color_box[i] = base[rand() % 10];
    }

    while (1){
    /* Erase any boxes and lines that were drawn in the last iteration */
    clear_screen();

	
	for(int i = 0; i < 8; i++){
		if(x[i] == 0 || x[i] == 317){
			x_dir[i] = -x_dir[i];
		}
		if(y[i] == 0 || y[i] == 237){
			y_dir[i] = -y_dir[i];
		}
	}
	
    for(int i = 0; i < 8; i++){
        x[i] = x[i] + x_dir[i];
		y[i] = y[i] + y_dir[i];
    }	
	
	for(int i = 0; i < 8; i++){
        draw_square(x[i],y[i],color_box[i]);
		draw_line(x[i], y[i], x[(i+1)%8], y[(i+1)%8], color_box[i]);
    }
    // code for drawing the boxes and lines (not shown)
    // code for updating the locations of boxes (not shown)
    wait_for_vsync(); // swap front and back buffers on VGA vertical sync
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
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

    // make all black
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

void draw_square(int x, int y, short int line_color){
    for(int i = 0; i < 3 ; i++){
        for(int j = 0; j < 3; j++){
              plot_pixel(x+i, y+j, line_color);  
        }
    }
}

void wait_for_vsync(){
    volatile int * pixel_ctrl_ptr = (int *) 0xff203020; 
	int status;
	*pixel_ctrl_ptr = 1;        
	status = *(pixel_ctrl_ptr + 3); 
	while ((status & 0x01) != 0){
        status = *(pixel_ctrl_ptr + 3); 
    }
}