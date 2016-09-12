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
	// TODO map сама тоже хранит свои размеры, надо убрать где-то
	int mapWidth;
	int mapHeight;

	int cameraWidth;
	int cameraHeight;
	int camerax;
	int cameray; 

	Gui *gui;
	TCOD_key_t lastKey;
	TCOD_mouse_t mouse;

	int level;

	Engine(int screenWidth, int screenHeight);
	Engine(int screenWidth, int screenHeight, int mapWidth, int mapHeight);
	~Engine();
	void update();
	void render();
	void toCameraCoords(int x, int y, int &rx, int &ry);

	void sendToBack(Actor *actor);
	Actor *getClosestMonster(int x, int y, float range) const;

	// x,y - variables to safe the position in map of picked tile
	// radius - the highlighted zone around mouse cursor
	// maxRange - the highlighted zone around character where tile may be picked
	bool pickATile(int &x, int &y, float radius = 0.0f, float maxRange = 0.0f);

	Actor *getActor(int x, int y) const;

	void nextLevel();

	void init();
	void term();
	void load();
	void save();

protected:
	void moveCamera(int x, int y);
};

extern Engine engine;
