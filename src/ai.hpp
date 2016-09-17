#pragma once

#include "persistent.hpp"

class Actor;
// TODO уточнить эту константу
static const int ScentThreshold = 10;

class Ai : public Persistent
{
public:
	Ai(Actor *owner);
	virtual ~Ai() {};
	void setOwner(Actor *owner) { this->owner = owner; };
	virtual void update() = 0;

	static Ai *create(TCODZip &zip);

protected:
	enum AiType
	{
		Monster, TemporaryAi, Player
	};

	Actor *owner;
};

class PlayerAi : public Ai
{
public:
	PlayerAi(Actor *owner);

	void update();
	int getNextLevelXp();
	int getXpLevel() const { return xpLevel; };

	void save(TCODZip &zip);
	void load(TCODZip &zip);

protected:
	int xpLevel;

	// Возвращает true, если owner действительно переместился
	bool moveOrAttack(int targetx, int targety);
	// ascii - символ нажатой клавиши
	void handleActionKey(int ascii);
	// Возвращает объект с компонентой Pickable выбранный из Container
	Actor *choseFromInventory();
	void levelUp();
};

class MonsterAi : public Ai
{
public:
	MonsterAi(Actor *owner);
	void update();

	void save(TCODZip &zip);
	void load(TCODZip &zip);

protected:
	void moveOrAttack(Actor *target);
	// Использует pathfinding
	void moveToTarget(Actor *target);
	// Старая модель поиска пути
	void moveToCoords(int targetx, int targety);
};

class TemporaryAi : public Ai
{
public:
	TemporaryAi(Actor *owner, int nbTurns);
	void update();
	void applyTo(Actor *actor);

	static TemporaryAi *create(TCODZip &zip);
	void save(TCODZip &zip);
	void load(TCODZip &zip);

protected:
	int nbTurns;
	Ai *oldAi;

	enum TemporaryAiType
	{
		ConfusedMonster
	};
};

class ConfusedMonsterAi : public TemporaryAi
{
public:
	ConfusedMonsterAi(Actor *owner, int nbTurns);
	void update();

	void save(TCODZip &zip);
	void load(TCODZip &zip);
};