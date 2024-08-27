#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <math.h>

#define TFT_CS     10
#define TFT_RST    8
#define TFT_DC     9

#define SCREEN_WIDTH  160
#define SCREEN_HEIGHT 128

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

#define MAP_WIDTH   16
#define MAP_HEIGHT  16

const int worldMap[MAP_WIDTH][MAP_HEIGHT] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};


float posX = 8.0, posY = 8.0;  
float dirX = -1.0, dirY = 0.0; 
float planeX = 0.0, planeY = 1; 

float moveSpeed = 0.5; 
float rotSpeed = 0.3;  

void setup() {
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1); 
    tft.fillScreen(ST7735_BLACK);

    Serial.begin(9600);

    pinMode(7, INPUT_PULLUP); // Backward
    pinMode(5, INPUT_PULLUP); // Forward
    pinMode(3, INPUT_PULLUP); // Turn left
    pinMode(2, INPUT_PULLUP); // Turn right
}

void loop() {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        float cameraX = 2.0 * x / SCREEN_WIDTH - 1.0;
        float rayDirX = dirX + planeX * cameraX;
        float rayDirY = dirY + planeY * cameraX;

        int mapX = int(posX);
        int mapY = int(posY);

        float sideDistX, sideDistY;

        float deltaDistX = sqrt(1 + (rayDirY * rayDirY) / (rayDirX * rayDirX));
        float deltaDistY = sqrt(1 + (rayDirX * rayDirX) / (rayDirY * rayDirY));
        float perpWallDist;

        int stepX, stepY;

        int hit = 0; 
        int side;   

        if (rayDirX < 0) {
            stepX = -1;
            sideDistX = (posX - mapX) * deltaDistX;
        } else {
            stepX = 1;
            sideDistX = (mapX + 1.0 - posX) * deltaDistX;
        }
        if (rayDirY < 0) {
            stepY = -1;
            sideDistY = (posY - mapY) * deltaDistY;
        } else {
            stepY = 1;
            sideDistY = (mapY + 1.0 - posY) * deltaDistY;
        }

        while (hit == 0) {
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            } else {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }
            if (worldMap[mapX][mapY] > 0) {
              hit = 1;
            }
        }

        if (side == 0) {
          perpWallDist = (mapX - posX + (1 - stepX) / 2) / rayDirX;
        }
        else {
          perpWallDist = (mapY - posY + (1 - stepY) / 2) / rayDirY;
        }           

        int lineHeight = (int)(SCREEN_HEIGHT / perpWallDist);

        int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawStart < 0) {
          drawStart = 0; 
        }
        int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawEnd >= SCREEN_HEIGHT) drawEnd = SCREEN_HEIGHT - 1;

        tft.drawFastVLine(x, 0, SCREEN_HEIGHT, ST7735_BLACK);

        uint16_t color = (side == 0) ? ST7735_RED : ST7735_BLUE;

        tft.drawFastVLine(x, drawStart, drawEnd - drawStart + 1, color);
    }

    //player movement
    if (digitalRead(7) == HIGH) { // Move backward
        if(worldMap[int(posX + dirX * moveSpeed)][int(posY)] == 0) posX += dirX * moveSpeed;
        if(worldMap[int(posX)][int(posY + dirY * moveSpeed)] == 0) posY += dirY * moveSpeed;
    }
    if (digitalRead(5) == HIGH) { // Move forward
        if(worldMap[int(posX - dirX * moveSpeed)][int(posY)] == 0) posX -= dirX * moveSpeed;
        if(worldMap[int(posX)][int(posY - dirY * moveSpeed)] == 0) posY -= dirY * moveSpeed;
    }
    if (digitalRead(3) == HIGH) { // Turn left
        float oldDirX = dirX;
        dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
        dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
        float oldPlaneX = planeX;
        planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
        planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
    }
    if (digitalRead(2) == HIGH) { // Turn right
        float oldDirX = dirX;
        dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
        dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
        float oldPlaneX = planeX;
        planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
        planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
    }
}