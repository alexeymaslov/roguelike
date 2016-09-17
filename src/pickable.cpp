#include "pickable.hpp"

#include "actor.hpp"
#include "engine.hpp"

Pickable::Pickable(Actor *owner, TargetSelector *selector,  Effect *effect) :
	owner(owner), selector(selector), effect(effect), container(nullptr)
{

}

Pickable::~Pickable()
{
	if (selector) delete selector;
	if (effect) delete effect;
}

void Pickable::setContainer(Container *container)
{
	this->container = container;
	if (selector)
		selector->setWearer(container->getOwner());
}

bool Pickable::pick(Actor *wearer)
{
	container = wearer->getContainer();
	if (container && container->add(owner))
	{
		Equipment *equipment = owner->getEquipment();
		if (equipment)
			equipment->setContainer(container);

		engine.getActors().remove(owner);
		return true;
	}
	return false;
}

void Pickable::drop()
{
	if (container)
	{
		Equipment *equipment = owner->getEquipment();
		if (equipment)
		{
			equipment->dequip();
			equipment->eraseContainer();
		}

		container->remove(owner);
		engine.getActors().push(owner);
		Actor *wearer = container->getOwner();
		owner->setCoords(wearer->getX(), wearer->getY());
		engine.getGui()->message(TCODColor::lightGrey, "%s drops a %s.",
			wearer->getName(), owner->getName());
	}
}

bool Pickable::use()
{
	Equipment *equipment = owner->getEquipment();
	if (equipment)
	{
		equipment->toggleEquip();
		return true;
	}
	Actor *wearer = container->getOwner();
	TCODList<Actor *> list;
	if (selector)
		selector->selectTargets(list);
	else
		list.push(wearer);
	bool succeed = false;
	for (Actor **i = list.begin(); i != list.end(); ++i)
		if (effect->applyTo(*i))
			succeed = true;


	if (succeed)
		if (wearer->getContainer())
		{
			wearer->getContainer()->remove(owner);
			delete owner;
		}
	return succeed;
}