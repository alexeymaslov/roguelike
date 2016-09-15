#pragma once

#include "persistent.hpp"
#include "targetselector.hpp"
#include "effect.hpp"

class Actor;

class Pickable : public Persistent
{
public:
	Pickable(TargetSelector *selector = nullptr, Effect *effect = nullptr);
	virtual ~Pickable();

	bool pick(Actor *owner, Actor *wearer);
	void drop(Actor *owner, Actor *wearer);
	virtual bool use(Actor *owner, Actor *wearer);

	void load(TCODZip &zip);
	void save(TCODZip &zip);

protected:
	TargetSelector *selector;
	Effect *effect;
};