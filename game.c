//create a 2d array as the game grid

// #include <stdio.h>
// #include <stdlib.h>
// #include <stdbool.h>
// #include <time.h>
// #include <unistd.h>

short int grid[320][240];
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

struct Pair
{
    int x;
    int y;
};

int collision(struct Pair a){
    if(grid[a.x][a.y] == 1){
        return 1;
    }
    return 0;
}

//function to load the grid with 0s
void clear_grid(){
    for (int i = 0; i < 320; i++){
        for (int j = 0; j < 240; j++){
            grid[i][j] = 0;
        }
    }
}


struct Pair getPlayerDirectionsFromPS2(){
    unsigned char byte1 = 0;
	unsigned char byte2 = 0;
	unsigned char byte3 = 0;
	
  	volatile int * PS2_ptr = (int *) 0xFF200100;
    int PS2_data, RVALID;
    // while (1) {
		PS2_data = *(PS2_ptr);	// read the Data register in the PS/2 port
		RVALID = (PS2_data & 0x8000);	// extract the RVALID field
		if (RVALID != 0)
		{
			/* always save the last three bytes received */
			byte1 = byte2;
			byte2 = byte3;
			byte3 = PS2_data & 0xFF;
		}
        //check WASD keys
        //W
        if(byte3 == 0x1D){
            return (struct Pair){0, 1};
        //A
        } else if(byte3 == 0x1C){
            return (struct Pair){-1, 0};
        //S
        } else if(byte3 == 0x1B){
            return (struct Pair){0, -1};
        //D
        } else if(byte3 == 0x23){
            return (struct Pair){1, 0};
        }else{
            return (struct Pair){0, 0};
        }

		// if ( (byte2 == 0xAA) && (byte3 == 0x00) )
		// {
		// 	// mouse inserted; initialize sending of data
		// 	*(PS2_ptr) = 0xF4;
		// }
	// }
}

//function to generate a random player at the start of the game, towards the bottom of the screen
struct Pair generate_player(){
    struct Pair player;
    player.x = rand() % 300;
    player.y = 200 + rand() % 20;
    return player;
}

//function to generate a random enemy at the start of the game, towards the top of the screen
struct Pair generate_enemy(){
    struct Pair enemy;
    enemy.x = rand() % 300;
    enemy.y = rand() % 20;
    return enemy;
}

//function to move the player or enemy targets in the direction of the joystick
void movePlayer(struct Pair *player, int x, int y){
    //check if the player is not going out of bounds
    if(player->x + x < 320 && player->x + x > 0){
        player->x += x;
    }
    if(player->y + y < 240 && player->y + y > 0){
        player->y += y;
    }
}

//function to make an enemy shoot randomly by changing the grid values
void shoot(struct Pair *copyOfEnemy){
    int x = copyOfEnemy->x;
    int y = copyOfEnemy->y;
    int direction = rand() % 4;
    if(direction == 1){
        x = x + 1;
    } else if(direction == 2){
        x = x - 1;
    } else if(direction == 3){
        y = y + 1;
    } else if(direction == 4){
        y = y - 1;
    }
    if (x < 320 && x > 0 && y < 240 && y > 0){
        grid[x][y] = 1;
    }
}


int main(void){
    //code for setting up the VGA display
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
    int base[10] = {0xFFFF, 0xFFE0,  0xF800,  0x07E0, 0x001F, 0x07FF, 0xF81F, 0xC618, 0xFC18, 0xFC00};

    
    int playerHealth = 3;
    clear_grid();
    struct Pair player = generate_player();
    struct Pair enemy1 = generate_player();
    //check if the second enemy is not on the same spot as the first enemy
    struct Pair enemy2 = generate_player();
    while(enemy2.x == enemy1.x && enemy2.y == enemy1.y){
        enemy2 = generate_player();
    }
 


    while(playerHealth > 0){
        clear_screen();
        draw_square(player.x, player.y, 0xFFFF);
        draw_square(enemy1.x, enemy1.y, 0xF800);
        draw_square(enemy2.x, enemy2.y, 0xF800);
        struct Pair playerDirections = getPlayerDirectionsFromPS2();
        movePlayer(&player, playerDirections.x, playerDirections.y);
        shoot(&enemy1);
        
        if(collision(player) == 1){
            playerHealth--;
        }
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    }

}


//functions to draw the player and enemy targets
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

//function to clear the screen based on given position
void clear_screen(struct Pair position, int width, int height){
    for(int i = 0; i < width; i++){
        for(int j = 0; j < height; j++){
            plot_pixel(position.x + i, position.y + j, 0x0000);
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
    for(int i = 0; i < 8 ; i++){
        for(int j = 0; j < 8; j++){
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
    



