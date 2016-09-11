#pragma once

#include "libtcod.hpp"

#include "actor.hpp"
#include "map.hpp"
#include "gui.hpp"

float getDistance(int x1, int y1, int x2, int y2);
	
class Engine
{
public :
	enum GameStatus
	{
		STARTUP,
		IDLE,
		NEW_TURN,
		VICTORY,
		DEFEAT
	} gameStatus;

	TCODList<Actor *> actors;
	Actor *player;
	Actor *stairs;
	Map *map;
	int fovRadius;

	int screenWidth;
	int screenHeight;
	Gui *gui;
	TCOD_key_t lastKey;
	TCOD_mouse_t mouse;

	int level;

	Engine(int screenWidth, int screenHeight);
	~Engine();
	void update();
	void render();

	void sendToBack(Actor *actor);
	Actor *getClosestMonster(int x, int y, float range) const;

	// x,y - variables to safe the position of picked tile
	// radius - the highlighted zone around mouse cursor
	// maxRange - the highlighted zone around character where tile may be picked
	bool pickATile(int &x, int &y, float radius = 0.0f, float maxRange = 0.0f);

	Actor *getActor(int x, int y) const;

	void nextLevel();

	void init();
	void term();
	void load();
	void save();
};

extern Engine engine;
