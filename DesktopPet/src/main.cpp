#include <iostream>
#include <unordered_set>
#include <chrono>
#include <vector>
#include <random>
#include <Windows.h>

#include "SDL3/SDL.h"
#include "SDL3_image/SDL_image.h"
#include "VectorStructs.h"
#include "Entity.h"

using timePoint = std::chrono::time_point<std::chrono::steady_clock>;
static std::mt19937 gen(std::random_device{}());

void renderClear(SDL_Renderer* renderer, const int& r, const int& g, const int& b, const int& a)
{
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
	SDL_RenderClear(renderer);
}

int intRNG()
{
	std::uniform_int_distribution<> dist(1, 100);
	return dist(gen);
}

float floatRNG()
{
	std::uniform_real_distribution<float> dist(0.f, 1.f);
	return dist(gen);
}

int main()
//WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}


	timePoint spawnTime = std::chrono::steady_clock::now();
	timePoint currentTime;
	static timePoint lastTrigger = spawnTime;

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

		if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTrigger).count() > 1000)
		{
			lastTrigger = currentTime;
			int randomInt = intRNG();
			int randomWalkCount = intRNG() % 3 + 1;

			if (Cartethyia.walkCounter == 0)
			{
				if (randomInt < 20 && !Cartethyia.isWalkingLeft && !Cartethyia.isWalkingRight 
				&& Cartethyia.getPosition().x > Cartethyia.getLeftWalkBound())
				{
					Cartethyia.behave_walkLeft(randomWalkCount);
				}
				if (randomInt > 20 && !Cartethyia.isWalkingLeft && !Cartethyia.isWalkingRight
				&& Cartethyia.getPosition().x < Cartethyia.getRightWalkBound())
				{
					Cartethyia.behave_walkRight(randomWalkCount);
				}
			}
			else if (Cartethyia.walkCounter > 0)
			{
				Cartethyia.walkCounter -= 1;
				if (Cartethyia.walkCounter == 0)
				{
					Cartethyia.behave_stopWalking();
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
			Cartethyia.behave_stopWalking();
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

			if (Cartethyia.isWalkingLeft)
			{
				Cartethyia.walkLeft();
			}
			if (Cartethyia.isWalkingRight)
			{
				Cartethyia.walkRight();
			}

			else
			{
				Cartethyia.breath(currentTime);
			}

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

