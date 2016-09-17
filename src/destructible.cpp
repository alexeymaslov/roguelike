#include <stdio.h>

#include "destructible.hpp"

#include "actor.hpp"
#include "engine.hpp"

Destructible::Destructible(Actor *owner, float maxHp, float defense, const char *corpseName, int xp) :
	owner(owner), baseMaxHp(maxHp), hp(maxHp), baseDefense(defense), xp(xp)
{
	if (corpseName) this->corpseName = strdup(corpseName);
}

Destructible::~Destructible()
{
	free(corpseName);
}

float Destructible::takeDamage(float damage)
{
	damage -= getDefense();
	if (damage > 0)
	{
		hp -= damage;
		if (hp <= 0)
			die();
	}
	else
		damage = 0;
	return damage;
}

void Destructible::die()
{
	owner->setCh('%');
	owner->setColor(TCODColor::darkRed);
	owner->setName(corpseName);
	owner->setBlocks(false);
	// Нужно, чтобы на клетке отрисовывался находящийся там живой объект, а не труп
	engine.sendToBack(owner);
}

float Destructible::maxHp() const
{
	return baseMaxHp + calculateBonusHp();
}

void Destructible::addMaxHp(int hp)
{
	baseMaxHp += hp;
	this->hp += hp;
}

float Destructible::getDefense() const
{
	return baseDefense + calculateBonusDefense();
}

// TODO вынести подсчет бонуса для defense, hp и power в одну функцию
float Destructible::calculateBonusDefense() const
{
	float sum = 0;
	Container *container = owner->getContainer();
	if (container)
		for (Actor **i = container->getInventory().begin(); i != container->getInventory().end(); ++i)
		{
			Equipment *equipment = (*i)->getEquipment();
			if (equipment && equipment->isEquipped())
				sum += equipment->getDefenseBonus();
		}

	return sum;
}

// TODO вынести подсчет бонуса для defense, hp и power в одну функцию
float Destructible::calculateBonusHp() const
{
	float sum = 0;
	Container *container = owner->getContainer();
	if (container)
		for (Actor **i = container->getInventory().begin(); i != container->getInventory().end(); ++i)
		{
			Equipment *equipment = (*i)->getEquipment();
			if (equipment && equipment->isEquipped())
				sum += equipment->getHpBonus();
		}

	return sum;
}

float Destructible::heal(float amount)
{
	hp += amount;
	float maxHp = this->maxHp();
	if (hp > maxHp)
	{
		amount = amount - (hp - maxHp);
		hp = maxHp;
	}
	return amount;
}

PlayerDestructible::PlayerDestructible(Actor *owner, float maxHp, float defense, const char *corpseName) :
	Destructible(owner, maxHp, defense, corpseName, 0)
{

}

MonsterDestructible::MonsterDestructible(Actor *owner, float maxHp, float defense, 
	const char *corpseName, int xp) :
	Destructible(owner, maxHp, defense, corpseName, xp)
{
	
}

void MonsterDestructible::die()
{
	engine.getGui()->message(TCODColor::lightGrey, "%s is dead. You gain %d xp", owner->getName(), xp);
	// TODO добавить в параметр убийцу, чтобы передавать ему опыт
	engine.getPlayer()->getDestructible()->addXp(xp);
	Destructible::die();
}

void PlayerDestructible::die()
{
	engine.getGui()->message(TCODColor::red, "You died!");
	Destructible::die();
	engine.setGameStatus(Engine::Defeat);
}