#pragma once

#include "persistent.hpp"

class Actor;

class Attacker : public Persistent
{
public:
	Attacker(Actor *owner, float basePower);
	float power() const;
	void addBasePower(float amount) { basePower += amount; };
	void attack(Actor *target) const;

	void save(TCODZip &zip);
	void load(TCODZip &zip);

private:
	Actor *owner;
	float basePower;
	float calculateBonusPower() const;
};