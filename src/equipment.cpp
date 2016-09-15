#include "equipment.hpp"

#include "actor.hpp"
#include "engine.hpp"

Equipment::Equipment(Slot slot, float powerBonus, float defenseBonus, float hpBonus) : 
	slot(slot), equipped(false), powerBonus(powerBonus), 
	defenseBonus(defenseBonus), hpBonus(hpBonus)
{

}

void Equipment::toggleEquip(Actor *owner, Actor *wearer)
{
	if (equipped)
		dequip(owner);
	else
		equip(owner, wearer);
}

Actor *Equipment::getEquippedInSlot(Equipment::Slot slot, Actor *wearer)
{
	for (Actor **i = wearer->container->inventory.begin();
		i != wearer->container->inventory.end(); ++i)
	{
		Actor *item = *i;
		if (item->equipment && item->equipment->slot == slot && item->equipment->equipped)
			return item;
	}
	return nullptr;
}

void Equipment::equip(Actor *owner, Actor *wearer)
{
	Actor *oldEquipment = getEquippedInSlot(slot, wearer);
	if (oldEquipment)
		oldEquipment->equipment->dequip(oldEquipment);

	equipped = true;
	engine.gui->message(TCODColor::lightGreen, "Equipped %s on %s.", owner->name, getSlotAsChar(slot));

}

void Equipment::dequip(Actor *owner)
{
	if (!equipped) return;

	equipped = false;
	engine.gui->message(TCODColor::lightYellow, "Dequipped %s on %s.", owner->name, getSlotAsChar(slot));
}

const char *Equipment::getSlotAsChar(Equipment::Slot slot)
{
	switch(slot)
	{
	case RIGHT_HAND:
		return "right hand";
	break;
	case LEFT_HAND:
		return "left hand";
	break;
	}
}