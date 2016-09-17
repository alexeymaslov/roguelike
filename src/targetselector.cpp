#include "targetselector.hpp"

#include "engine.hpp"

TargetSelector::TargetSelector(TargetSelector::SelectorType type, float range) :
	type(type), range(range)
{

}

void TargetSelector::selectTargets(TCODList<Actor *> & list)
{
	switch(type)
	{
		case ClosestMonster:
		{
			Actor *closestMonster = engine.getClosestMonster(wearer->getX(), wearer->getY(), range);
			if (closestMonster)
				list.push(closestMonster);
		}
		break;
		case SelectedMonster:
		{
			int x;
			int y;
			engine.getGui()->message(TCODColor::cyan, "Left-click to select an enemy,\nor right-click to cancel.");
			if (engine.pickATile(x, y, 0.0f, range))
			{
				Actor *actor = engine.getActor(x, y);
				if (actor)
					list.push(actor);
			}
		}
		break;
		case WearerRange:
		{
			for (Actor **i = engine.getActors().begin();
				i != engine.getActors().end(); ++i)
			{
				Actor *actor = *i;
				if (actor != wearer && actor->getDestructible() && !actor->getDestructible()->isDead()
					&& actor->getDistanceTo(wearer->getX(), wearer->getY()) <= range)
					list.push(actor);
			}
		}
		break;
		case SelectedRange:
		{
			int x;
			int y;
			engine.getGui()->message(TCODColor::cyan, "Left-click to select an enemy,\nor right-click to cancel.");
			if (engine.pickATile(x, y, range))
				for (Actor **i = engine.getActors().begin();
					i != engine.getActors().end(); ++i)
				{
					Actor *actor = *i;
					if (actor->getDestructible() && !actor->getDestructible()->isDead()
						&& actor->getDistanceTo(x, y) <= range)
						list.push(actor);
				}
		}
		break;
	}
	if (list.isEmpty())
		engine.getGui()->message(TCODColor::lightGrey, "No enemy is close enough");
}