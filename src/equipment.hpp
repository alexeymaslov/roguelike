// TODO2 derive from Persistent
#pragma once

#include "persistent.hpp"

class Actor;

class Equipment
{
public:
	enum Slot
	{
		RIGHT_HAND, LEFT_HAND
	} slot;
	bool equipped;

	float powerBonus;
	float defenseBonus;
	float hpBonus;

	Equipment(Slot slot, float powerBonus = 0, float defenseBonus = 0, float hpBonus = 0);
	void toggleEquip(Actor *owner, Actor *wearer);
	void equip(Actor *owner, Actor *wearer);
	void dequip(Actor *owner);
	Actor *getEquippedInSlot(Equipment::Slot slot, Actor *wearer);

	void load(TCODZip &zip) {}
	void save(TCODZip &zip) {}

	static const char *getSlotAsChar(Slot slot);
};