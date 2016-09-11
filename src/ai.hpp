#pragma once

#include "persistent.hpp"

class Actor;

static const int SCENT_THRESHOLD = 20;

class Ai : public Persistent
{
public:
	virtual ~Ai() {};
	virtual void update(Actor *owner) = 0;

	static Ai *create(TCODZip &zip);

protected:
	enum AiType
	{
		MONSTER, CONFUSED_MONSTER, PLAYER
	};
};

class PlayerAi : public Ai
{
public:
	int xpLevel;
	PlayerAi();
	int getNextLevelXp();
	void update(Actor *owner);

	void save(TCODZip &zip);
	void load(TCODZip &zip);

protected:
	// returns true if owner actually moved
	bool moveOrAttack(Actor *owner, int targetx, int targety);

	void handleActionKey(Actor *owner, int ascii);

	Actor *choseFromInventory(Actor *owner);
};

class MonsterAi : public Ai
{
public:
	void update(Actor *owner);

	void save(TCODZip &zip);
	void load(TCODZip &zip);

protected:
	void moveOrAttack(Actor *owner, int targetx, int targety);
};

class ConfusedMonsterAi : public Ai
{
public:
	ConfusedMonsterAi(int nbTurns, Ai *oldAi);
	void update(Actor *owner);

	void save(TCODZip &zip);
	void load(TCODZip &zip);

protected:
	int nbTurns;
	Ai *oldAi;
};