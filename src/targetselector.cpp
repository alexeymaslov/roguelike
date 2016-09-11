#include "targetselector.hpp"

#include "engine.hpp"

TargetSelector::TargetSelector(TargetSelector::SelectorType type, float range) :
	type(type), range(range)
{

}

void TargetSelector::selectTargets(Actor *wearer, TCODList<Actor *> & list)
{
	switch(type)
	{
		case CLOSEST_MONSTER:
		{
			Actor *closestMonster = engine.getClosestMonster(wearer->x, wearer->y, range);
			if (closestMonster)
				list.push(closestMonster);
		}
		break;
		case SELECTED_MONSTER:
		{
			int x;
			int y;
			engine.gui->message(TCODColor::cyan, "Left-click to select an enemy,\nor right-click to cancel.");
			if (engine.pickATile(x, y, 0.0f, range))
			{
				Actor *actor = engine.getActor(x, y);
				if (actor)
					list.push(actor);
			}
		}
		break;
		case WEARER_RANGE:
		{
			for (Actor **i = engine.actors.begin();
				i != engine.actors.end(); ++i)
			{
				Actor *actor = *i;
				if (actor != wearer && actor->destructible && !actor->destructible->isDead()
					&& actor->getDistance(wearer->x, wearer->y) <= range)
					list.push(actor);
			}
		}
		break;
		case SELECTED_RANGE:
		{
			int x;
			int y;
			engine.gui->message(TCODColor::cyan, "Left-click to select an enemy,\nor right-click to cancel.");
			if (engine.pickATile(x, y, range))
				for (Actor **i = engine.actors.begin();
					i != engine.actors.end(); ++i)
				{
					Actor *actor = *i;
					if (actor->destructible && !actor->destructible->isDead()
						&& actor->getDistance(wearer->x, wearer->y) <= range)
						list.push(actor);
				}
		}
		break;
	}
	if (list.isEmpty())
		engine.gui->message(TCODColor::lightGrey, "No enemy is close enough");
}