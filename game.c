//create a 2d array as the game grid

#include <stdio.h>

volatile int pixel_buffer_start; // global variable
short int Buffer1[240][512]; // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];

//define bullet direction (down, left, right)
#define DOWN 1
#define LEFTDOWN 2
#define RIGHTDOWN 3
#define MORELEFTDOWN 4
#define MORERIGHTDOWN 5

#define RIGHT 1
#define LEFT -1



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

struct Bullet
{
    int x;
    int y;
    int x_direction;
    int y_direction;
    int chase;
};

//function to check if the player has collided with an enemy bullet in the bullet array
int collision(struct Pair player, struct Bullet Bulletarray1[], struct Bullet Bulletarray2[], int count){
    for(int i = 0; i < count; i++){
        //check all 8 sides of the player if the coordinates are the same as the bullet
        if(Bulletarray1[i].x == player.x && Bulletarray1[i].y == player.y || Bulletarray1[i].x == player.x+1 && Bulletarray1[i].y == player.y+1
        || Bulletarray1[i].x == player.x && Bulletarray1[i].y == player.y+1 || Bulletarray1[i].x == player.x+1 && Bulletarray1[i].y == player.y
        || Bulletarray1[i].x == player.x && Bulletarray1[i].y == player.y-1 || Bulletarray1[i].x == player.x-1 && Bulletarray1[i].y == player.y
        || Bulletarray1[i].x == player.x-1 && Bulletarray1[i].y == player.y-1 || Bulletarray1[i].x == player.x-1 && Bulletarray1[i].y == player.y+1
        || Bulletarray1[i].x == player.x+1 && Bulletarray1[i].y == player.y-1){
            return 1;
        }
        if(Bulletarray2[i].x == player.x && Bulletarray2[i].y == player.y || Bulletarray2[i].x == player.x+1 && Bulletarray2[i].y == player.y+1
        || Bulletarray2[i].x == player.x && Bulletarray2[i].y == player.y+1 || Bulletarray2[i].x == player.x+1 && Bulletarray2[i].y == player.y
        || Bulletarray2[i].x == player.x && Bulletarray2[i].y == player.y-1 || Bulletarray2[i].x == player.x-1 && Bulletarray2[i].y == player.y
        || Bulletarray2[i].x == player.x-1 && Bulletarray2[i].y == player.y-1 || Bulletarray2[i].x == player.x-1 && Bulletarray2[i].y == player.y+1
        || Bulletarray2[i].x == player.x+1 && Bulletarray2[i].y == player.y-1){
            return 1;
        }

    }
    return 0;
}


//function to generate a random player at the start of the game, towards the bottom of the screen
struct Pair generate_player(){
    struct Pair player;
    player.x = rand() % 150;
    player.y = 200 + rand() % 20;
    return player;
}

//function to generate a random enemy at the start of the game, towards the top of the screen
struct Pair generate_enemy(){
    struct Pair enemy;
    enemy.x = rand() % 150;
    enemy.y = rand() % 20;
    return enemy;
}

//function to move the player or enemy targets in the direction of the joystick
void movePlayer(struct Pair *player, int x, int y){
    //check if the player is not going out of bounds
    if(player->x + x < 155 && player->x + x > 5){
        player->x += x;
    }
    if(player->y + y < 235 && player->y + y > 5){
        player->y += y;
    }
}

//function to find the length of the bullet array
int findBulletLength(struct Bullet Bulletarray[]){
    int i = 0;
    int count = 0;
    while (i<=999){
        if (Bulletarray[i].x != 0){
            count++;
        }
        i++;
    }
    return count;
}

//function to make an ememy generate a bullet in one of the three directions, store the bullet in the bullet array
void generateBullet(struct Pair enemy, struct Bullet Bulletarray[], int x_direction, int y_direction, int chase){
    int i = 0;
    while (Bulletarray[i].x != 0){
        i++;
    }
    Bulletarray[i] = (struct Bullet){enemy.x, enemy.y, x_direction, y_direction, chase};
}


void moveBulletInDirection(struct Bullet *bullet, struct Pair player){
    //clear the bullet if its out of bounds
    if(bullet->y > 235 || bullet->y < 5 || bullet->x >= 155 || bullet->x < 5){
        bullet->x = 0;
        bullet->y = 0;
    }
    if(bullet->x != 0 || bullet->y != 0){
        if(bullet->chase == 0){
            bullet->x += bullet->x_direction;
            bullet->y += bullet->y_direction;
        }else{
            int t = abs(bullet->y - player.y) / bullet->y_direction;
            int temp = ((player.x - bullet->x)/t);
            while (abs(temp) >= 3){
                temp = temp/2;
            }
            bullet->x += temp;
            bullet->y += bullet->y_direction;
        }
        
    }
    
}

int main(void){
    struct Bullet Bulletarray1[1000];
    struct Bullet Bulletarray2[1000];
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
    struct Pair enemy1 = (struct Pair){5, 15};
    //check if the second enemy is not on the same spot as the first enemy
    struct Pair enemy2 = (struct Pair){145, 15};
    int timeCounter = 0;
    struct Pair playerDirections;
    
    //code to move the player
    unsigned char byte1 = 0;
    unsigned char byte2 = 0;
    unsigned char byte3 = 0;
    volatile int *PS2_ptr = (int *)0xFF200100;
    int PS2_data, RVALID;
 


    while(playerHealth > 0){

        playerDirections = (struct Pair){0, 0};

        //Initialize
        reset_screen();
        draw_square(player.x, player.y, 0xFFFF);
        draw_square(enemy1.x, enemy1.y, 0xF800);
        draw_square(enemy2.x, enemy2.y, 0xF800);

        //code to generate bullet in one of the five directions based on a random number
        int random1 = rand() % 4; 
        int random2 = rand() % 3;
        int random3 = rand() % 4; 
        int random4 = rand() % 3;
        int random5 = rand() % 4;

        int total = findBulletLength(Bulletarray1) + findBulletLength(Bulletarray2);
        int random6 = rand() % 10;

        int chase = 0;
        if(random5 == 0){
            chase = 1;
        }


        if(random6 <= 5 - (total/400)*2){
            generateBullet(enemy1, Bulletarray1, random1, random2 + 1, chase);
            generateBullet(enemy2, Bulletarray2, -random3, random4 + 1 , chase);
        }
        else if (random6 <= 10 - (total/200)*2){
            int random7 = rand() % 2;
            if (random7 == 0){
                generateBullet(enemy1, Bulletarray1, random1, random2 + 1, chase);
            }else{
                generateBullet(enemy2, Bulletarray2, -random3, random4 + 1, chase);
            }
        }


        //code to move all the bullets in the bullet array
        for(int i = 0; i < 1000; i++){
            if (Bulletarray1[i].x != 0){
                moveBulletInDirection(&Bulletarray1[i], player);
                if(Bulletarray1[i].x != 0){
                    plot_pixel(Bulletarray1[i].x, Bulletarray1[i].y, 0x07E0);
                }
            }


            if (Bulletarray2[i].x != 0){
                moveBulletInDirection(&Bulletarray2[i], player);
                if(Bulletarray2[i].x != 0){
                    plot_pixel(Bulletarray2[i].x, Bulletarray2[i].y, 0x07E0);
                }
            }
        }



        PS2_data = *(PS2_ptr);        // read the Data register in the PS/2 port
        RVALID = (PS2_data & 0x8000); // extract the RVALID field
        if (RVALID != 0){
        /* always save the last three bytes received */
        byte1 = byte2;
        byte2 = byte3;
        byte3 = PS2_data & 0xFF;
        }
        if ( (byte2 == 0xAA) && (byte3 == 0x00) )
		{
			// mouse inserted; initialize sending of data
			*(PS2_ptr) = 0xF4;
		}
    // check WASD keys
    // W
        if(byte2 != 0xF0){
            if (byte3 == 0x1D){
                playerDirections = (struct Pair){0, -5};
            // A
            }
            else if (byte3 == 0x1C){
                playerDirections = (struct Pair){-5, 0};
            // S
            }
            else if (byte3 == 0x1B){
                playerDirections = (struct Pair){0, 5};
            // D
            }
            else if (byte3 == 0x23){
                playerDirections = (struct Pair){5, 0};
            }
        }
        else if(byte2 == 0xF0){
            playerDirections =  (struct Pair){0, 0};
        }
        movePlayer(&player, playerDirections.x, playerDirections.y);
        if(collision(player, Bulletarray1, Bulletarray2, timeCounter) == 1){
            playerHealth--;
        }

        draw_line(160, 0, 160, 240, 0xFFFF);
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);
        timeCounter++;

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
    for(int i = 0; i < 160; i++){
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

//draw a 3x3 square when the current position is the center of the square
void draw_square(int x, int y, short int line_color){
    for(int i = -1; i < 2; i++){
        for(int j = -1; j < 2; j++){
            plot_pixel(x + i, y + j, line_color);
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
    



