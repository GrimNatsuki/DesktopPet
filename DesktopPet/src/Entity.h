#pragma once
#include <iostream>
#include <chrono>
#include <vector>
#include "SDL3/SDL.h"
#include "SDL3_image/SDL_image.h"
#include "VectorStructs.h"

using timePoint = std::chrono::time_point<std::chrono::steady_clock>;
class Entity
{
private:
	const char* name;
	SDL_Renderer* renderer;
	SDL_Window* window;

	SDL_DisplayID displayID;
	SDL_Rect displayBounds;
	

	SDL_Surface* surface;
	SDL_Texture* tex;

	std::vector<SDL_FRect> srcRect;
	int srcCount = 0;

	SDL_FRect dsRect;

	timePoint spawnTime;
	int64_t lifetime;

	float virtualXPos;
	float virtualYPos;
	int xPos;
	int yPos;

	double rotationAngle = 0;

	SDL_FPoint head;

	Vector2f velocity = { 0, 0 };
	float acceleration = 0.0025f;

	int displaySpriteIndices[5] = {0, 0, 0, 0, 0};

public:
	Entity(const char* name, SDL_Window& window, SDL_Renderer& renderer, const char* filePath, int spriteRowCount, int spriteColumnCount, timePoint spawnTime)
		:name(name), window(&window), renderer(&renderer), spawnTime(spawnTime)
	{
		//surface = SDL_LoadBMP(filePath);
		surface = IMG_Load(filePath);
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
		head = { dsRect.w/2, 0 };
		displayID = SDL_GetDisplayForWindow(&window);
		SDL_GetDisplayBounds(displayID, &displayBounds);
		virtualXPos = (displayBounds.w / 2.f) - 64.f;
		virtualYPos = displayBounds.h / 2.f;
		xPos = virtualXPos;
		yPos = virtualYPos;
		SDL_SetWindowPosition(&window, xPos, yPos );
	}
	~Entity()
	{
		std::cout << "Entity '" << name << "' destroyed" << std::endl;
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

	void updateDisplayPosition()
	{
		xPos = virtualXPos;
		yPos = virtualYPos;
	};

	void mouseDrag(Vector2f newPos)
	{
		virtualXPos = newPos.x - 64;
		virtualYPos = newPos.y;
		updateDisplayPosition();
		SDL_SetWindowPosition(this->window, xPos, yPos);
	}

	void moveUp()
	{
		SDL_SetWindowPosition(this->window, xPos, yPos -= velocity.y);
	}
	void moveLeft()
	{
		SDL_SetWindowPosition(this->window, xPos -= velocity.x, yPos);
	}
	void moveDown()
	{
		SDL_SetWindowPosition(this->window, xPos, yPos += velocity.y);
	}
	void moveRight()
	{
		SDL_SetWindowPosition(this->window, xPos += velocity.x, yPos);
	}

	void breath(timePoint currentTime)
	{
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
		if (lifetime % 500 < 250) { displaySpriteIndices[0] = 4; }
		else if (lifetime % 500 < 500) { displaySpriteIndices[0] = 5; }
	}

	void sweat(timePoint currentTime)
	{
		if (lifetime % 1000 < 500) { displaySpriteIndices[1] = 6; }
		else if (lifetime % 1000 < 1000) { displaySpriteIndices[1] = 7; }
	}

	void fall(bool& isFalling, timePoint currentTime)
	{
		if (yPos < displayBounds.h - dsRect.h)
		{
			if (lifetime % 2 == 0) {moveDown();}
			velocity.y += acceleration;
			
		}
		else
		{
			isFalling = false;
			velocity.y = 0;
		}
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

	Vector2int getPosition()
	{
		return { xPos, yPos };
	}

	float getGround()
	{
		return {displayBounds.h - dsRect.h };
	}

};
