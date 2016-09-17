#include "math.h"

#include "engine.hpp"

Engine::Engine(int screenWidth, int screenHeight) :
	Engine(screenWidth, screenHeight, screenWidth, screenHeight - PanelHeight)
{

}

Engine::Engine(int screenWidth, int screenHeight, int mapWidth, int mapHeight) : 
	gameStatus(StartUp), player(nullptr), map(nullptr), fovRadius(10), 
	level(1), screenWidth(screenWidth), screenHeight(screenHeight),
	cameraWidth(screenWidth), cameraHeight(screenHeight - PanelHeight),
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
	player->setDestructible(new PlayerDestructible(player, 30, 2, "your cadaver"));
	player->setAttacker(new Attacker(player, 2));
	player->setAi(new PlayerAi(player));
	player->setContainer(new Container(player, 26));

	Actor *dagger = new Actor(0, 0, '-', "dagger",TCODColor::sky);
	dagger->setBlocks(false);
	dagger->setEquipment(new Equipment(dagger, Equipment::RightHand, 2));
	dagger->setPickable(new Pickable(dagger));
	player->getContainer()->add(dagger);
	dagger->getEquipment()->equip();

	actors.push(player);
	stairs = new Actor(0, 0, '>', "stairs", TCODColor::white);
	stairs->setBlocks(false);
	stairs->setFovOnly(false);
	actors.push(stairs);

	map = new Map(mapWidth, mapHeight);
	map->init(true);
	gui->message(TCODColor::red, 
		"Welcome to the dungeon, traveller!");
	gameStatus = StartUp;
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
	if (gameStatus == StartUp) map->computeFov();
	gameStatus = Idle;

	TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS | TCOD_EVENT_MOUSE, &lastKey, &mouse);
	if (lastKey.vk == TCODK_ESCAPE)
	{
		save();
		load();
	}
	player->update();

	if (gameStatus == NewTurn)
	{
		map->incCurrentScentValue();
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
	moveCamera(player->getX(), player->getY());

	TCODConsole::root->clear();
	// draw the map
	map->render();
	// draw the actors
	for (Actor **iterator = actors.begin(); iterator != actors.end(); ++iterator)
	{
		Actor *actor = *iterator;
		if (actor != player 
			&& ((!actor->isFovOnly() && map->isExplored(actor->getX(), actor->getY())) 
				|| map->isInFov(actor->getX(), actor->getY())))
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
	Actor *closest = nullptr;
	float bestDistance = 1E6f;
	for (Actor **it = actors.begin(); it != actors.end(); ++it)
	{
		Actor *actor = *it;
		if (actor != player && actor->getDestructible() && !actor->getDestructible()->isDead())
		{
			float distance = actor->getDistanceTo(x, y);
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

		for (int cx = 0; cx < map->getWidth(); ++cx)
			for (int cy = 0; cy < map->getHeight(); ++cy)
				if (map->isInFov(cx, cy)
				 && (maxRange == 0 || player->getDistanceTo(cx, cy) <= maxRange))
				{
					TCODColor col = TCODConsole::root->getCharBackground(cx - camerax, cy - cameray);
					col = col * 1.2f;
					TCODConsole::root->setCharBackground(cx - camerax, cy - cameray, col);
				}

		TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS|TCOD_EVENT_MOUSE,&lastKey,&mouse);
		int mapx = mouse.cx + camerax;
		int mapy = mouse.cy + cameray;
		if (mouse.cy < cameraHeight && map->isInFov(mapx, mapy)
			&& (maxRange == 0 || player->getDistanceTo(mapx, mapy) <= maxRange))
		{
			for (int cx = 0; cx < map->getWidth(); ++cx)
				for (int cy = 0; cy < map->getHeight(); ++cy)
					if (getDistance(mapx, mapy, cx, cy) <= radius)
						TCODConsole::root->setCharBackground(cx - camerax, cy - cameray, TCODColor::white);

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
		if (actor->getX() == x && actor->getY() == y && actor->getDestructible()
			&& !actor->getDestructible()->isDead())
			return actor;
	}
	return nullptr;
}

void Engine::nextLevel()
{
	++level;
	gui->message(TCODColor::lightViolet,"You take a moment to rest, and recover your strength.");
	player->getDestructible()->heal(player->getDestructible()->maxHp() / 2);
	gui->message(TCODColor::red,"You descend deeper into the heart of the dungeon...");

	delete map;
	for (Actor **i = actors.begin(); i != actors.end(); ++i)
		if (*i != player && *i != stairs)
		{
			delete *i;
			i = actors.remove(i);
		}

	map = new Map(mapWidth, mapHeight);
	map->init(true);
	gameStatus = StartUp;
}

void Engine::moveCamera(int x, int y)
{
	// новые координаты левого верхнего угла
	int cx = x - cameraWidth / 2;
	int cy = y - cameraHeight / 2;

	if (cx > map->getWidth() - cameraWidth) cx = map->getWidth() - cameraWidth;
	if (cy > map->getHeight() - cameraHeight) cy = map->getHeight() - cameraHeight;
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