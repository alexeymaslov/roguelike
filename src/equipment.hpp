// TODO2 derive from Persistent
#pragma once

#include "persistent.hpp"

class Actor;
class Container;

class Equipment : public Persistent
{
public:
	enum Slot
	{
		RightHand, LeftHand
	};
	Equipment(Actor *owner, Slot slot, float powerBonus = 0, float defenseBonus = 0, float hpBonus = 0);

	Slot getSlot() { return slot; };
	bool isEquipped() { return equipped; };
	float getPowerBonus() { return powerBonus; };
	float getDefenseBonus() { return defenseBonus; };
	float getHpBonus() { return hpBonus; };
	void setContainer(Container *container) { this->container = container; };
	void eraseContainer() { this->container = nullptr; };

	void toggleEquip();
	void equip();
	void dequip();
	static Actor *getEquippedInSlot(Equipment::Slot slot, Container *container);
	static const char *getSlotAsChar(Slot slot);

	void load(TCODZip &zip);
	void save(TCODZip &zip);

private:
	Actor *owner;
	Container *container;
	Slot slot;
	bool equipped;
	float powerBonus;
	float defenseBonus;
	float hpBonus;
};