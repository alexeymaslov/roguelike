#pragma once

#include "persistent.hpp"

class Actor;

class Destructible : public Persistent
{
public:
	float baseMaxHp;
	float hp;
	float baseDefense;
	char *corpseName;
	int xp;


	float defense(Actor *owner) const;
	float maxHp(Actor *owner) const;

	Destructible(float baseMaxHp, float baseDefense, const char *corpseName, int xp);
	virtual ~Destructible();
	inline bool isDead() {return hp <= 0;}
	float takeDamage(Actor *owner, float damage);
	virtual void die(Actor *owner);
	float heal(float amount, Actor *owner);

	void load(TCODZip &zip);
	void save(TCODZip &zip);
	static Destructible *create(TCODZip &zip);

protected:
	enum DestructibleType
	{
		MONSTER, PLAYER
	};

	float calculateBonusDefense(Actor *owner) const;
	float calculateBonusHp(Actor *owner) const;
};

class MonsterDestructible : public Destructible
{
public:
	MonsterDestructible(float maxHp, float defense, const char *corpseName, int xp);
	void die(Actor *owner);

	void save(TCODZip &zip);
};

class PlayerDestructible : public Destructible
{
public:
	PlayerDestructible(float maxHp, float defense, const char *corpseName);
	void die(Actor *owner);

	void save(TCODZip &zip);
};