// Header files for various components and constants
#include "console.h"
#include "page.h"
#include "process.h"
#include "keyboard.h"
#include "mouse.h"
#include "interrupt.h"
#include "clock.h"
#include "ata.h"
#include "device.h"
#include "cdromfs.h"
#include "string.h"
#include "graphics.h"
#include "kernel/ascii.h"
#include "kernel/syscall.h"
#include "rtc.h"
#include "kernelcore.h"
#include "kmalloc.h"
#include "memorylayout.h"
#include "kshell.h"
#include "cdromfs.h"
#include "diskfs.h"
#include "serial.h"


#define FONT_WIDTH 10


// Screen dimensions constants
#define SCREEN_WIDTH 1020
#define SCREEN_HEIGHT 700

// Spaceship dimensions constants
#define SPACESHIP_WIDTH 100
#define SPACESHIP_HEIGHT 60

// Enemy dimensions constants
#define ENEMY_WIDTH 30
#define ENEMY_HEIGHT 20

// Maximum number of enemies and bullets
#define MAX_ENEMIES 100
#define MAX_BULLETS 10000

// Bullet speed and dimensions constants
#define BULLET_SPEED 21
#define BULLET_WIDTH 5
#define BULLET_HEIGHT 7
#define MAX_ACTIVE_ENEMIES 1 


// Global variables
int keyPress = 0;
int score = 0;
int enemySpeed = 15;





// Movement enumeration for spaceship
typedef enum {
    DOWN,
    UP,
    NONE

} Movement;




// Structure for Spaceship
typedef struct {
    int x;
    int y;
    Movement movement;

} Spaceship;



// Structure for Enemy
typedef struct {
    int x;
    int y;
    int isAlive;
    Movement movement;

} Enemy;




Enemy enemies[MAX_ENEMIES];
int numberOfEnemies = 0; // Current number of enemies



// Structure for Bullet
typedef struct {
    int x;
    int y;
    Movement movement;

} Bullet;




Bullet bullets[MAX_BULLETS]; // The array where the bullets are stored
int numberOfBullets = 0; // Number of bullets available

Spaceship spaceship;



// Pseudo-random number generator
int seed = 0xDEADBEEF; // Seed for the random number generator
int random() {
    // Generate a pseudo-random number
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    return seed;
}



// Delay function
void delay(int milliseconds) {

    int i, j;

    for (i = 0; i < milliseconds; i++) {

        for (j = 0; j < 1000000; j++) {

        }
    }
}



// Function to initialize enemies
void enemyInit() {

    for (int i = 0; i < MAX_ACTIVE_ENEMIES; i++) {
        enemies[i].x = SCREEN_WIDTH - ENEMY_WIDTH;
        enemies[i].y = random() % (SCREEN_HEIGHT - ENEMY_HEIGHT);
        enemies[i].isAlive = 1;
        enemies[i].movement = NONE;
    }
}



// Function to check collisions between bullets and enemies
void collisionCheck() {

    for (int i = 0; i < numberOfBullets; i++) {

        int bulletX = bullets[i].x + BULLET_WIDTH;
        int bulletY = bullets[i].y + BULLET_HEIGHT;
        
        for (int j = 0; j < MAX_ENEMIES; j++) {

            if (enemies[j].isAlive) {

                int enemyX = enemies[j].x + ENEMY_WIDTH+1;
                int enemyY = enemies[j].y + ENEMY_HEIGHT;

                if (bullets[i].x < enemyX && bulletX > enemies[j].x && bullets[i].y < enemyY && bulletY > enemies[j].y) {
                    // Collision occurred
                    score += 1;
                    enemies[j].isAlive = 0;

                    
                    enemies[j].y = -ENEMY_HEIGHT; // Move the enemy off-screen
                    numberOfEnemies--;
                    
                    
                    bullets[i] = bullets[numberOfBullets - 1];
                    numberOfBullets--;
                    i--;
                    break;
                }
            }
        }
    }
}



// Function to handle collision between spaceship and enemy
void collisionCheckForPlayer_Enemy(struct graphics *g) {

    int spaceShipX = spaceship.x + SPACESHIP_WIDTH - 5;
    int spaceShipY = spaceship.y + SPACESHIP_HEIGHT - 5;
    
    for (int j = 0; j < MAX_ENEMIES; j++) {

        if (enemies[j].isAlive) {

            int enemyX = enemies[j].x + ENEMY_WIDTH;

            int enemyY = enemies[j].y + ENEMY_HEIGHT;


            if (spaceship.x < enemyX && spaceShipX > enemies[j].x && spaceship.y < enemyY && spaceShipY > enemies[j].y) {
                // If spaceship collides with an enemy
                clearScreen(g);
                

                struct graphics_color red_color = {255, 0, 0, 0};
                graphics_fgcolor(g, red_color);

                for (int i = 0; i < strlen("Game Over"); i++) {
                    graphics_char(g, 450 + i * (FONT_WIDTH * 2), 300, "Game Over"[i]);
                }
                
                while (1) {
                    // Wait for ending.
                }
            }
        }
    }
}



// Function to move enemies
void enemyMove(struct graphics *g) {

    for (int i = 0; i < MAX_ACTIVE_ENEMIES; i++) {

        if (enemies[i].isAlive) {

            enemies[i].x -= enemySpeed;

            if (enemies[i].x < 20) {

                clearScreen(g);
                

                struct graphics_color red_color = {255, 0, 0, 0};

                graphics_fgcolor(g, red_color);

                for (int i = 0; i < strlen("Game Over"); i++) {
                    graphics_char(g, 450 + i * (FONT_WIDTH * 2), 300, "Game Over"[i]);
                }

                
                while (1) {
                    // Wait for ending.
                }

            }
        }
    }
}



// Function to draw enemies
void enemyDraw(struct graphics *g) {

    struct graphics_color blue_color = {0, 0, 255, 0};

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].y >= 0) {
            graphics_fgcolor(g, blue_color);
            graphics_rect(g, enemies[i].x, enemies[i].y, ENEMY_WIDTH, ENEMY_HEIGHT);
        }
    }
}



// Function to spawn new enemies periodically
void spawnNewEnemy() {

    static int spawnTimer = 0;
    static int spawnDelay = 500;
    static int maxEnemies = MAX_ACTIVE_ENEMIES;

    spawnTimer += 15;

    if (spawnTimer >= spawnDelay && numberOfEnemies < maxEnemies) {

        for (int i = 0; i < MAX_ENEMIES; i++) {

            if (!enemies[i].isAlive) {

                enemies[i].x = SCREEN_WIDTH - ENEMY_WIDTH;
                enemies[i].y = random() % (SCREEN_HEIGHT - ENEMY_HEIGHT);
                enemies[i].isAlive = 1;
                enemies[i].movement = NONE;

                numberOfEnemies++;
                break;
            }
        }

        spawnTimer = 0;
    }
}



// Function to initialize spaceship
void spaceshipInit() {

    spaceship.x = (SCREEN_WIDTH - SPACESHIP_WIDTH) / 15;
    spaceship.y = SCREEN_HEIGHT - SPACESHIP_HEIGHT - 350;
    spaceship.movement = NONE;

}





// Function to move spaceship
void spaceshipMove() {

    if (spaceship.movement == DOWN) {

        if (spaceship.y < SCREEN_HEIGHT - SPACESHIP_HEIGHT) {

            spaceship.y += 20;

        }


    } 
    
    else if (spaceship.movement == UP) {

        if (spaceship.y > 0) {
            spaceship.y -= 20;
        }
    }
}


// Function to draw spaceship
void spaceshipDraw(struct graphics *g) {

    struct graphics_color white_color = {255, 255, 255, 0};


    int leftX = spaceship.x;
    int leftY = spaceship.y;
    int rightX = leftX;
    int rightY = leftY;
    
    graphics_fgcolor(g, white_color); 

    for (int i = 0; i < SPACESHIP_HEIGHT - 10; i++) {
        graphics_line(g, rightX - i, rightY - i, 0, 2 * i);
    }
}



// Function to fire a bullet
void bulletFire() {

    if (numberOfBullets < MAX_BULLETS) {

        bullets[numberOfBullets].x = spaceship.x + 5;
        bullets[numberOfBullets].y = spaceship.y - 8;
        bullets[numberOfBullets].movement = NONE;
        numberOfBullets++;
    }
}



// Function to handle input
void inputHandle() {
   
    char key = keyboard_read(1);
	
	if (key == 'W' || key == 'w') {
		spaceship.movement = UP;
		keyPress = 1;
	}
		
    else if (key == 's' || key == 'S') {
		spaceship.movement = DOWN;
		keyPress = 1;
	}
		
    else if (key == ' ') {
		bulletFire();
			
	}
}



// Function to move bullets
void bulletsMove() {

    for (int i = 0; i < numberOfBullets; i++) {

        bullets[i].x += BULLET_SPEED;

        if (bullets[i].x + BULLET_WIDTH >= SCREEN_WIDTH) {

            bullets[i] = bullets[numberOfBullets - 1];
            numberOfBullets--;
            i--;
        }
    }
}


// Function to draw bullets
void bulletsDraw(struct graphics *g) {

    struct graphics_color red_color = {255, 0, 0, 0};

    graphics_fgcolor(g, red_color);

    for (int i = 0; i < numberOfBullets; i++) {
        graphics_rect(g, bullets[i].x, bullets[i].y, BULLET_WIDTH, BULLET_HEIGHT);
    }
}



// Function to clear the screen
void clearScreen(struct graphics *g) {
    
    struct graphics_color black_color = {0, 0, 0, 0};
    graphics_clear(g, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    graphics_fgcolor(g, black_color);
}



// Function to print a string at specified coordinates
void printString(struct graphics *g, int x, int y, char *str) {

    int i = 0;
    while (str[i] != '\0') {
        graphics_char(g, x + i * 8, y, str[i]);
        i++;
    }
}



// Function to print an integer at specified coordinates
void printInt(struct graphics *g, int x, int y, int value) {

    char buffer[12];
    int i = 0;

    if (value == 0) {
        graphics_char(g, x, y, '0');
        return;
    }


    if (value < 0) {
        graphics_char(g, x, y, '-');
        value *= -1;
        x += 8;
    }


    while (value != 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }


    while (i > 0) {
        graphics_char(g, x, y, buffer[--i]);
        x += 8;
    }
}


// Main kernel function
int kernel_main() {
    struct graphics *g = graphics_create_root();

    console_init(g);
    console_addref(&console_root);

    page_init();
    kmalloc_init((char *)KMALLOC_START, KMALLOC_LENGTH);
    interrupt_init();
    rtc_init();
    keyboard_init();
    process_init();

    spaceshipInit();
    enemyInit();

  while (1) {
    clearScreen(g);
    inputHandle();
    
    if (keyPress == 1) {
        spaceshipMove();
        keyPress = 0;
    }
    
    enemyMove(g);
    spaceshipDraw(g);
    enemyDraw(g);
    bulletsDraw(g);
    bulletsMove();
    collisionCheck();
    collisionCheckForPlayer_Enemy(g);

    spawnNewEnemy();

    struct graphics_color cyan_color = {0, 255, 255, 0};
    graphics_fgcolor(g, cyan_color);
    
    // Print scores
    printString(g, 10, 700, "Score: ");
    printInt(g, 70, 700, score);

    delay(15);
}


    return 0;
}
