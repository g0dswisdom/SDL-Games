#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <math.h>

/*
    Simple ping pong game created in SDL
    Re-used code from my first project (coin collector game)
*/

#define AUDIO "pulseaudio" // You can replace this with the audio software you use like pipewire for example
#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif

enum {
    PLAY,
    EXIT,
    BUTTON_COUNT
};


bool checkCollision(SDL_Rect rectA, SDL_Rect rectB) {
    return (rectA.x < rectB.x + rectB.w &&
            rectA.x + rectA.w > rectB.x &&
            rectA.y < rectB.y + rectB.h &&
            rectA.y + rectA.h > rectB.y);
}

int main() {
    // Init libs
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("Couldnt initialize SDL! %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    if (TTF_Init() != 0) {
        printf("Couldnt initialize SDL text library! %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    if (SDL_AudioInit(AUDIO) != 0) {
        printf("Couldnt initialize SDL audio! %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Ping Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 700, 700, 0);
    Uint32 render_flags = SDL_RENDERER_ACCELERATED;
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, render_flags);

    SDL_AudioSpec audioSpec;
    Uint8* audioStart;
    Uint32 audioLength;
    if (SDL_LoadWAV("./sounds/Countdown.wav", &audioSpec, &audioStart, &audioLength) == NULL) {
        printf("Couldnt load countdown audio file! %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_AudioDeviceID deviceId = SDL_OpenAudioDevice(NULL, 0, &audioSpec, NULL, 0);
    if (deviceId == 0) {
        printf("Couldnt load audio device! %s\n", SDL_GetError());
        SDL_FreeWAV(audioStart);
        SDL_Quit();
        return 1;
    }

    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    TTF_Font* font = TTF_OpenFont("./fonts/JetBrainsMono-Bold.ttf", 28);
    if (font == NULL) {
        printf("Couldnt load font! %s\n", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Color textColor = {0, 0, 0, 255};

    SDL_Surface* readyText = TTF_RenderText_Solid(font, "Ready?", textColor);
    if (readyText == NULL) {
        printf("Couldnt load text! %s\n", TTF_GetError());
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    SDL_Texture* readyTexture = SDL_CreateTextureFromSurface(renderer, readyText);
    SDL_FreeSurface(readyText);
    if (readyTexture == NULL) {
        printf("Couldnt create ready text texture! %s\n", TTF_GetError());
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    int textWidth, textHeight;
    SDL_QueryTexture(readyTexture, NULL, NULL, &textWidth, &textHeight);
    SDL_Rect readyRect = {(windowWidth - textWidth) / 2, 10, textWidth, textHeight}; 

    SDL_Rect buttons[BUTTON_COUNT];
    int buttonX = (windowWidth - 200) / 2;

    buttons[PLAY] = (SDL_Rect){
        buttonX,
        200,
        200,
        50
    };

    buttons[EXIT] = (SDL_Rect) {
        buttonX,
        200 + 50 + 20,
        200,
        50
    };

    int selectedButton = PLAY;
    bool inMenu = true;

    // Main menu
    while (inMenu) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // Keys :p
            switch(event.type) {
                case SDL_QUIT:
                    TTF_Quit();
                    SDL_Quit();
                    exit(1);
                case SDL_KEYDOWN:
                    switch (event.key.keysym.scancode) {
                        case SDL_SCANCODE_UP:
                            selectedButton = (selectedButton - 1 + BUTTON_COUNT) % BUTTON_COUNT;
                            break;
                        case SDL_SCANCODE_DOWN:
                            selectedButton = (selectedButton + 1) % BUTTON_COUNT;
                            break;
                        case SDL_SCANCODE_RETURN:
                            if (selectedButton == PLAY) {
                                inMenu = false;
                            } else if (selectedButton == EXIT) {
                                TTF_Quit();
                                SDL_Quit();
                                return -1;
                            }
                            break;
                    }
                    break;
            }
        }
        // Just rendering and creating stuff (text, buttons)
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        SDL_Surface* playTextSurface = TTF_RenderText_Solid(font, "Play", textColor);
        SDL_Texture* playTextTexture = SDL_CreateTextureFromSurface(renderer, playTextSurface);
        
        SDL_Surface* exitTextSurface = TTF_RenderText_Solid(font, "Exit", textColor);
        SDL_Texture* exitTextTexture = SDL_CreateTextureFromSurface(renderer, exitTextSurface);

        int playTextWidth, playTextHeight, exitTextWidth, exitTextHeight;
        SDL_QueryTexture(playTextTexture, NULL, NULL, &playTextWidth, &playTextHeight);
        SDL_QueryTexture(exitTextTexture, NULL, NULL, &exitTextWidth, &exitTextHeight);

        SDL_Rect playTextRect = {
            buttons[PLAY].x + (buttons[PLAY].w - playTextWidth) / 2,
            buttons[PLAY].y + (buttons[PLAY].h - playTextHeight) / 2,
            playTextWidth,
            playTextHeight
        };

        SDL_Rect exitTextRect = {
            buttons[EXIT].x + (buttons[EXIT].w - exitTextWidth) / 2,
            buttons[EXIT].y + (buttons[EXIT].h - exitTextHeight) / 2,
            exitTextWidth,
            exitTextHeight
        };

        for(int i = 0; i < BUTTON_COUNT; ++i) {
            if (i == selectedButton) {
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
            }
            SDL_RenderFillRect(renderer, &buttons[i]);
        }

        SDL_RenderCopy(renderer, playTextTexture, NULL, &playTextRect);
        SDL_RenderCopy(renderer, exitTextTexture, NULL, &exitTextRect);
        SDL_RenderPresent(renderer);
        SDL_FreeSurface(playTextSurface);
        SDL_FreeSurface(exitTextSurface);
        SDL_DestroyTexture(playTextTexture);
        SDL_DestroyTexture(exitTextTexture);
    }

    // Game
    int quit = 0;
    int speed = 10; // The speed the player's paddle has
    bool countdown = false; // Detects if the countdown audio has been played or not
    bool ballMoving = false;
    SDL_Event event;

    SDL_Rect playerPaddle = {
        50,
        (windowHeight - 90) / 2,
        30,
        100
    };
    
    SDL_Rect enemyPaddle = {
        windowWidth - 80,
        (windowHeight - 90) / 2,
        30,
        100
    };

    SDL_Rect ball = {
        (windowHeight - 50) / 2,
        (windowWidth - 50) / 2,
        20,
        20
    };

    Uint32 prevFrameTime = SDL_GetTicks();
    Uint32 currFrameTime, deltaTime;

    float ballSpeed = 0.05;

    while (!quit) {
        currFrameTime = SDL_GetTicks();
        deltaTime = currFrameTime - prevFrameTime;
        prevFrameTime = currFrameTime;
        while (SDL_PollEvent(&event)) {
            // Paddle movement
            switch(event.type) {
                case SDL_QUIT:
                    quit = 1;
                    break;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.scancode) {
                        case SDL_SCANCODE_UP:
                            playerPaddle.y -= speed;
                            break;
                        case SDL_SCANCODE_DOWN:
                            playerPaddle.y += speed;
                            break; 
                    }
                    break;
            }
        }
        // Countdown audio
        if (!countdown) {
            SDL_QueueAudio(deviceId, audioStart, audioLength);
            SDL_PauseAudioDevice(deviceId, 0);
            countdown = true;
        }

        // Ball movement
        if (countdown && SDL_GetQueuedAudioSize(deviceId) == 0 && !ballMoving) {
            ballMoving = true; // Moves the ball after the countdown is done playing
        }

        bool ballCollisionPlr = checkCollision(playerPaddle, ball);
        bool ballCollisionEnemy = checkCollision(enemyPaddle, ball);

        if (ballMoving) {
            ball.x -= ballSpeed * deltaTime;;

            // Detect if the ball goes out of bounds (shouldn't happen)
            if (ball.x < 0) {
                ball.x = 0;
                ballSpeed *= -1;
            } else if (ball.x > windowWidth - ball.w) {
                ball.x = windowWidth - ball.w;
                ballSpeed *= -1;
            }

            if (ballCollisionPlr) { // This was pain to make
                ballSpeed = fabs(ballSpeed);
                float relativeIntersectY = (playerPaddle.y + playerPaddle.h / 2) - (ball.y + ball.h / 2);
                float relIntersection = (relativeIntersectY / (playerPaddle.h / 2));
                float bounceAngle = relIntersection * (5 * M_PI / 12);

                float ballSpeedX = ballSpeed * cos(bounceAngle);
                float ballSpeedY = ballSpeed * -sin(bounceAngle); 
                ball.x += ballSpeedX * deltaTime;
                ball.y += ballSpeedY * deltaTime;

                if (ball.x < enemyPaddle.x) {
                    ball.x = enemyPaddle.x;
                }
            }
        }

        // Rendering
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, readyTexture, NULL, &readyRect);

        // Render paddles
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 
        SDL_RenderFillRect(renderer, &playerPaddle); 
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 
        SDL_RenderFillRect(renderer, &enemyPaddle); 
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 
        SDL_RenderFillRect(renderer, &ball); 

        SDL_RenderPresent(renderer);
    }

    TTF_CloseFont(font);
    TTF_Quit();
    SDL_CloseAudioDevice(deviceId);
    SDL_FreeWAV(audioStart);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}