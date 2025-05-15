#include <iostream>
#include <unordered_set>
#include <chrono>
#include <vector>
#include <Windows.h>
#include "SDL3/SDL.h"
#include "SDL3_image/SDL_image.h"
#include "VectorStructs.h"
#include "Entity.h"

using timePoint = std::chrono::time_point<std::chrono::steady_clock>;

void renderClear(SDL_Renderer* renderer, const int& r, const int& g, const int& b, const int& a)
{
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
	SDL_RenderClear(renderer);
}

int 
main()
//WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}


	timePoint spawnTime = std::chrono::steady_clock::now();
	timePoint currentTime;
	int64_t elapsedTimeMilliseconds;

	SDL_Window* window;
	SDL_Renderer* renderer;

	SDL_Window* popup = nullptr;
	SDL_Renderer* popupRenderer = nullptr;

	SDL_Window* clickedWindow = nullptr;

	bool isRunning = true;

	unsigned int windowWidth = 128;
	unsigned int windowHeight = 128;
	window = SDL_CreateWindow("Cartethyia window",windowWidth, windowHeight,
		 SDL_WINDOW_TRANSPARENT | SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALWAYS_ON_TOP);

	renderer = SDL_CreateRenderer(window, NULL);

	Entity Cartethyia = {"Cartethyia",*window, *renderer, "tex/CartethiyaSpriteMap.png", 4, 2, spawnTime};

	SDL_Surface* exitButtonSurface = nullptr; 
	SDL_Texture* exitButtonTexture = nullptr;

	std::unordered_set <SDL_Keycode> keyBuffer;
	
	bool showPopup = false;

	unsigned int spriteIndex = 0;
	int spriteLoopcount = 0;

	float mouseXPos;
	float mouseYPos;
	SDL_GetGlobalMouseState(&mouseXPos, &mouseYPos);
	float lastMouseXPos = mouseXPos;
	float lastMouseYPos = mouseYPos;
	bool mouseMoved = false;

	while (isRunning)
	{
		currentTime = std::chrono::steady_clock::now();
		SDL_Event event;
		SDL_GetGlobalMouseState(&mouseXPos, &mouseYPos);

		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
			{
				std::cout << "Application quit" << std::endl;
				isRunning = false;
			}
			if (event.type == SDL_EVENT_KEY_DOWN)
			{
				if (!event.key.repeat)
				{
					keyBuffer.insert(event.key.key);
				}
			}
			if (event.type == SDL_EVENT_KEY_UP)
			{
				keyBuffer.erase(event.key.key);
			}

			if (event.button.button == SDL_BUTTON_LEFT && Cartethyia.isHeld && !Cartethyia.isFalling)
			{
				Cartethyia.mouseDrag({ mouseXPos, mouseYPos });
				if (lastMouseXPos > mouseXPos)
				{
					Cartethyia.smoothRotate(-12);
				}
				else if (lastMouseXPos < mouseXPos)
				{
					Cartethyia.smoothRotate(12);
				}
				showPopup = false;
			}
			if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
			{
				if (event.button.button == SDL_BUTTON_RIGHT)
				{
					if (showPopup)
					{
						showPopup = false;
					}
					else if (!showPopup)
					{
						showPopup = true;
					}
				}
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					Cartethyia.isHeld = true;
				}
				clickedWindow = SDL_GetWindowFromID(event.button.windowID);
				if (clickedWindow == popup && event.button.button == SDL_BUTTON_LEFT)
				{
					std::cout << "Application quit" << std::endl;
					isRunning = false;
				}

			}
			if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
			{
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					Cartethyia.isHeld = false;
				}
			}
		}
		

		if (lastMouseXPos!= mouseXPos || lastMouseYPos!= mouseYPos)
		{
			lastMouseXPos = mouseXPos;
			lastMouseYPos = mouseYPos;
		}
		else
		{
			Cartethyia.updateLifetime(currentTime);
			Cartethyia.smoothResetRotate();
		}

		if (Cartethyia.isHeld)
		{
			Cartethyia.sweat(currentTime);
			Cartethyia.flail(currentTime);
			Cartethyia.displayAll();
		}
		else
		{
			if (Cartethyia.getPosition().y < Cartethyia.getGround())
			{
				Cartethyia.isFalling = true;
			}

			if (Cartethyia.isFalling)
			{
				Cartethyia.fall(currentTime);
			}
			Cartethyia.breath(currentTime);
			Cartethyia.displayAll();
		}

		Cartethyia.updateLifetime(currentTime);
		
		if (showPopup && !popup)
		{
			popup = SDL_CreatePopupWindow(window, 100, -10, 16, 16, SDL_WINDOW_POPUP_MENU | SDL_WINDOW_ALWAYS_ON_TOP);
			popupRenderer = SDL_CreateRenderer(popup, NULL);
			exitButtonSurface = SDL_LoadBMP("tex/exitButton.bmp");
			exitButtonTexture = SDL_CreateTextureFromSurface(popupRenderer, exitButtonSurface);
		}
		
		if (!showPopup && popup)
		{
			SDL_DestroyRenderer(popupRenderer);
			SDL_DestroyWindow(popup);
			popup = nullptr;
			popupRenderer = nullptr;
		}

		SDL_FRect srcExit = { 0, 0, 16, 16 };
		SDL_FRect dsExit = { 0, 0, 16, 16 };
		SDL_RenderTexture(popupRenderer, exitButtonTexture, &srcExit, &dsExit);
		SDL_RenderPresent(popupRenderer);
		renderClear(popupRenderer, 255, 255, 255, 255);

		renderClear(renderer, 0, 0, 0, 0);
		
	}
	SDL_DestroyRenderer(popupRenderer);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	return 0;
}

