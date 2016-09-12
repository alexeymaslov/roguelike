#pragma once

#include "libtcod.hpp"
#include "persistent.hpp"

class Actor;

class TargetSelector : public Persistent
{
public:
	enum SelectorType
	{
		CLOSEST_MONSTER,
		SELECTED_MONSTER,
		WEARER_RANGE,
		SELECTED_RANGE
	};
	TargetSelector(SelectorType type, float range);
	void selectTargets(Actor *wearer, TCODList<Actor *> & list);

	void load(TCODZip &zip);
	void save(TCODZip &zip);
	
protected:
	SelectorType type;
	float range;
};