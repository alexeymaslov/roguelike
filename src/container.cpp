#include "container.hpp"

#include "actor.hpp"

Container::Container(Actor *owner, int maxSize) : owner(owner), maxSize(maxSize)
{

}

Container::~Container()
{
	inventory.clearAndDelete();
}

bool Container::add(Actor *actor)
{
	if (maxSize < 0 || inventory.size() >= maxSize) return false;
	inventory.push(actor);
	if (actor->getEquipment())
		actor->getEquipment()->setContainer(this);
	if (actor->getPickable())
		actor->getPickable()->setContainer(this);
	return true;
}

void Container::remove(Actor *actor)
{
	inventory.remove(actor);
}
