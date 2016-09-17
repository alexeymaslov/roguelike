#pragma once

#include "persistent.hpp"

class Actor;

class Destructible : public Persistent
{
public:
	Destructible(Actor *owner, float baseMaxHp, float baseDefense, const char *corpseName, int xp);
	virtual ~Destructible();

	void setOwner(Actor *owner) { this->owner = owner; };
	void addXp(int xp) { this->xp += xp; };
	void addMaxHp(int hp);
	void addBaseDefense(float amount) { baseDefense += amount; };

	float getDefense() const;
	float getHp() const { return hp; };
	float maxHp() const;
	int getXp() const { return xp; };

	inline bool isDead() const { return hp <= 0; };
	// Возвращает полученный урон
	float takeDamage(float damage);
	// Возвращает полученное лечение
	float heal(float amount);

	void load(TCODZip &zip);
	void save(TCODZip &zip);
	static Destructible *create(TCODZip &zip);

protected:
	// Используем при сохранении
	enum DestructibleType
	{
		Monster, Player
	};
	Actor *owner;
	float baseMaxHp;
	float hp;
	float baseDefense;
	char *corpseName;
	int xp;

	// Меняет некоторые параметры owner
	virtual void die();
	float calculateBonusDefense() const;
	float calculateBonusHp() const;
};

class MonsterDestructible : public Destructible
{
public:
	MonsterDestructible(Actor *owner, float maxHp, float defense, const char *corpseName, int xp);
	void die();

	void save(TCODZip &zip);
};

class PlayerDestructible : public Destructible
{
public:
	PlayerDestructible(Actor *owner, float maxHp, float defense, const char *corpseName);
	void die();

	void save(TCODZip &zip);
};