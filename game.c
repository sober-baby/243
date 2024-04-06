//create a 2d array as the game grid

#include <stdio.h>

volatile int pixel_buffer_start; // global variable
short int Buffer1[240][512]; // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];

//define bullet direction (down, left, right)
#define DOWN 1
#define LEFTDOWN 2
#define RIGHTDOWN 3


void swap(int *a, int *b);
int abs(int num);
void plot_pixel(int x, int y, short int line_color);
void clear_screen();
void draw_line(int x0, int y0, int x1, int y1, short int line_color);
void draw_square(int x, int y, short int line_color);
void wait_for_vsync();

//struct to represent the player and enemy targets
struct Pair
{
    int x;
    int y;
};

//function to check if the player has collided with an enemy bullet in the bullet array
int collision(struct Pair player, struct Pair Bulletarray[]){
    for(int i = 0; i < 1000; i++){
        //check all 8 sides of the player if the coordinates are the same as the bullet
        if(Bulletarray[i].x == player.x && Bulletarray[i].y == player.y || Bulletarray[i].x == player.x+1 && Bulletarray[i].y == player.y+1
        || Bulletarray[i].x == player.x && Bulletarray[i].y == player.y+1 || Bulletarray[i].x == player.x+1 && Bulletarray[i].y == player.y
        || Bulletarray[i].x == player.x && Bulletarray[i].y == player.y-1 || Bulletarray[i].x == player.x-1 && Bulletarray[i].y == player.y
        || Bulletarray[i].x == player.x-1 && Bulletarray[i].y == player.y-1 || Bulletarray[i].x == player.x-1 && Bulletarray[i].y == player.y+1
        || Bulletarray[i].x == player.x+1 && Bulletarray[i].y == player.y-1){
            return 1;
        }
    }
    return 0;
}

// function to get input from the PS2 controller
struct Pair getPlayerDirectionsFromPS2()
{
    unsigned char byte1 = 0;
    unsigned char byte2 = 0;
    unsigned char byte3 = 0;
    volatile int *PS2_ptr = (int *)0xFF200100;
    int PS2_data, RVALID;
    PS2_data = *(PS2_ptr);        // read the Data register in the PS/2 port
    RVALID = (PS2_data & 0x8000); // extract the RVALID field
    if (RVALID != 0)
    {
        /* always save the last three bytes received */
        byte1 = byte2;
        byte2 = byte3;
        byte3 = PS2_data & 0xFF;
    }
    // check WASD keys
    // W
    if (byte3 == 0x1D)
    {
        return (struct Pair){0, -5};
        // A
    }
    else if (byte3 == 0x1C)
    {
        return (struct Pair){-5, 0};
        // S
    }
    else if (byte3 == 0x1B)
    {
        return (struct Pair){0, 5};
        // D
    }
    else if (byte3 == 0x23)
    {
        return (struct Pair){5, 0};
    }
    else
    {
        return (struct Pair){0, 0};
    }
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


//function to make an ememy generate a bullet in one of the three directions, store the bullet in the bullet array
void generateBullet(struct Pair enemy, struct Pair Bulletarray[], int bulletDirection, int bulletCount){
    if(bulletDirection == DOWN){
        Bulletarray[bulletCount] = (struct Pair){enemy.x, enemy.y-1};
    } else if(bulletDirection == LEFTDOWN){
        Bulletarray[bulletCount] = (struct Pair){enemy.x-1, enemy.y-1};
    } else if(bulletDirection == RIGHTDOWN){
        Bulletarray[bulletCount] = (struct Pair){enemy.x+1, enemy.y-1};
    }
}

void moveBulletInDirection(struct Pair *bullet, int direction){
    if(direction == DOWN){
        bullet->y += 1;
    } else if(direction == LEFTDOWN){
        bullet->x -= 1;
        bullet->y += 1;
    } else if(direction == RIGHTDOWN){
        bullet->x += 1;
        bullet->y += 1;
    }
}

int main(void){
    struct Pair Bulletarray[1000];
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
    reset_screen();
    /* set back pixel buffer to Buffer 2 */
	*(pixel_ctrl_ptr + 1) = (int) &Buffer2;
	pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    reset_screen();  
    int base[10] = {0xFFFF, 0xFFE0,  0xF800,  0x07E0, 0x001F, 0x07FF, 0xF81F, 0xC618, 0xFC18, 0xFC00};
    

    
    int playerHealth = 3;
    struct Pair player = generate_player();
    struct Pair enemy1 = generate_enemy();
    //check if the second enemy is not on the same spot as the first enemy
    struct Pair enemy2 = generate_enemy();
    while(enemy2.x == enemy1.x && enemy2.y == enemy1.y){
        enemy2 = generate_enemy();
    }
    int timeCounter = 0;
 


    while(playerHealth > 0){

        //Initialize
        reset_screen();
        draw_square(player.x, player.y, 0xFFFF);
        draw_square(enemy1.x, enemy1.y, 0xF800);
        draw_square(enemy2.x, enemy2.y, 0xF800);

        //code to generate bullets, first enenmy generates the first bullet occpying 0-499 in the bullet array
        generateBullet(enemy1, Bulletarray, DOWN, timeCounter);
        generateBullet(enemy2, Bulletarray, DOWN, timeCounter+500);
        timeCounter++;

        //code to move all the bullets in the bullet array
        for(int i = 0; i < 1000; i++){
            if(Bulletarray[i].x != 0 && Bulletarray[i].y != 0){
                moveBulletInDirection(&Bulletarray[i], DOWN);
                //draw the bullet
                plot_pixel(Bulletarray[i].x, Bulletarray[i].y, 0x07E0);
            }
        }



        //code to move the player
        struct Pair playerDirections = getPlayerDirectionsFromPS2();
        movePlayer(&player, playerDirections.x, playerDirections.y);
        if(collision(player, Bulletarray) == 1){
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

void reset_screen(){
    for(int i = 0; i < 320; i++){
        for(int j = 0; j < 240; j++){
            plot_pixel(i, j, 0x0000);
        }
    }
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
    



