#pragma once

#include "libtcod.hpp"

class Actor;

class TargetSelector
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

protected:
	SelectorType type;
	float range;
};