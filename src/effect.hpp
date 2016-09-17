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
		Health, AiChange
	};
};

class HealthEffect : public Effect
{
public:
	// TODO добавить цвет к сообщению
	HealthEffect(float amount, const char *message);
	bool applyTo(Actor *actor);

	void save(TCODZip &zip);
	void load(TCODZip &zip);

protected:
	float amount;	
	// message должно иметь %s для actor->getName() и %g для количества хп  
	const char *message;
};

class AiChangeEffect : public Effect
{
public:
	AiChangeEffect(TemporaryAi *newAi, const char *message);
	bool applyTo(Actor *actor);

	void save(TCODZip &zip);
	void load(TCODZip &zip);

protected:
	TemporaryAi *newAi;
	// message должно иметь %s для actor->getName()
	const char *message;
};