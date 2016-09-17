#include "ai.hpp"

#include "actor.hpp"
#include "engine.hpp"

#include "stdio.h"
#include "math.h"

Ai::Ai(Actor *owner) : owner(owner)
{

}

PlayerAi::PlayerAi(Actor *owner) : Ai(owner), xpLevel(1)
{

}

int PlayerAi::getNextLevelXp()
{
	static const int LevelUpBase = 200;
	static const int LevelUpFactor = 150;
	return LevelUpBase + xpLevel * LevelUpFactor;
}

void PlayerAi::levelUp()
{
	Gui *gui = engine.getGui();
	gui->message(TCODColor::yellow, "Your battle skills grow stronger! You reached level %d", xpLevel);
	Menu &menu = gui->getMenu();
	menu.clear();
	menu.addItem(Menu::Constitution, "Constitution (+20HP)");
	menu.addItem(Menu::Strength, "Strength (+1 attack)");
	menu.addItem(Menu::Agility, "Agility (+1 defense)");
	Menu::MenuItemCode menuItem = menu.pick(Menu::Pause);
	switch (menuItem)
	{
		case Menu::Constitution:
		{
			owner->getDestructible()->addMaxHp(20);
		}
		break;
		case Menu::Strength:
			owner->getAttacker()->addBasePower(1);
		break;
		case Menu::Agility:
			owner->getDestructible()->addBaseDefense(1);
		break;
		default:
		break;
	}
}

void PlayerAi::update()
{
	Destructible *destructible = owner->getDestructible();
	if (destructible && destructible->isDead())
		return;

	int levelUpXp = getNextLevelXp();
	if (destructible->getXp() >= levelUpXp)
	{
		++xpLevel;
		destructible->addXp(-levelUpXp);
		levelUp();
	}

	int dx = 0;
	int dy = 0;
	switch (engine.getLastKey().vk)
	{
		case TCODK_KP7:
		{
			dx = -1;
			dy = -1;
		}
		break;
		case TCODK_KP8: case TCODK_UP:
			dy = -1;
		break;
		case TCODK_KP9:
		{
			dx = 1;
			dy = -1;
		}
		break;
		case TCODK_KP4: case TCODK_LEFT:
			dx = -1; 
		break;
		case TCODK_KP5:
			engine.setGameStatus(Engine::NewTurn);
		break;
		case TCODK_KP6: case TCODK_RIGHT:
			dx = 1;
		break;
		case TCODK_KP1:
		{
			dx = -1;
			dy = 1;
		}
		break;
		case TCODK_KP2: case TCODK_DOWN:
			dy = 1;
		break;
		case TCODK_KP3:
		{
			dx = 1;
			dy = 1;
		}
		break;
		case TCODK_CHAR:
			handleActionKey(engine.getLastKey().c); break;
		default: break;
	}

	if (dx != 0 || dy != 0)
	{
		engine.setGameStatus(Engine::NewTurn);
		if (moveOrAttack(owner->getX() + dx, owner->getY() + dy))
			engine.getMap()->computeFov();
	}
}

bool PlayerAi::moveOrAttack(int targetx, int targety)
{
	if (engine.getMap()->isWall(targetx, targety))
		return false;

	for (Actor **iterator = engine.getActors().begin(); iterator != engine.getActors().end(); ++iterator)
	{
		Actor *actor = *iterator;
		if (actor->getDestructible() && !actor->getDestructible()->isDead() 
			&& actor->getX() == targetx && actor->getY() == targety)
		{
			owner->getAttacker()->attack(actor);
			return false;
		}
	}
	for (Actor **iterator = engine.getActors().begin(); iterator != engine.getActors().end(); ++iterator)
	{
		Actor *actor = *iterator;
		if (((actor->getDestructible() && actor->getDestructible()->isDead()) || actor->getPickable()) 
			&& actor->getX() == targetx && actor->getY() == targety)
			engine.getGui()->message(TCODColor::lightGrey, "There's a %s here", actor->getName());
	}
	owner->setCoords(targetx, targety);
	return true;
}

void PlayerAi::handleActionKey(int ascii)
{
	switch (ascii)
	{
		case 'g': // Подобрать предмет
		{
			bool found = false;
			for (Actor **it = engine.getActors().begin(); it != engine.getActors().end(); ++it)
			{
				Actor *actor = *it;
				if (actor->getPickable() && actor->getX() == owner->getX() && actor->getY() == owner->getY())
				{
					if (actor->getPickable()->pick(owner))
					{
						found = true;
						engine.getGui()->message(TCODColor::lightGrey, "You pick up the %s.",
							actor->getName());
						break;
					}
					else if (!found)
					{
						found = true;
						engine.getGui()->message(TCODColor::lightGrey, "Your inventory is full.");
					}
				}
			}
			if (!found)
				engine.getGui()->message(TCODColor::lightGrey,"There's nothing here that you can pick up.");
			engine.setGameStatus(Engine::NewTurn);
		}
		break;
		case 'i': // Показать инвентарь
		{
			Actor *actor = choseFromInventory();
			if (actor)
			{
				actor->getPickable()->use();
				engine.setGameStatus(Engine::NewTurn);
			}
		}
		break;
		case 'd': // Выкинуть предмет
		{
			Actor *actor = choseFromInventory();
			if (actor)
			{
				actor->getPickable()->drop();
				engine.setGameStatus(Engine::NewTurn);
			}
		}
		break;
		case '>': // Спуститься по лестнице ниже
		{
			if (engine.getStairs()->getX() == owner->getX() && engine.getStairs()->getY() == owner->getY())
				engine.nextLevel();
			else
				engine.getGui()->message(TCODColor::lightGrey, "There are no stairs here.");
		}
		break;
	}
}

Actor *PlayerAi::choseFromInventory()
{
	static const int InventoryWidth = 50;
	static const int InventoryHeight = 28;
	static TCODConsole con(InventoryWidth, InventoryHeight);

	con.setDefaultForeground(TCODColor(200, 180, 50));
	con.printFrame(0, 0, InventoryWidth, InventoryHeight, true, 
		TCOD_BKGND_DEFAULT, "inventory");

	con.setDefaultForeground(TCODColor::white);
	int shortcut = 'a';
	int y = 1;
	for (Actor **it = owner->getContainer()->getInventory().begin();
		it != owner->getContainer()->getInventory().end(); ++it)
	{
		Actor *actor = *it;
		const char *text = actor->getName();
		char buf[80];
		if (actor->getEquipment() && actor->getEquipment()->isEquipped())
		{
			strcpy(buf, text);
			strcat(buf, " (on ");
			strcat(buf, Equipment::getSlotAsChar(actor->getEquipment()->getSlot()));
			strcat(buf, ")");
			text = buf;
		}

		con.print(2, y, "(%c) %s", shortcut, text);
		++y;
		++shortcut;
	}

	TCODConsole::blit(&con, 0, 0, InventoryWidth, InventoryHeight,
		TCODConsole::root, engine.getScreenWidth() / 2 - InventoryWidth / 2,
		engine.getScreenHeight() / 2 - InventoryHeight / 2);
	TCODConsole::flush();

	TCOD_key_t key;
	TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS, &key, nullptr, true);
	if (key.vk == TCODK_CHAR)
	{
		int actorIndex = key.c - 'a';
		if (actorIndex >= 0 && actorIndex < owner->getContainer()->getInventory().size())
			return owner->getContainer()->getInventory().get(actorIndex);
	}
	return nullptr;
}

MonsterAi::MonsterAi(Actor *owner) : Ai(owner)
{

}

void MonsterAi::update()
{
	if (owner->getDestructible() && owner->getDestructible()->isDead())
		return;

	moveOrAttack(engine.getPlayer());
}

void MonsterAi::moveOrAttack(Actor *target)
{
	if (engine.getMap()->isInFov(owner->getX(), owner->getY()))
	{
		if (owner->getDistanceTo(target->getX(), target->getY()) >= 2)
			moveToTarget(target);
		else // атакуем в ближнем бою
			if (owner->getAttacker() && target->getDestructible() && !target->getDestructible()->isDead())
				owner->getAttacker()->attack(target);
		return;
	}
	else if (target == engine.getPlayer()) // игрок невидим, используем запах
	{
		unsigned int bestLevel = 0;
		int bestCellIndex = -1;
		static int tdx[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
		static int tdy[8] = {-1, -1, -1, 0, 0, 1, 1, 1}; 
		for (int i = 0; i < 8; ++i)
		{
			int cellx = owner->getX() + tdx[i];
			int celly = owner->getY() + tdy[i];
			if (engine.getMap()->canWalk(cellx, celly))
			{
				unsigned int cellScent = engine.getMap()->getScent(cellx, celly);
				if (cellScent > engine.getMap()->getCurrentScentValue() - ScentThreshold
					&& cellScent > bestLevel)
				{
					bestLevel = cellScent;
					bestCellIndex = i;
				}
			}
		}
		if (bestCellIndex != -1)
			owner->setCoords(owner->getX() + tdx[bestCellIndex], owner->getY() + tdy[bestCellIndex]);
	}
}

void MonsterAi::moveToTarget(Actor *target)
{
	TCODMap pathMap(engine.getMap()->getWidth(), engine.getMap()->getHeight());
	pathMap.copy(engine.getMap()->getMap());
	for (Actor **i = engine.getActors().begin(); i != engine.getActors().end(); ++i)
	{
		Actor *actor = *i;
		if (actor->getBlocks() && actor != owner && actor != target)
			pathMap.setProperties(actor->getX(), actor->getY(), true, false);
	}
	const float diagonalStepCost = 1.0;
	TCODPath path(&pathMap, diagonalStepCost);
	path.compute(owner->getX(), owner->getY(), target->getX(), target->getY());
	if (!path.isEmpty())
	{
		int x;
		int y;
		bool walkable = path.walk(&x, &y, true);
		if (walkable)
			owner->setCoords(x, y);
	}
	else // используем старый метод
		moveToCoords(target->getX(), target->getY());
}

void MonsterAi::moveToCoords(int targetx, int targety)
{
	int dx = targetx - owner->getX();
	int dy = targety - owner->getY();
	int stepdx = (dx > 0 ? 1 : -1);
	int stepdy = (dy > 0 ? 1 : -1);
	float distance = sqrtf(dx * dx + dy * dy);
	dx = (int)(round(dx / distance));
	dy = (int)(round(dy / distance));
	if (engine.getMap()->canWalk(owner->getX() + dx, owner->getY() + dy))
		owner->setCoords(owner->getX() + dx, owner->getY() + dy);
	else if (engine.getMap()->canWalk(owner->getX() + stepdx, owner->getY()))
		owner->setCoords(owner->getX() + stepdx, owner->getY());
	else if (engine.getMap()->canWalk(owner->getX(), owner->getY() + stepdy))
		owner->setCoords(owner->getX(), owner->getY() + stepdy);
}

TemporaryAi::TemporaryAi(Actor *owner, int nbTurns) : Ai(owner), nbTurns(nbTurns), oldAi(nullptr)
{

}

void TemporaryAi::update()
{
	--nbTurns;
	if (nbTurns == 0)
	{
		owner->setAi(oldAi);
		delete this;
	}
}

void TemporaryAi::applyTo(Actor *actor)
{
	owner = actor;
	oldAi = actor->getAi();
	actor->setAi(this);
}

ConfusedMonsterAi::ConfusedMonsterAi(Actor *owner, int nbTurns) : TemporaryAi(owner, nbTurns)
{

}

void ConfusedMonsterAi::update()
{
	TCODRandom *rng = TCODRandom::getInstance();
	// can get (0,0) and attack itself, nice feature
	int dx = rng->getInt(-1, 1);
	int dy = rng->getInt(-1, 1);
	int destx = owner->getX() + dx;
	int desty = owner->getY() + dy;
	if (engine.getMap()->canWalk(destx, desty))
		owner->setCoords(destx, desty);
	else
	{
		Actor *actor = engine.getActor(destx, desty);
		if (actor)
			owner->getAttacker()->attack(actor);
	}

	TemporaryAi::update();
}