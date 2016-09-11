#pragma once

#include "libtcod.hpp"

#include "persistent.hpp"
#include "attacker.hpp"
#include "destructible.hpp"
#include "ai.hpp"
#include "pickable.hpp"
#include "container.hpp"
#include "equipment.hpp"

class Actor : public Persistent
{ 
public :
	int x;
	int y; // position on map
	int ch; // ascii code
	TCODColor col; // color
	const char *name;
	bool blocks; // Можно ли пройти на него
	bool fovOnly;

	Attacker *attacker;
	Destructible *destructible;
	Ai *ai;
	Pickable *pickable;
	Container *container;
	Equipment *equipment;

	Actor(int x, int y, int ch, const char *name, const TCODColor &col);
	~Actor();
	void render() const;
	void update();

	float getDistance(int cx, int cy) const;

	void save(TCODZip &zip);
	void load(TCODZip &zip);

};