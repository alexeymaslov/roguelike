#pragma once

#include "libtcod.hpp"
#include "persistent.hpp"

class TemporaryAi;

class Actor;

class Effect : public Persistent
{
public:
	virtual bool applyTo(Actor *actor) = 0;

	static Effect *create(TCODZip &zip);
	
protected:
	enum EffectType
	{
		HEALTH, AI_CHANGE
	};
};

class HealthEffect : public Effect
{
public:
	float amount;
	// message должно иметь %s для actor->name и %g для количества хп  
	const char *message;
	// TODO добавить цвет к сообщению
	HealthEffect(float amount, const char *message);
	bool applyTo(Actor *actor);

	void save(TCODZip &zip);
	void load(TCODZip &zip);
};

class AiChangeEffect : public Effect
{
public:
	TemporaryAi *newAi;
	// message должно иметь %s для actor->name
	const char *message;

	AiChangeEffect(TemporaryAi *newAi, const char *message);
	bool applyTo(Actor *actor);

	void save(TCODZip &zip);
	void load(TCODZip &zip);
};