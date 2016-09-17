#include "math.h"

#include "actor.hpp"
#include "engine.hpp"


Actor::Actor(int x, int y, int ch, const char *name, const TCODColor &color) :
	x(x), y(y), ch(ch), color(color), name(name), blocks(true), fovOnly(true), 
	attacker(nullptr), destructible(nullptr), ai(nullptr), pickable(nullptr), container(nullptr),
	equipment(nullptr)
{
	
}

Actor::~Actor()
{
	if (attacker) delete attacker;
	if (destructible) delete destructible;
	if (ai) delete ai;
	if (pickable) delete pickable;
	if (container) delete container;
	if (equipment) delete equipment;
}

void Actor::render() const
{
	int cx;
	int cy;
	// TODO пусть возвращает лучше пару
	engine.toCameraCoords(x, y, cx, cy);
	if (cx != -1)
	{
		TCODConsole::root->setChar(cx, cy, ch);
		TCODConsole::root->setCharForeground(cx, cy, color);
	}
}

void Actor::update()
{
	if (ai) ai->update();
}

float Actor::getDistanceTo(int cx, int cy) const
{
	int dx = cx - x;
	int dy = cy - y;
	return sqrtf(dx * dx + dy * dy);
}

