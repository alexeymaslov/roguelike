#pragma once

#include "persistent.hpp"

class Actor;

class Container : public Persistent
{
public:

	Container(Actor *owner, int maxSize);
	~Container();

	TCODList<Actor *> &getInventory() { return inventory; };
	Actor *getOwner() { return owner; };
	// false - не смог добавить, тк уже заполнен
	// true - смог добавить успешно
	bool add(Actor *actor);
	void remove(Actor *actor);

	void save(TCODZip &zip);
	void load(TCODZip &zip);

private:
	Actor *owner;
	// Если 0, то бесконечный контейнер
	int maxSize;
	// TODO объединить как нибудь Pickable и Equipment, чтобы не хранить тут Actor
	TCODList<Actor *> inventory;
};