#pragma once

#include "libtcod.hpp"

#include "actor.hpp"
#include "map.hpp"
#include "gui.hpp"

float getDistance(int x1, int y1, int x2, int y2);
	
class Engine
{
public :
	Engine(int screenWidth, int screenHeight);
	Engine(int screenWidth, int screenHeight, int mapWidth, int mapHeight);
	~Engine();

	enum GameStatus
	{
		StartUp,
		Idle,
		NewTurn,
		Victory,
		Defeat
	};
	void setGameStatus(GameStatus status) { gameStatus = status; };
	TCODList<Actor *> &getActors() { return actors; };
	Actor *getPlayer() { return player; };
	Actor *getStairs() { return stairs; };
	Map *getMap() { return map; };
	int getScreenWidth() const { return screenWidth; };
	int getScreenHeight() const { return screenHeight; };
	int getCamerax() const { return camerax; };
	int getCameray() const { return cameray; };
	int getCameraHeight() const { return cameraHeight; };
	int getCameraWidth() const { return cameraWidth; };
	Gui *getGui() { return gui; };
	TCOD_key_t &getLastKey() {return lastKey; };
	TCOD_mouse_t &getMouse() { return mouse; };
	int getLevel() const { return level; };
	int getFovRadius() const { return fovRadius; };
	void update();
	void render();

	void toCameraCoords(int x, int y, int &rx, int &ry);
	void sendToBack(Actor *actor);
	Actor *getClosestMonster(int x, int y, float range) const;

	// x,y - сохраняют выбранную клетку
	// radius - подсвеченная зона вокруг курсора
	// maxRange - подсвеченная зона вокруг персонажа, где можно выбрать клетку
	bool pickATile(int &x, int &y, float radius = 0.0f, float maxRange = 0.0f);

	Actor *getActor(int x, int y) const;

	void nextLevel();

	void init();
	void term();
	void load();
	void save();

protected:
	GameStatus gameStatus;
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
	void moveCamera(int x, int y);
};

extern Engine engine;
