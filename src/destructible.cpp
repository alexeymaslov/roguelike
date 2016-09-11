#include <stdio.h>

#include "destructible.hpp"

#include "actor.hpp"
#include "engine.hpp"

Destructible::Destructible(float maxHp, float defense, const char *corpseName, int xp) :
	baseMaxHp(maxHp), hp(maxHp), baseDefense(defense), xp(xp)
{
	if (corpseName) this->corpseName = strdup(corpseName);
}

Destructible::~Destructible()
{
	free(corpseName);
}

float Destructible::takeDamage(Actor *owner, float damage)
{
	damage -= defense(owner);
	if (damage > 0)
	{
		hp -= damage;
		if (hp <= 0)
			die(owner);
	}
	else
		damage = 0;
	return damage;
}

void Destructible::die(Actor *owner)
{
	owner->ch = '%';
	owner->col = TCODColor::darkRed;
	owner->name = corpseName;
	owner->blocks = false;
	engine.sendToBack(owner);
}

float Destructible::maxHp(Actor *owner) const
{
	return baseMaxHp + calculateBonusHp(owner);
}

float Destructible::defense(Actor *owner) const
{
	return baseDefense + calculateBonusDefense(owner);
}

float Destructible::calculateBonusDefense(Actor *owner) const
{
	if (owner != engine.player)
		return 0;

	float sum = 0;
	auto &items = engine.player->container->inventory;
	for (Actor **i = items.begin(); i != items.end(); ++i)
		if ((*i)->equipment)
			sum += (*i)->equipment->defenseBonus;

	return sum;
}

float Destructible::calculateBonusHp(Actor *owner) const
{
	if (owner != engine.player)
		return 0;

	float sum = 0;
	auto &items = engine.player->container->inventory;
	for (Actor **i = items.begin(); i != items.end(); ++i)
		if ((*i)->equipment)
			sum += (*i)->equipment->hpBonus;

	return sum;
}

float Destructible::heal(float amount, Actor *owner)
{
	hp += amount;
	if (hp > maxHp(owner))
	{
		amount -= hp - maxHp(owner);
		hp = maxHp(owner);
	}
	return amount;
}

PlayerDestructible::PlayerDestructible(float maxHp, float defense, const char *corpseName) :
	Destructible(maxHp, defense, corpseName, 0)
{

}

MonsterDestructible::MonsterDestructible(float maxHp, float defense, const char *corpseName, int xp) :
	Destructible(maxHp, defense, corpseName, xp)
{
	
}

void MonsterDestructible::die(Actor *owner)
{
	engine.gui->message(TCODColor::lightGrey, "%s is dead. You gain %d xp", owner->name, xp);
	engine.player->destructible->xp += xp;
	Destructible::die(owner);
}

void PlayerDestructible::die(Actor *owner)
{
	engine.gui->message(TCODColor::red, "You died!");
	Destructible::die(owner);
	engine.gameStatus = Engine::DEFEAT;
}