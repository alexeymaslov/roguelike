#include "pickable.hpp"

#include "actor.hpp"
#include "engine.hpp"

Pickable::Pickable(TargetSelector *selector,  Effect *effect) :
	selector(selector), effect(effect)
{

}

Pickable::~Pickable()
{
	if (selector) delete selector;
	if (effect) delete effect;
}

bool Pickable::pick(Actor *owner, Actor *wearer)
{
	if (wearer->container && wearer->container->add(owner))
	{
		if (owner->equipment && !owner->equipment->getEquippedInSlot(owner->equipment->slot, wearer))
			owner->equipment->equip(owner, wearer);

		engine.actors.remove(owner);
		return true;
	}
	return false;
}

void Pickable::drop(Actor *owner, Actor *wearer)
{
	if (wearer->container)
	{
		if (owner->equipment)
			owner->equipment->dequip(owner);

		wearer->container->remove(owner);
		engine.actors.push(owner);
		owner->x = wearer->x;
		owner->y = wearer->y;
		engine.gui->message(TCODColor::lightGrey, "%s drops a %s.",
			wearer->name, owner->name);
	}
}

bool Pickable::use(Actor *owner, Actor *wearer)
{
	if (owner->equipment)
	{
		owner->equipment->toggleEquip(owner, wearer);
		return true;
	}
	
	TCODList<Actor *> list;
	if (selector)
		selector->selectTargets(wearer, list);
	else
		list.push(wearer);
	bool succeed = false;
	for (Actor **i = list.begin(); i != list.end(); ++i)
		if (effect->applyTo(*i))
			succeed = true;


	if (succeed)
		if (wearer->container)
		{
			wearer->container->remove(owner);
			delete owner;
		}
	return succeed;
}