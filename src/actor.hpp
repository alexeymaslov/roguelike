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
	Actor(int x, int y, int ch, const char *name, const TCODColor &color);
	~Actor();

	int getX() const { return x; };
	int getY() const { return y; };
	void setCoords(int x, int y) { this->x = x; this->y = y; };
	void setCh(int ch) { this->ch = ch; };
	void setColor(const TCODColor &col) { this->color = col; };
	const char *getName() const { return name; };
	void setName(const char *name) { this->name = strdup(name); };
	bool getBlocks() const { return blocks; };
	void setBlocks(bool blocks) { this->blocks = blocks; };
	bool isFovOnly() const { return fovOnly; };
	void setFovOnly(bool fovOnly) { this->fovOnly = fovOnly; };
	Attacker *getAttacker() { return attacker; };
	void setAttacker(Attacker *attacker) { this->attacker = attacker; };
	Destructible *getDestructible() { return destructible; };
	void setDestructible(Destructible *destructible) { this->destructible = destructible; };
	Ai *getAi() { return ai; };
	void setAi(Ai *ai) { this->ai = ai; };
	Pickable *getPickable() { return pickable; };
	void setPickable(Pickable *pickable) { this->pickable = pickable; };
	Container *getContainer() { return container; };
	void setContainer(Container *container) { this->container = container; };
	Equipment *getEquipment() { return equipment; };
	void setEquipment(Equipment *equipment) { this->equipment = equipment; };

	void render() const;
	void update();

	float getDistanceTo(int cx, int cy) const;

	void save(TCODZip &zip);
	void load(TCODZip &zip);

private:
	int x;
	int y;
	// ascii код
	int ch;
	TCODColor color;
	const char *name;
	// Можно ли пройти на него
	bool blocks;
	// Рендерится только когда в поле зрения игрока 
	bool fovOnly;
	Attacker *attacker;
	Destructible *destructible;
	Ai *ai;
	Pickable *pickable;
	Container *container;
	Equipment *equipment;
};