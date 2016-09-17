#include "equipment.hpp"

#include "actor.hpp"
#include "engine.hpp"

Equipment::Equipment(Actor *owner, Slot slot, float powerBonus, float defenseBonus, float hpBonus) : 
	owner(owner), slot(slot), equipped(false), powerBonus(powerBonus), 
	defenseBonus(defenseBonus), hpBonus(hpBonus), container(nullptr)
{

}

void Equipment::toggleEquip()
{
	if (container)
	{
		if (equipped)
			dequip();
		else
			equip();
	}
}

Actor *Equipment::getEquippedInSlot(Equipment::Slot slot, Container *container)
{
	for (Actor **i = container->getInventory().begin();
		i != container->getInventory().end(); ++i)
	{
		Actor *item = *i;
		Equipment *equipment = item->getEquipment();
		if (equipment && equipment->getSlot() == slot && equipment->isEquipped())
			return item;
	}
	return nullptr;
}

void Equipment::equip()
{
	Actor *oldEquipment = getEquippedInSlot(slot, container);
	if (oldEquipment)
		oldEquipment->getEquipment()->dequip();

	equipped = true;
	engine.getGui()->message(TCODColor::lightGreen, "Equipped %s on %s.", owner->getName(), getSlotAsChar(slot));

}

void Equipment::dequip()
{
	if (!equipped) return;

	equipped = false;
	engine.getGui()->message(TCODColor::lightYellow, "Dequipped %s on %s.", owner->getName(), getSlotAsChar(slot));
}

const char *Equipment::getSlotAsChar(Equipment::Slot slot)
{
	switch(slot)
	{
	case RightHand:
		return "right hand";
	break;
	case LeftHand:
		return "left hand";
	break;
	}
}