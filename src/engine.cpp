#include "math.h"

#include "engine.hpp"

Engine::Engine(int screenWidth, int screenHeight) :
	Engine(screenWidth, screenHeight, screenWidth, screenHeight - PANEL_HEIGHT)
{

}

Engine::Engine(int screenWidth, int screenHeight, int mapWidth, int mapHeight) : 
	gameStatus(STARTUP), player(NULL), map(NULL), fovRadius(10), 
	level(1), screenWidth(screenWidth), screenHeight(screenHeight),
	cameraWidth(screenWidth), cameraHeight(screenHeight - PANEL_HEIGHT),
	mapWidth(mapWidth), mapHeight(mapHeight)
{
	TCODConsole::initRoot(screenWidth, screenHeight, "", false);
	gui = new Gui();
}

// Вызывается только в момент начала новой игры, при переходе на новый уровень
// игрок и ступеньки устанавливаются в методах map
void Engine::init()
{
	player = new Actor(0, 0, '@', "player", TCODColor::white);
	player->destructible = new PlayerDestructible(30, 2, "your cadaver");
	player->attacker = new Attacker(2);
	player->ai = new PlayerAi();
	player->container = new Container(26);

	Actor *dagger = new Actor(0, 0, '-', "dagger",TCODColor::sky);
	dagger->blocks = false;
	dagger->equipment = new Equipment(Equipment::RIGHT_HAND, 2);
	dagger->pickable = new Pickable();
	player->container->add(dagger);
	dagger->equipment->equip(dagger, player);

	actors.push(player);
	stairs = new Actor(0, 0, '>', "stairs", TCODColor::white);
	stairs->blocks = false;
	stairs->fovOnly = false;
	actors.push(stairs);

	map = new Map(mapWidth, mapHeight);
	map->init(true);
	gui->message(TCODColor::red, 
		"Welcome stranger!\nPrepare to perish in the Tombs of the Ancient Kings.");
	gameStatus = STARTUP;
}

void Engine::term()
{
	actors.clearAndDelete();
	if (map) delete map;
	gui->clear();
}

Engine::~Engine()
{
	term();
	delete gui;
}
void Engine::update()
{
	if (gameStatus == STARTUP) map->computeFov();
	gameStatus = IDLE;

	TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS | TCOD_EVENT_MOUSE, &lastKey, &mouse);
	if (lastKey.vk == TCODK_ESCAPE)
	{
		save();
		load();
	}
	player->update();

	if (gameStatus == NEW_TURN)
	{
		map->currentScentValue++;
		for (Actor **iterator = actors.begin(); iterator != actors.end(); ++iterator)
		{
			Actor *actor = *iterator;
			if (actor != player)
				actor->update();
		}
	}
}

void Engine::render()
{
	moveCamera(player->x, player->y);

	TCODConsole::root->clear();
	// draw the map
	map->render();
	// draw the actors
	for (Actor **iterator = actors.begin(); iterator != actors.end(); ++iterator)
	{
		Actor *actor = *iterator;
		if (actor != player 
			&& ((!actor->fovOnly && map->isExplored(actor->x, actor->y)) 
				|| map->isInFov(actor->x, actor->y)))
			actor->render();
	}

	player->render();
	gui->render();
}

void Engine::sendToBack(Actor *actor)
{
	actors.remove(actor);
	actors.insertBefore(actor, 0);
}

Actor *Engine::getClosestMonster(int x, int y, float range) const
{
	Actor *closest = NULL;
	float bestDistance = 1E6f;
	for (Actor **it = actors.begin(); it != actors.end(); ++it)
	{
		Actor *actor = *it;
		if (actor != player && actor->destructible && !actor->destructible->isDead())
		{
			float distance = actor->getDistance(x, y);
			if (distance < bestDistance && (distance <= range || range == 0.0f))
			{
				bestDistance = distance;
				closest = actor;
			}
		}
	}
	return closest;
}

bool Engine::pickATile(int &x, int &y, float radius, float maxRange)
{
	while (!TCODConsole::isWindowClosed())
	{
		render();

		for (int cx = 0; cx < map->width; ++cx)
			for (int cy = 0; cy < map->height; ++cy)
				if (map->isInFov(cx, cy)
				 && (maxRange == 0 || player->getDistance(cx, cy) <= maxRange))
				{
					TCODColor col = TCODConsole::root->getCharBackground(cx, cy);
					col = col * 1.2f;
					TCODConsole::root->setCharBackground(cx, cy, col);
				}

		TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS|TCOD_EVENT_MOUSE,&lastKey,&mouse);
		int mapx = mouse.cx + camerax;
		int mapy = mouse.cy + cameray;
		if (mouse.cy < engine.cameraHeight && map->isInFov(mapx, mapy)
			&& (maxRange == 0 || player->getDistance(mapx, mapy) <= maxRange))
		{
			for (int cx = 0; cx < map->width; ++cx)
				for (int cy = 0; cy < map->height; ++cy)
					if (getDistance(mapx, mapy, cx, cy) <= radius)
						TCODConsole::root->setCharBackground(cx, cy, TCODColor::white);

			if (mouse.lbutton_pressed)
			{
				x = mapx;
				y = mapy;
				return true;
			}
		}
		if (mouse.rbutton_pressed || lastKey.vk != TCODK_NONE)
			return false;
		TCODConsole::flush();
	}
	return false;
}

float getDistance(int x1, int y1, int x2, int y2)
{
	return sqrtf((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

Actor *Engine::getActor(int x, int y) const
{
	for (Actor **it = actors.begin(); it != actors.end(); ++it)
	{
		Actor *actor = *it;
		if (actor->x == x && actor->y == y && actor->destructible
			&& !actor->destructible->isDead())
			return actor;
	}
	return NULL;
}

void Engine::nextLevel()
{
	++level;
	gui->message(TCODColor::lightViolet,"You take a moment to rest, and recover your strength.");
	player->destructible->heal(player->destructible->maxHp(player) / 2, player);
	gui->message(TCODColor::red,"After a rare moment of peace, you descend\n"
		"deeper into the heart of the dungeon...");

	delete map;
	for (Actor **i = actors.begin(); i != actors.end(); ++i)
		if (*i != player && *i != stairs)
		{
			delete *i;
			i = actors.remove(i);
		}

	map = new Map(mapWidth, mapHeight);
	map->init(true);
	gameStatus = STARTUP;
}

void Engine::moveCamera(int x, int y)
{
	// новые координаты левого верхнего угла
	int cx = x - cameraWidth / 2;
	int cy = y - cameraHeight / 2;

	if (cx > map->width - cameraWidth) cx = map->width - cameraWidth;
	if (cy > map->height - cameraHeight) cy = map->height - cameraHeight;
	if (cx < 0) cx = 0;
	if (cy < 0) cy = 0;

	camerax = cx;
	cameray = cy;
}


void Engine::toCameraCoords(int x, int y, int &rx, int &ry)
{
	rx = x - camerax;
	ry = y - cameray;
	if (rx < 0 || ry < 0 || rx >= cameraWidth || ry >= cameraHeight)
	{
		rx = -1;
		ry = -1;
	}
}