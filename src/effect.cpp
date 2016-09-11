#include "effect.hpp"

#include "engine.hpp"

HealthEffect::HealthEffect(float amount, const char *message) :
	amount(amount), message(message)
{

}

bool HealthEffect::applyTo(Actor *actor)
{
	if (!actor->destructible) return false;

	if (amount > 0)
	{
		float healed = actor->destructible->heal(amount, actor);
		if (healed > 0)
		{
			if (message)
				engine.gui->message(TCODColor::lightGrey, message, actor->name, healed);
			return true;
		}
	}
	else
	{
		if (message && -amount - actor->destructible->defense(actor) > 0)
			engine.gui->message(TCODColor::lightGrey, message, actor->name,
				-amount - actor->destructible->defense(actor));
		if (actor->destructible->takeDamage(actor, -amount) > 0)
			return true;
	}
	return false;
}

AiChangeEffect::AiChangeEffect(TemporaryAi *newAi, const char *message) : newAi(newAi), 
	message(message)
{

}

bool AiChangeEffect::applyTo(Actor *actor)
{
	newAi->applyTo(actor);
	if (message)
		engine.gui->message(TCODColor::lightGrey, message, actor->name);
	return true;
}