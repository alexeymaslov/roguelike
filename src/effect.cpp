#include "effect.hpp"

#include "engine.hpp"

HealthEffect::HealthEffect(float amount, const char *message) :
	amount(amount), message(message)
{

}

bool HealthEffect::applyTo(Actor *actor)
{
	Destructible *destructible = actor->getDestructible();
	if (!destructible) return false;

	if (amount > 0)
	{
		float healed = destructible->heal(amount);
		if (healed > 0)
		{
			if (message)
				engine.getGui()->message(TCODColor::lightGrey, message, actor->getName(), healed);
			return true;
		}
	}
	else
	{
		// TODO вроде бы считаем лишнее, переделать
		float damage = -amount - destructible->getDefense();
		if (message &&  damage > 0)
			engine.getGui()->message(TCODColor::lightGrey, message, actor->getName(),
				damage);
		if (destructible->takeDamage(-amount) > 0)
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
		engine.getGui()->message(TCODColor::lightGrey, message, actor->getName());
	return true;
}