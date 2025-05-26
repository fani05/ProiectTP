//Librariile incluse
#include "raylib.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

//Toate definurile - am setat dimensiunea ecranului la 1280x1024 deci toate imaginile de background trebuie sa aiba aceasta dimensiune

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 1024
#define GROUND_Y (SCREEN_HEIGHT - 360)
#define GROUND_THICKNESS 15
#define DINO_WIDTH 120
#define DINO_HEIGHT 120
#define GRAVITY 1000
#define JUMP_SPEED -600
#define MAX_OBSTACLES 3

//enumeratile petnru obstacole, ecrane si powerupuri - sa nu uit de iteratii ca trebuie updatate

typedef enum { MENU, GAME, SHOP, GAME_OVER } GameScreen;
typedef enum { TREE1, TREE2, BIRD } ObstacleType;
typedef enum { COIN, JETPACK, INVINCIBLE, SLOWTIME } PowerupType;

//structura pentru obiecte

typedef struct {
    ObstacleType type;
    Vector2 position;
    float scale;
    Rectangle hitbox;
    bool passed;
    bool active;
} Obstacle;

//structura pentru poweups

typedef struct {
    PowerupType type;
    Vector2 position;
    float scale;
    Rectangle hitbox;
    bool active;
    bool collected;
} Powerup;

//main

int main(void)
{
    //Deschidere fereastra si incarcare backgrounduri
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Dino Run");

    Texture2D background1 = LoadTexture("assets/background1.png");
    Texture2D background2 = LoadTexture("assets/background2.png");
    
    // Incarcare imagini pentru skinuri
    Texture2D dinoTextures[5];
    dinoTextures[0] = LoadTexture("assets/dino.png");
    dinoTextures[1] = LoadTexture("assets/dino2.png");
    dinoTextures[2] = LoadTexture("assets/dino3.png");
    dinoTextures[3] = LoadTexture("assets/dino4.png");
    dinoTextures[4] = LoadTexture("assets/dino5.png");
    
    //Incarcare imagini pentru obstacole
    
    Texture2D tree1Texture = LoadTexture("assets/tree1.png");
    Texture2D tree2Texture = LoadTexture("assets/tree2.png");
    Texture2D birdTexture = LoadTexture("assets/bird.png");
    Texture2D menuBackground = LoadTexture("assets/menu_background.png");
    Texture2D shopBackground = LoadTexture("assets/shop.png");
    
    // Incarcare imagini pentru powerups
    Texture2D coinTexture = LoadTexture("assets/coin.png");
    Texture2D jetpackTexture = LoadTexture("assets/jetpack.png");
    Texture2D invincibleTexture = LoadTexture("assets/invincible.png");
    Texture2D clockTexture = LoadTexture("assets/clock.png");

    //Alternare fundal si linie de pamant pentru fiecare 15 secunde
    
    float switchTimer = 0.0f;
    const float switchInterval = 15.0f;
    float fadeTimer = 0.0f;
    const float fadeDuration = 1.5f;
    int currentBackground = 0;
    bool inTransition = false;
    Color groundColor = DARKGREEN;
    Color nextGroundColor = PURPLE;
    float groundFadeAlpha = 0.0f;

    //initializam jocul in meniu
    
    GameScreen currentScreen = MENU;
    int selectedSkin = 0;//selectam automat primul skin
    int coins = 0;
    bool skinUnlocked[5] = {true, false, false, false, false}; // setare skinuri initiala
    int skinPrices[5] = {0, 5, 25, 50, 9999}; // preturi pentru skinuri
    
    //Ajustare pozitie imagine dinozaur si initalizare

    Rectangle dino = { 200, GROUND_Y - DINO_HEIGHT + 26, DINO_WIDTH, DINO_HEIGHT };
    float velocityY = 0;
    bool isJumping = false;
    bool isCrouching = false;
    
    //Initializare obstacole

    Obstacle obstacles[MAX_OBSTACLES];
    Powerup powerups[MAX_OBSTACLES];
    float baseObstacleSpeed = 300;
    float obstacleSpeed = 300;
    const float maxSpeedMultiplier = 2.5f;
    const float obstacleDistance = 800.0f; // distanta fixa intre obstacole

    // Powerups 
    bool jetpackActive = false;
    int jetpackObstaclesLeft = 0;
    bool jetpackJustEnded = false;
    bool invincibleActive = false;
    int invincibleObstaclesLeft = 0;
    bool slowTimeActive = false;
    float slowTimeTimer = 0.0f;
    float timeMultiplier = 1.0f;

    int score = 0;
    int gameScore = 0; // Score jocul in desfasurare
    int highScore = 0;
    int obstaclesPassed = 0; //numarul de obstacole trecute

    SetTargetFPS(60);

    // Initialize obstacles - doar unul activ la inceput
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        obstacles[i].type = (i % 2 == 0) ? TREE1 : TREE2;
        obstacles[i].position = (Vector2){ SCREEN_WIDTH + i * obstacleDistance, GROUND_Y - 120 + 26 };
        obstacles[i].scale = 1.0f;
        obstacles[i].hitbox = (Rectangle){ 0 };
        obstacles[i].passed = false;
        obstacles[i].active = (i == 0); // doar primul obstacol e activ
    }

    // Initializare powerups
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        powerups[i].type = COIN;
        powerups[i].position = (Vector2){ 0, 0 };
        powerups[i].scale = 0.05f; // dimensiune pentru texturi
        powerups[i].hitbox = (Rectangle){ 0 };
        powerups[i].active = false;
        powerups[i].collected = false;
    }
    
    //while loop principal
    
    while (!WindowShouldClose()) {
        float dt = GetFrameTime() * timeMultiplier;

        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        //Menu
        
        if (currentScreen == MENU) {
            DrawTexture(menuBackground, 0, 0, WHITE);
            DrawText("DINO RUN", SCREEN_WIDTH / 2 - 285, 100, 120, BLACK);

            Rectangle playBtn = { SCREEN_WIDTH / 2 - 100, 300, 200, 60 };
            Rectangle shopBtn = { SCREEN_WIDTH / 2 - 100, 400, 200, 60 };
            Rectangle quitBtn = { SCREEN_WIDTH / 2 - 100, 500, 200, 60 };

            DrawRectangleRec(playBtn, RED);
            DrawText("PLAY", playBtn.x + 60, playBtn.y + 15, 30, WHITE);

            DrawRectangleRec(shopBtn, BLUE);
            DrawText("SHOP", shopBtn.x + 60, shopBtn.y + 15, 30, WHITE);

            DrawRectangleRec(quitBtn, GREEN);
            DrawText("QUIT", quitBtn.x + 60, quitBtn.y + 15, 30, WHITE);

            DrawText(TextFormat("Coins: %d", coins), 20, 20, 30, GOLD);
            
            //logica mouse
            
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mouse = GetMousePosition();
                if (CheckCollisionPointRec(mouse, playBtn)) {
                    currentScreen = GAME;
                    currentBackground = 0;
                    switchTimer = 0.0f;
                    fadeTimer = 0.0f;
                    inTransition = false;
                    groundColor = DARKGREEN;
                    nextGroundColor = PURPLE;
                    groundFadeAlpha = 0.0f;
                    gameScore = 0;
                    obstacleSpeed = baseObstacleSpeed;
                    obstaclesPassed = 0;
                    
                    // Reset obstacles - doar primul activ
                    for (int i = 0; i < MAX_OBSTACLES; i++) {
                        obstacles[i].position.x = SCREEN_WIDTH + i * obstacleDistance;
                        obstacles[i].passed = false;
                        obstacles[i].active = (i == 0);
                    }
                    
                    // Resetare powerups la un nou joc
                    jetpackActive = false;
                    jetpackObstaclesLeft = 0;
                    jetpackJustEnded = false;
                    invincibleActive = false;
                    invincibleObstaclesLeft = 0;
                    slowTimeActive = false;
                    slowTimeTimer = 0.0f;
                    timeMultiplier = 1.0f;
                    
                    for (int i = 0; i < MAX_OBSTACLES; i++) {
                        powerups[i].active = false;
                        powerups[i].collected = false;
                    }
                }
                if (CheckCollisionPointRec(mouse, shopBtn)) currentScreen = SHOP;
                if (CheckCollisionPointRec(mouse, quitBtn)) break;
            }
        }
        else if (currentScreen == GAME) {
            // power up pentru slow time
            if (slowTimeActive) {
                slowTimeTimer -= GetFrameTime();
                if (slowTimeTimer <= 0) {
                    slowTimeActive = false;
                    timeMultiplier = 1.0f;
                }
            }

            switchTimer += dt;
            if (!inTransition && switchTimer >= switchInterval) {
                inTransition = true;
                fadeTimer = 0.0f;
                groundFadeAlpha = 0.0f;
            }
            if (inTransition) {
                fadeTimer += dt;
                groundFadeAlpha += dt / fadeDuration;
                if (fadeTimer >= fadeDuration) {
                    currentBackground = !currentBackground;
                    inTransition = false;
                    switchTimer = 0.0f;
                    groundColor = nextGroundColor;
                    nextGroundColor = (groundColor.r == DARKGREEN.r) ? PURPLE : DARKGREEN;
                    groundFadeAlpha = 0.0f;
                }
            }

            float alpha = inTransition ? fadeTimer / fadeDuration : 0.0f;
            if (alpha > 1.0f) alpha = 1.0f;
            Color fade1 = WHITE;
            Color fade2 = WHITE;
            fade1.a = (unsigned char)(255 * (1.0f - alpha));
            fade2.a = (unsigned char)(255 * alpha);

            DrawTexture(currentBackground == 0 ? background1 : background2, 0, 0, fade1);
            DrawTexture(currentBackground == 0 ? background2 : background1, 0, 0, fade2);

            Color blendedColor = Fade(groundColor, 1.0f - groundFadeAlpha);
            Color nextBlendedColor = Fade(nextGroundColor, groundFadeAlpha);
            DrawRectangle(0, GROUND_Y, SCREEN_WIDTH, GROUND_THICKNESS, blendedColor);
            DrawRectangle(0, GROUND_Y, SCREEN_WIDTH, GROUND_THICKNESS, nextBlendedColor);

            // Jetpack
            if (jetpackActive) {
                dino.y = GROUND_Y - DINO_HEIGHT - 200; // inaltime de zbor - de coretat sa treaca peste pasari
                velocityY = 0;
                isJumping = false;
            } else {
                // Sa modific sa te arunce jetoacku inapoi pe pamant
                if (jetpackJustEnded && dino.y < GROUND_Y - DINO_HEIGHT + 26) {
                    isJumping = true;
                    velocityY = 0; 
                    jetpackJustEnded = false;
                }
                
                if (IsKeyPressed(KEY_SPACE) && !isJumping && !isCrouching) {
                    velocityY = JUMP_SPEED;
                    isJumping = true;
                }
            }

            if (IsKeyDown(KEY_LEFT_CONTROL) && !isJumping && !jetpackActive) {
                isCrouching = true;
                dino.width = DINO_HEIGHT;
                dino.height = DINO_WIDTH;
                dino.y = GROUND_Y - dino.height + 26;
            } else if (isCrouching) {
                isCrouching = false;
                dino.width = DINO_WIDTH;
                dino.height = DINO_HEIGHT;
                dino.y = GROUND_Y - dino.height + 26;
            }

            if (isJumping && !jetpackActive) {
                dino.y += velocityY * dt;
                velocityY += GRAVITY * dt;
                if (dino.y >= GROUND_Y - dino.height + 26) {
                    dino.y = GROUND_Y - dino.height + 26;
                    velocityY = 0;
                    isJumping = false;
                }
            }

            // Hitbox dinozaur
            Rectangle dinoHitbox;
            if (isCrouching) {
                dinoHitbox = (Rectangle){ dino.x + 30, dino.y + 60, 80, 60 };
            } else {
                dinoHitbox = (Rectangle){ dino.x + 30, dino.y + dino.height - 100, 60, 80 };
            }
            
            

            // Update obstacole
            for (int i = 0; i < MAX_OBSTACLES; i++) {
                if (!obstacles[i].active) continue;

                obstacles[i].position.x -= obstacleSpeed * dt;

                // Cand un obstacol iese din ecran, spawneaza urmatorul
                if (obstacles[i].position.x < -150) {
                    obstacles[i].active = false;
                    
                    // Gaseste urmatorul obstacol inactiv
                    for (int j = 0; j < MAX_OBSTACLES; j++) {
                        if (!obstacles[j].active) {
                            int rand = GetRandomValue(1, 100);
                            if (rand <= 70) obstacles[j].type = TREE1;
                            else if (rand <= 90) obstacles[j].type = TREE2;
                            else obstacles[j].type = BIRD;

                            obstacles[j].position.x = SCREEN_WIDTH + GetRandomValue(100, 300);
                            obstacles[j].position.y = (obstacles[j].type == BIRD) ? GROUND_Y - 150 : GROUND_Y - 120 + 26;
                            obstacles[j].passed = false;
                            obstacles[j].active = true;
                            break;
                        }
                    }
                }

                Texture2D tex;
                switch (obstacles[i].type) {
                    case TREE1:
                        obstacles[i].scale = 120.0f / tree1Texture.height;
                        tex = tree1Texture;
                        break;
                    case TREE2:
                        obstacles[i].scale = 120.0f / tree2Texture.height;
                        tex = tree2Texture;
                        break;
                    case BIRD:
                        obstacles[i].scale = 120.0f / birdTexture.height;
                        tex = birdTexture;
                        break;
                }

                Vector2 obstacleDrawPos = obstacles[i].position;
                if (obstacles[i].type == TREE2) {
                    obstacleDrawPos.y -= 15; 
                }
                DrawTextureEx(tex, obstacleDrawPos, 0.0f, obstacles[i].scale, WHITE);

                switch (obstacles[i].type) {
                    case TREE1:
                        obstacles[i].hitbox = (Rectangle){ obstacles[i].position.x + 25, obstacles[i].position.y + 20, 70, 90 };
                        break;
                    case TREE2:
                        obstacles[i].hitbox = (Rectangle){ obstacles[i].position.x + 20, obstacles[i].position.y - 5, 80, 100 };
                        break;
                    case BIRD:
                        obstacles[i].hitbox = (Rectangle){ obstacles[i].position.x + 35, obstacles[i].position.y + 0, 100, 100 };
                        break;
                }

                
                
                // Logica coliziuni
                if (!invincibleActive && CheckCollisionRecs(dinoHitbox, obstacles[i].hitbox)) {
                    if (gameScore > highScore) highScore = gameScore;
                    currentScreen = GAME_OVER;
                } else {
                    // Score
                    if (!obstacles[i].passed && dino.x > obstacles[i].position.x + obstacles[i].hitbox.width) {
                        gameScore++;
                        obstaclesPassed++;
                        float speedMultiplier = 1.0f + (gameScore / 20.0f);
                        if (speedMultiplier > maxSpeedMultiplier) speedMultiplier = maxSpeedMultiplier;
                        obstacleSpeed = baseObstacleSpeed * speedMultiplier;
                        obstacles[i].passed = true;
                        
                        
                        if (GetRandomValue(1, 100) <= 60) { // 60% șansă să apară un powerup
                            for (int j = 0; j < MAX_OBSTACLES; j++) {
                            if (!powerups[j].active) {
                            powerups[j].active = true;
                            powerups[j].collected = false;

                            int typeChance = GetRandomValue(1, 100);
                            if (typeChance <= 70) powerups[j].type = COIN;
                            else if (typeChance <= 80) powerups[j].type = INVINCIBLE;
                            else if (typeChance <= 90) powerups[j].type = JETPACK;
                            else powerups[j].type = SLOWTIME;

                            powerups[j].position.x = obstacles[i].position.x + obstacles[i].hitbox.width + 800;
                            powerups[j].position.y = GROUND_Y - 60;
                            break;
                        }
                    }
                }

                        
                        // Jetpack - numaratoare pentru a se termina
                        if (jetpackActive) {
                            jetpackObstaclesLeft--;
                            if (jetpackObstaclesLeft <= 0) {
                                jetpackActive = false;
                                jetpackJustEnded = true; 
                            }
                        }
                        
                        // Invincible - numaratoare pentru a se termina
                        if (invincibleActive) {
                            invincibleObstaclesLeft--;
                            if (invincibleObstaclesLeft <= 0) {
                                invincibleActive = false;
                            }
                        }
                    }
                }
            }

            for (int i = 0; i < MAX_OBSTACLES; i++) {
                if (powerups[i].active && !powerups[i].collected) {
                    powerups[i].position.x -= obstacleSpeed * dt;

                    if (powerups[i].position.x < -100) {
                        powerups[i].active = false;
                    }

                    
                    Texture2D powerupTexture;
                    switch (powerups[i].type) {
                        case COIN:
                            powerupTexture = coinTexture;
                            break;
                        case JETPACK:
                            powerupTexture = jetpackTexture;
                            break;
                        case INVINCIBLE:
                            powerupTexture = invincibleTexture;
                            break;
                        case SLOWTIME:
                            powerupTexture = clockTexture;
                            break;
                    }
                    
                    
                    DrawTextureEx(powerupTexture, powerups[i].position, 0.0f, powerups[i].scale, WHITE);
                    
                    // calculam dimensiunea hitbox pentru powrups
                    int scaledWidth = (int)(powerupTexture.width * powerups[i].scale);
                    int scaledHeight = (int)(powerupTexture.height * powerups[i].scale);
                    powerups[i].hitbox = (Rectangle){ 
                        powerups[i].position.x, 
                        powerups[i].position.y, 
                        scaledWidth, 
                        scaledHeight 
                    };
                    
                 
                    
                    if (CheckCollisionRecs(dinoHitbox, powerups[i].hitbox)) {
                        powerups[i].collected = true;
                        powerups[i].active = false;
                        
                        switch (powerups[i].type) {
                            case COIN:
                                coins++;
                                break;
                            case JETPACK:
                                jetpackActive = true;
                                jetpackObstaclesLeft = 3;
                                jetpackJustEnded = false; 
                                break;
                            case INVINCIBLE:
                                invincibleActive = true;
                                invincibleObstaclesLeft = 3;
                                break;
                            case SLOWTIME:
                                slowTimeActive = true;
                                slowTimeTimer = 10.0f;
                                timeMultiplier = 0.5f;
                                break;
                        }
                    }
                }
            }

            // Dinozaur
            Color dinoColor = WHITE;
            if (invincibleActive) dinoColor = BLUE;
            if (jetpackActive) dinoColor = YELLOW;
            
            DrawTextureEx(dinoTextures[selectedSkin], 
                (Vector2){dino.x + (isCrouching ? 120 : 0), dino.y + (isCrouching ? 20 : 0)}, 
                90.0f * isCrouching, 
                (float)dino.width / dinoTextures[selectedSkin].width, 
                dinoColor);

            // UI
            Color scoreColor = (currentBackground == 1) ? WHITE : BLACK;
            DrawText(TextFormat("Score: %d", gameScore), 20, 20, 30, scoreColor);
            DrawText(TextFormat("Highscore: %d", highScore), 20, 60, 30, DARKGRAY);
            DrawText(TextFormat("Coins: %d", coins), 20, 100, 30, GOLD);
            
            // Afisare status powerups
            if (jetpackActive) {
                DrawText(TextFormat("Jetpack: %d obstacles left", jetpackObstaclesLeft), 20, 140, 20, YELLOW);
            }
            if (invincibleActive) {
                DrawText(TextFormat("Invincible: %d obstacles left", invincibleObstaclesLeft), 20, 160, 20, BLUE);
            }
            if (slowTimeActive) {
                DrawText(TextFormat("Slow Time: %.1fs left", slowTimeTimer), 20, 180, 20, GREEN);
            }
        }//desenare imagine de final joc
        else if (currentScreen == GAME_OVER) {
            ClearBackground((Color){139, 0, 0, 255});
            DrawText("GAME OVER", SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT/2 - 60, 60, RED);
            DrawText(TextFormat("SCORE: %d", gameScore), SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2, 40, BLACK);
            DrawText("Apasa R pentru restart sau Q pentru meniu", SCREEN_WIDTH/2 - 350, SCREEN_HEIGHT/2 + 60, 30, WHITE);

            if (IsKeyPressed(KEY_Q)) currentScreen = MENU;
            if (IsKeyPressed(KEY_R)) {
                dino = (Rectangle){ 200, GROUND_Y - DINO_HEIGHT + 26, DINO_WIDTH, DINO_HEIGHT };
                isJumping = false;
                isCrouching = false;
                velocityY = 0;
                gameScore = 0;
                obstacleSpeed = baseObstacleSpeed;
                obstaclesPassed = 0;
                
                // resetare background la joc nou
                currentBackground = 0;
                switchTimer = 0.0f;
                fadeTimer = 0.0f;
                inTransition = false;
                groundColor = DARKGREEN;
                nextGroundColor = PURPLE;
                groundFadeAlpha = 0.0f;
                
                // resetare powerups la joc nou
                jetpackActive = false;
                jetpackObstaclesLeft = 0;
                jetpackJustEnded = false;
                invincibleActive = false;
                invincibleObstaclesLeft = 0;
                slowTimeActive = false;
                slowTimeTimer = 0.0f;
                timeMultiplier = 1.0f;
                
                for (int i = 0; i < MAX_OBSTACLES; i++) {
                    obstacles[i].position.x = SCREEN_WIDTH + i * obstacleDistance;
                    obstacles[i].passed = false;
                    obstacles[i].active = (i == 0);
                    powerups[i].active = false;
                    powerups[i].collected = false;
                }
                currentScreen = GAME;
            }
        }
        else if (currentScreen == SHOP) {
             DrawTexture(shopBackground, 0, 0, WHITE);
            
            DrawText("SHOP", SCREEN_WIDTH/2 - 100, 50, 60, WHITE);
            DrawText(TextFormat("Coins: %d", coins), 20, 20, 30, GOLD);
            
            // skinuri
            for (int i = 0; i < 5; i++) {
                int x = 100 + i * 220;
                int y = 200;
                Rectangle skinRect = {x, y, 180, 180};
                
                Color skinColor = LIGHTGRAY;
                if (skinUnlocked[i]) {
                    skinColor = (selectedSkin == i) ? GREEN : WHITE;
                } else {
                    skinColor = GRAY;
                }
                
                DrawRectangleRec(skinRect, skinColor);
                DrawRectangleLinesEx(skinRect, 3, BLACK);
                DrawTextureEx(dinoTextures[i], (Vector2){x + 30, y + 30}, 0, 120.0f / dinoTextures[i].width, WHITE);
                
                if (!skinUnlocked[i]) {
                    DrawText(TextFormat("%d coins", skinPrices[i]), x + 40, y + 190, 28, RED);
                    if (coins < skinPrices[i]) {
                        DrawText("Need more coins!", x + 10, y + 220, 20, (Color){ 200, 0, 0, 255 });
                    }
                } else if (selectedSkin == i) {
                    DrawText("SELECTED", x + 12, y + 190, 28, GREEN);
                } else {
                    DrawText("UNLOCKED", x + 12, y + 190, 28, BLUE);
                }
                
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mouse = GetMousePosition();
                    if (CheckCollisionPointRec(mouse, skinRect)) {
                        if (skinUnlocked[i]) {
                            selectedSkin = i;
                        } else if (coins >= skinPrices[i]) {
                            coins -= skinPrices[i];
                            skinUnlocked[i] = true;
                            selectedSkin = i;
                        }
                    }
                }
            }
            
            DrawText("Apasa Q pentru inapoi", 480, 460, 30, WHITE);
            if (IsKeyPressed(KEY_Q)) currentScreen = MENU;
        }

        EndDrawing();
    }

    // Unload everything
    UnloadTexture(background1);
    UnloadTexture(background2);
    for (int i = 0; i < 5; i++) {
        UnloadTexture(dinoTextures[i]);
    }
    UnloadTexture(tree1Texture);
    UnloadTexture(tree2Texture);
    UnloadTexture(birdTexture);
    UnloadTexture(menuBackground);
    UnloadTexture(coinTexture);
    UnloadTexture(jetpackTexture);
    UnloadTexture(invincibleTexture);
    UnloadTexture(clockTexture);
    CloseWindow();

    return 0;
}