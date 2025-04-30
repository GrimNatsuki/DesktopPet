#pragma once
#include <iostream>
#include <chrono>
#include <vector>
#include "SDL3/SDL.h"
#include "VectorStructs.h"

using timePoint = std::chrono::time_point<std::chrono::steady_clock>;
class Entity
{
private:
	const char* name;
	SDL_Renderer* renderer;
	SDL_Window* window;

	SDL_Surface* surface;
	SDL_Texture* tex;

	std::vector<SDL_FRect> srcRect;
	int srcCount = 0;

	SDL_FRect dsRect;

	timePoint spawnTime;
	int64_t lifetime;

	int xPos;
	int yPos;

	SDL_FPoint head = { 64, 0 };

	double rotationAngle = 0;

	Vector2int velocity = { 1, 1 };

	int displaySpriteIndices[5] = {0, 0, 0, 0, 0};

public:
	Entity(const char* name, SDL_Window& window, SDL_Renderer& renderer, const char* bitmapFile, int spriteRowCount, int spriteColumnCount, timePoint spawnTime)
		:name(name), window(&window), renderer(&renderer), spawnTime(spawnTime)
	{
		surface = SDL_LoadBMP(bitmapFile);
		tex = SDL_CreateTextureFromSurface(&renderer, surface);
		for (int i = 0; i < spriteColumnCount;i++)
		{
			for (int j = 0; j < spriteRowCount; j++)
			{
				float x = j * 128.f;
				float y = i * 128.f;
				SDL_FRect tempRect = { x, y, 128, 128 };
				srcRect.push_back(tempRect);
			}
		}

		dsRect = { 0, 0, 128, 128 };
	}
	~Entity()
	{
		std::cout << "Entity '" << name << "' is destroyed" << std::endl;
		SDL_DestroyTexture(tex);
		SDL_DestroySurface(surface);
	}
	void display(int indexBufferIndex = 0)
	{
		if (rotationAngle == 0)
		{
			SDL_RenderTexture(this->renderer, tex, &srcRect[displaySpriteIndices[indexBufferIndex]], &dsRect);
			SDL_RenderPresent(this->renderer);
		}
		else
		{
			SDL_RenderTextureRotated(renderer, tex, &srcRect[displaySpriteIndices[indexBufferIndex]], &dsRect, rotationAngle, &head, SDL_FLIP_NONE);
			SDL_RenderPresent(this->renderer);
		}
	}
	void displayMultiple(int spriteCount)
	{
		if (rotationAngle == 0)
		{
			for (int i = 0; i < spriteCount; i++)
			{
				SDL_RenderTexture(this->renderer, tex, &srcRect[displaySpriteIndices[i]], &dsRect);
			}
			SDL_RenderPresent(this->renderer);
		}
		else
		{
			for (int i = 0; i < spriteCount; i++)
			{
				SDL_RenderTextureRotated(renderer, tex, &srcRect[displaySpriteIndices[i]], &dsRect, rotationAngle, &head, SDL_FLIP_NONE);
			}
			SDL_RenderPresent(this->renderer);
		}

	}

	void setSprite(int spriteIndex, int indexBufferIndex = 0)
	{
		displaySpriteIndices[indexBufferIndex] = spriteIndex;
	}

	void setRotation(double angle)
	{
		rotationAngle = angle;
	}
	void smoothRotate(double maxAngle)
	{
		if (maxAngle > 0 && rotationAngle < maxAngle)
		{
			rotationAngle += 1;
		}
		else if (maxAngle < 0 && rotationAngle > maxAngle)
		{
			rotationAngle -= 1;
		}
	}
	void smoothResetRotate()
	{
		if (rotationAngle > 0 && lifetime % 10 == 0)
		{
			rotationAngle -= 0.1;
			if (rotationAngle < 0.01) rotationAngle = 0;
		}

		if (rotationAngle < 0 && lifetime % 10 == 0)
		{
			rotationAngle += 0.1;
			if (rotationAngle > -0.01) rotationAngle = 0;
		}

	}
	void updatePosition()
	{
		SDL_GetWindowPosition(this->window, &xPos, &yPos);
	}
	void mouseDrag(Vector2f newPos)
	{
		updatePosition();
		SDL_SetWindowPosition(this->window, newPos.x - 64, newPos.y);
	}

	void moveUp()
	{
		updatePosition();
		SDL_SetWindowPosition(this->window, xPos, yPos -= velocity.y);
	}
	void moveLeft()
	{
		updatePosition();
		SDL_SetWindowPosition(this->window, xPos -= velocity.x, yPos);
	}
	void moveDown()
	{
		updatePosition();
		SDL_SetWindowPosition(this->window, xPos, yPos += velocity.y);
	}
	void moveRight()
	{
		updatePosition();
		SDL_SetWindowPosition(this->window, xPos += velocity.x, yPos);
	}

	void breath(timePoint currentTime)
	{
		updateLifetime(currentTime);
		if (lifetime % 4000 < 1000)
		{
			if (lifetime % 1000 < 500) { displaySpriteIndices[0] = 0; }
			else if (lifetime % 1000 < 600) { displaySpriteIndices[0] = 1; }
			else if (lifetime % 1000 < 500) { displaySpriteIndices[0] = 0; }
			else if (lifetime % 1000 < 800) { displaySpriteIndices[0] = 2; }
			else if (lifetime % 1000 < 1000) { displaySpriteIndices[0] = 3; }
		}
		else
		{
			if (lifetime % 1000 < 700) { displaySpriteIndices[0] = 0; }
			else if (lifetime % 1000 < 800) { displaySpriteIndices[0] = 2; }
			else if (lifetime % 1000 < 1000) { displaySpriteIndices[0] = 3; }
		}
	}

	void flail(timePoint currentTime)
	{
		updateLifetime(currentTime);
		if (lifetime % 500 < 250) { displaySpriteIndices[0] = 4; }
		else if (lifetime % 500 < 500) { displaySpriteIndices[0] = 5; }
	}

	void sweat(timePoint currentTime)
	{
		updateLifetime(currentTime);
		if (lifetime % 1000 < 500) { displaySpriteIndices[1] = 6; }
		else if (lifetime % 1000 < 1000) { displaySpriteIndices[1] = 7; }
	}

	bool isHeld(SDL_Event& event)
	{
		return (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN);
	}
	bool isReleased(SDL_Event& event)
	{
		return (event.type == SDL_EVENT_MOUSE_BUTTON_UP);
	}
	void updateLifetime(timePoint currentTime)
	{
		lifetime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - spawnTime).count();
	}
	int64_t getLifetime()
	{
		return lifetime;
	}

};
