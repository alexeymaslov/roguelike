#pragma once

#include "libtcod.hpp"
#include "persistent.hpp"

class Actor;

class TargetSelector : public Persistent
{
public:	
	enum SelectorType
	{
		ClosestMonster,
		SelectedMonster,
		WearerRange,
		SelectedRange
	};
	TargetSelector(SelectorType type, float range);

	void setWearer(Actor *wearer) { this->wearer = wearer; };
	// Выбранные цели добавляются в список
	void selectTargets(TCODList<Actor *> & list);

	void load(TCODZip &zip);
	void save(TCODZip &zip);
	
protected:
	Actor *wearer;
	SelectorType type;
	float range;
};