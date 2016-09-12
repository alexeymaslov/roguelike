#pragma once

#include "persistent.hpp"
#include "targetselector.hpp"
#include "effect.hpp"

class Actor;

class Pickable : public Persistent
{
public:
	Pickable(TargetSelector *selector = NULL, Effect *effect = NULL);
	virtual ~Pickable();

	bool pick(Actor *owner, Actor *wearer);
	void drop(Actor *owner, Actor *wearer);
	virtual bool use(Actor *owner, Actor *wearer);

	static Pickable* create(TCODZip &zip);

	void load(TCODZip &zip);
	void save(TCODZip &zip);

protected:
	enum PickableType
	{
		HEALER, LIGHTNING_BOLT, FIREBALL, CONFUSER, PICKABLE
	};
	// если NULL то цель - wearer
	TargetSelector *selector;
	Effect *effect;
};

class Healer : public Pickable
{
public:
	float amount;

	Healer(float amount);
	bool use(Actor *owner, Actor *wearer);

	void load(TCODZip &zip);
	void save(TCODZip &zip); 
};

class LightningBolt : public Pickable
{
public:
	float range;
	float damage;
	LightningBolt(float range, float damage);
	bool use(Actor *owner, Actor *wearer);

	void load(TCODZip &zip);
	void save(TCODZip &zip); 
};

class Fireball : public LightningBolt
{
public:
	Fireball(float range, float damage);
	bool use(Actor *owner, Actor *wearer);

	void save(TCODZip &zip); 
};

class Confuser : public Pickable
{
public:
	int nbTurns;
	float range;
	Confuser(int nbTurns, float range);
	bool use(Actor *owner, Actor *wearer);

	void load(TCODZip &zip);
	void save(TCODZip &zip); 
};