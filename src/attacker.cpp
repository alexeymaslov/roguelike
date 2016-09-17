#include "attacker.hpp"

#include "actor.hpp"
#include "engine.hpp"

#include <stdio.h>

Attacker::Attacker(Actor *owner, float basePower) : owner(owner), basePower(basePower)
{

}
// TODO вынести подсчет бонуса для defense, hp и power в одну функцию
float Attacker::calculateBonusPower() const
{
	float sum = 0;
	Container *container = owner->getContainer();
	if (container)
		for (Actor **i = container->getInventory().begin(); i != container->getInventory().end(); ++i)
		{
			Equipment *equipment = (*i)->getEquipment();
			if (equipment && equipment->isEquipped())
				sum += equipment->getPowerBonus();
		}

	return sum;
}

float Attacker::power() const
{
	return basePower + calculateBonusPower();
}

void Attacker::attack(Actor *target) const
{
	Destructible *targetDestr = target->getDestructible();
	if (targetDestr && !targetDestr->isDead())
	{
		float damage = power() - targetDestr->getDefense();
		if (damage > 0)
			engine.getGui()->message(owner == engine.getPlayer() ? TCODColor::red : TCODColor::lightGrey,
				"%s attacks %s for %g hit points.", owner->getName(), target->getName(), damage);
		else
			engine.getGui()->message(TCODColor::lightGrey,
				"%s attacks %s but it has no effect!", owner->getName(), target->getName());
		targetDestr->takeDamage(power());
	}
	else
		engine.getGui()->message(TCODColor::lightGrey,
				"%s attacks %s in vain.", owner->getName(), target->getName());
}

