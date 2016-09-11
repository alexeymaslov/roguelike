#include "attacker.hpp"

#include "actor.hpp"
#include "engine.hpp"

#include <stdio.h>

Attacker::Attacker(float basePower) : basePower(basePower)
{

}
// TODO make same func for defense and hp
float Attacker::calculateBonusPower(Actor *owner) const
{
	if (owner != engine.player)
		return 0;

	float sum = 0;
	auto &items = engine.player->container->inventory;
	for (Actor **i = items.begin(); i != items.end(); ++i)
		if ((*i)->equipment)
			sum += (*i)->equipment->powerBonus;

	return sum;
}

float Attacker::power(Actor *owner) const
{
	return basePower + calculateBonusPower(owner);
}

void Attacker::attack(Actor *owner, Actor *target)
{
	if (target->destructible && !target->destructible->isDead())
	{
		if (power(owner) - target->destructible->defense(target) > 0)
			engine.gui->message(owner == engine.player ? TCODColor::red : TCODColor::lightGrey,
				"%s attacks %s for %g hit points.", owner->name, 
				target->name, power(owner) - target->destructible->defense(target));
		else
			engine.gui->message(TCODColor::lightGrey,
				"%s attacks %s but it has no effect!", owner->name, target->name);
		target->destructible->takeDamage(target, power(owner));
	}
	else
		engine.gui->message(TCODColor::lightGrey,
				"%s attacks %s in vain.", owner->name, target->name);
}

