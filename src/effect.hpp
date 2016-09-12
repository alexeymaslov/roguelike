#pragma once

class TemporaryAi;

class Actor;

class Effect
{
public:
	virtual bool applyTo(Actor *actor) = 0;
};

class HealthEffect : public Effect
{
public:
	float amount;
	const char *message;

	HealthEffect(float amount, const char *message);
	bool applyTo(Actor *actor);
};

class AiChangeEffect : public Effect
{
public:
	TemporaryAi *newAi;
	// message должно иметь %s для actor->name
	const char *message;

	AiChangeEffect(TemporaryAi *newAi, const char *message);
	bool applyTo(Actor *actor);
};