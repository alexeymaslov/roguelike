#pragma once

#include "persistent.hpp"

class Actor;

class Attacker : public Persistent
{
public:
	float basePower;
	float power(Actor *owner) const;

	Attacker(float basePower);
	void attack(Actor *owner, Actor *target);

	void save(TCODZip &zip);
	void load(TCODZip &zip);

protected:
	float calculateBonusPower(Actor *owner) const;
};