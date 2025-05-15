#pragma once
#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <cmath>
#include "SDL3/SDL.h"
#include "SDL3_image/SDL_image.h"
#include "VectorStructs.h"

using timePoint = std::chrono::time_point<std::chrono::steady_clock>;
class Entity
{
public:
	bool isHeld = false;
	bool isFalling = true;

	bool isWalkingLeft = false;
	bool isWalkingRight = false;

	int walkCounter = 0;

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
	float ground;
	float rightWalkBound;
	float leftWalkBound;

	double rotationAngle = 0;

	SDL_FPoint head;

	Vector2f velocity = { 1, 0 };
	const float acceleration = 0.0025f;

	std::vector <int> layerIndices = { 0 };

public:
	Entity(const char* name, SDL_Window& window, SDL_Renderer& renderer, const char* filePath, int spriteRowCount, int spriteColumnCount, timePoint spawnTime)
		:name(name), window(&window), renderer(&renderer), spawnTime(spawnTime)
	{
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

		ground = displayBounds.h - dsRect.h;
		leftWalkBound = (displayBounds.w) / 4;
		rightWalkBound = 3 * (displayBounds.w) / 4;

		SDL_SetWindowPosition(&window, xPos, yPos );


	}
	~Entity()
	{
		std::cout << "Entity '" << name << "' destroyed" << std::endl;
		SDL_DestroyTexture(tex);
		SDL_DestroySurface(surface);
	}

	void addLayers(int layerCount)
	{
		for (int i = 0; i < layerCount;i++)
		{
			layerIndices.push_back(0);
		}
	}

	void displayAll()
	{
		if (rotationAngle == 0)
		{
			for (int i = 0; i < layerIndices.size(); i++)
			{
				SDL_RenderTexture(this->renderer, tex, &srcRect[layerIndices[i]], &dsRect);
			}
			SDL_RenderPresent(this->renderer);
		}
		else
		{
			for (int i = 0; i < layerIndices.size(); i++)
			{
				SDL_RenderTextureRotated(renderer, tex, &srcRect[layerIndices[i]], &dsRect, rotationAngle, &head, SDL_FLIP_NONE);
			}
			SDL_RenderPresent(this->renderer);
		}
	}

	void displayLayer(int layerIndex)
	{
		if (rotationAngle == 0)
		{
			SDL_RenderTexture(this->renderer, tex, &srcRect[layerIndices[layerIndex]], &dsRect);
			SDL_RenderPresent(this->renderer);
		}
		else
		{
			SDL_RenderTextureRotated(renderer, tex, &srcRect[layerIndices[layerIndex]], &dsRect, rotationAngle, &head, SDL_FLIP_NONE);
			SDL_RenderPresent(this->renderer);
		}
	}

	void setSprite(int spriteIndex, int indexBufferIndex = 0)
	{
		layerIndices[indexBufferIndex] = spriteIndex;
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
		xPos = static_cast<int>(std::round(virtualXPos));
		yPos = static_cast<int>(std::round(virtualYPos));
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
		SDL_SetWindowPosition(this->window, xPos, yPos);
	}
	void moveLeft()
	{
		virtualXPos -= velocity.x;
		updateDisplayPosition();
		SDL_SetWindowPosition(this->window, xPos, yPos);
	}
	void moveDown()
	{
		virtualYPos += velocity.y;
		updateDisplayPosition();
		SDL_SetWindowPosition(this->window, xPos, yPos);
	}
	void moveRight()
	{
		virtualXPos += velocity.x;
		updateDisplayPosition();
		SDL_SetWindowPosition(this->window, xPos, yPos);
	}

	void behave_walkLeft(int walkCountIn)
	{
		if (!isHeld)
		{
			walkCounter = walkCountIn;
			std::cout << "walkleft" << std::endl;
			isWalkingLeft = true;
			isWalkingRight = false;
			
		}

	}
	void behave_walkRight(int walkCountIn)
	{
		if (!isHeld)
		{
			walkCounter = walkCountIn;
			std::cout << "walkright" << std::endl;
			isWalkingLeft = false;
			isWalkingRight = true;
		}
	}

	void behave_stopWalking()
	{
		isWalkingLeft = false;
		isWalkingRight = false;
		velocity.x = 0;
	}

	void walkRight()
	{
		if (velocity.x < 1) { velocity.x = 1; }
		if (lifetime % 20 == 0) { moveRight(); }
	}
	void walkLeft()
	{
		if (velocity.x < 1) { velocity.x = 1; }
		if (lifetime % 20 == 0) { moveLeft(); }
	}

	void breath(timePoint currentTime)
	{
		if (layerIndices.size() > 1)
		{
			layerIndices = { 0 };
		}
		if (lifetime % 4000 < 1000)
		{
			if (lifetime % 1000 < 500) { layerIndices[0] = 0; }
			else if (lifetime % 1000 < 600) { layerIndices[0] = 1; }
			else if (lifetime % 1000 < 500) { layerIndices[0] = 0; }
			else if (lifetime % 1000 < 800) { layerIndices[0] = 2; }
			else if (lifetime % 1000 < 1000) { layerIndices[0] = 3; }
		}
		else
		{
			if (lifetime % 1000 < 700) { layerIndices[0] = 0; }
			else if (lifetime % 1000 < 800) { layerIndices[0] = 2; }
			else if (lifetime % 1000 < 1000) { layerIndices[0] = 3; }
		}
	}


	void flail(timePoint currentTime)
	{
		if (lifetime % 500 < 250) { layerIndices[0] = 4; }
		else if (lifetime % 500 < 500) { layerIndices[0] = 5; }
	}

	void sweat(timePoint currentTime)
	{
		if (layerIndices.size()<2)
		{
			addLayers(1);
		}
		if (lifetime % 1000 < 500) { layerIndices[1] = 6; }
		else if (lifetime % 1000 < 1000) { layerIndices[1] = 7; }

	}

	void fall(timePoint currentTime)
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
		return {ground};
	}
	float getLeftWalkBound()
	{
		return { leftWalkBound };
	}
	float getRightWalkBound()
	{
		return { rightWalkBound };
	}

};