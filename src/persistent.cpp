#include "libtcod.hpp"
#include "persistent.hpp"
#include "destructible.hpp"
#include "attacker.hpp"
#include "ai.hpp"
#include "equipment.hpp"
#include "pickable.hpp"
#include "container.hpp"
#include "actor.hpp"
#include "map.hpp"
#include "gui.hpp"
#include "engine.hpp"

// Здесь реализуется функционал сохранения/загрузки

void Engine::save()
{
	if (player->destructible->isDead())
		TCODSystem::deleteFile("game.sav");
	else
	{
		TCODZip zip;
		zip.putInt(map->width);
		zip.putInt(map->height);
		map->save(zip);
		player->save(zip);
		stairs->save(zip);
		zip.putInt(actors.size() - 2);
		for (Actor **i = actors.begin(); i != actors.end(); ++i)
			if (*i != player && *i != stairs)
				(*i)->save(zip);
		gui->save(zip);
		zip.saveToFile("game.sav");
	}
}

void Engine::load()
{
	engine.gui->menu.clear();
	engine.gui->menu.addItem(Menu::NEW_GAME, "New game");
	if (TCODSystem::fileExists("game.sav"))
		engine.gui->menu.addItem(Menu::CONTINUE, "Continue");
	engine.gui->menu.addItem(Menu::EXIT, "Exit");

	Menu::MenuItemCode menuItem = engine.gui->menu.pick();
	if (menuItem == Menu::EXIT || menuItem == Menu::NONE)
		exit(0);
	else if (menuItem == Menu::NEW_GAME)
	{
		engine.term();
		engine.init();
	}
	else
	{
		TCODZip zip;
		engine.term();
		zip.loadFromFile("game.sav");
		int width = zip.getInt();
		int height = zip.getInt();
		map = new Map(width, height);
		map->load(zip);
		player = new Actor(0, 0, 0, NULL, TCODColor::white);
		player->load(zip);
		actors.push(player);
		stairs = new Actor(0, 0, 0, NULL, TCODColor::white);
		stairs->load(zip);
		actors.push(stairs);
		int nbActors = zip.getInt();
		while (nbActors > 0)
		{
			Actor *actor = new Actor(0, 0, 0, NULL, TCODColor::white);
			actor->load(zip);
			actors.push(actor);
			--nbActors;
		}
		gui->load(zip);
		gameStatus = STARTUP;
	}
}

void Gui::save(TCODZip &zip)
{	
	zip.putInt(log.size());
	for (Message **i = log.begin(); i != log.end(); ++i)
	{
		zip.putString((*i)->text);
		zip.putColor(&(*i)->col);
	}
}

void Gui::load(TCODZip &zip)
{
	int nbMessages = zip.getInt();
	while (nbMessages > 0)
	{
		const char *text = zip.getString();
		TCODColor col = zip.getColor();
		message(col, text);
		--nbMessages;
	}
}

void Map::save(TCODZip &zip)
{
	zip.putInt(seed);
	zip.putInt(currentScentValue);
	for (int i = 0; i < width * height; ++i)
	{
		zip.putInt(tiles[i].explored);
		zip.putInt(tiles[i].scent);
	}
}

void Map::load(TCODZip &zip)
{
	seed = zip.getInt();
	init(false);
	currentScentValue = zip.getInt();
	for (int i = 0; i < width * height; ++i)
	{
		tiles[i].explored = zip.getInt();
		tiles[i].scent = zip.getInt();
	}
}

void Actor::save(TCODZip &zip)
{
	zip.putInt(x);
	zip.putInt(y);
	zip.putInt(ch);
	zip.putColor(&col);
	zip.putString(name);
	zip.putInt(blocks);
	zip.putInt(attacker != NULL);
	zip.putInt(destructible != NULL);
	zip.putInt(ai != NULL);
	zip.putInt(pickable != NULL);
	zip.putInt(container != NULL);
	zip.putInt(equipment != NULL);
	if (attacker) attacker->save(zip);
	if (destructible) destructible->save(zip);
	if (ai) ai->save(zip);
	if (pickable) pickable->save(zip);
	if (container) container->save(zip);
	if (equipment) equipment->save(zip);
}

void Actor::load(TCODZip &zip)
{
	x = zip.getInt();
	y = zip.getInt();
	ch = zip.getInt();
	col = zip.getColor();
	name = strdup(zip.getString());
	blocks = zip.getInt();
	bool hasAttacker = zip.getInt();
	bool hasDestructible = zip.getInt();
	bool hasAi = zip.getInt();
	bool hasPickable = zip.getInt();
	bool hasContainer = zip.getInt();
	bool hasEquipment = zip.getInt();
	if (hasAttacker)
	{
		attacker = new Attacker(0.0f);
		attacker->load(zip);
	}
	if (hasDestructible)
		destructible = Destructible::create(zip);
	if (hasAi)
		ai = Ai::create(zip);
	if (hasPickable)
	{
		pickable = new Pickable(NULL, NULL);
		pickable->load(zip);
	}
	if (hasContainer)
	{
		container = new Container(0);
		container->load(zip);
	}
	if (hasEquipment)
	{
		equipment = new Equipment(Equipment::RIGHT_HAND);
		equipment->load(zip);
	}
}

void Attacker::save(TCODZip &zip)
{
	zip.putFloat(basePower);
}

void Attacker::load(TCODZip &zip)
{
	basePower = zip.getFloat();
}

void Container::save(TCODZip &zip)
{
	zip.putInt(size);
	zip.putInt(inventory.size());
	for (Actor **i = inventory.begin(); i != inventory.end(); ++i)
		(*i)->save(zip);
}

void Container::load(TCODZip &zip)
{
	size = zip.getInt();
	int nbActors = zip.getInt();
	while (nbActors > 0)
	{
		Actor *actor = new Actor(0, 0, 0, NULL, TCODColor::white);
		actor->load(zip);
		inventory.push(actor);
		--nbActors;
	}
}

void Destructible::save(TCODZip &zip)
{
	zip.putFloat(baseMaxHp);
	zip.putFloat(hp);
	zip.putFloat(baseDefense);
	zip.putString(corpseName);
	zip.putInt(xp);
}

void Destructible::load(TCODZip &zip)
{
	baseMaxHp = zip.getFloat();
	hp = zip.getFloat();
	baseDefense = zip.getFloat();
	corpseName = strdup(zip.getString());
	xp = zip.getInt();
}

void PlayerDestructible::save(TCODZip &zip)
{
	zip.putInt(PLAYER);
	Destructible::save(zip);
}

void MonsterDestructible::save(TCODZip &zip)
{
	zip.putInt(MONSTER);
	Destructible::save(zip);
}

Destructible *Destructible::create(TCODZip &zip)
{
	DestructibleType type = (DestructibleType) zip.getInt();
	Destructible *destructible = NULL;
	switch (type)
	{
		case MONSTER:
			destructible = new MonsterDestructible(0, 0, NULL, 0);
		break;
		case PLAYER:
			destructible = new PlayerDestructible(0, 0, NULL);
		break;
	}
	destructible->load(zip);
	return destructible;
}

void MonsterAi::save(TCODZip &zip)
{
	zip.putInt(MONSTER);
}

void MonsterAi::load(TCODZip &zip)
{
	
}

void PlayerAi::save(TCODZip &zip)
{
	zip.putInt(PLAYER);
	zip.putInt(xpLevel);
}

void PlayerAi::load(TCODZip &zip)
{
	xpLevel = zip.getInt();
}

Ai *Ai::create(TCODZip &zip)
{
	AiType type = (AiType) zip.getInt();
	Ai *ai = NULL;
	switch(type)
	{
		case PLAYER: 
			ai = new PlayerAi();
		break;
		case MONSTER: 
			ai = new MonsterAi();
		break;
		case TEMPORARY_AI:
		{
			ai = TemporaryAi::create(zip);
			return ai;
		}
		break;
	}
	ai->load(zip);
	return ai;
}

TemporaryAi *TemporaryAi::create(TCODZip &zip)
{
	TemporaryAiType type = (TemporaryAiType) zip.getInt();
	TemporaryAi *ai = NULL;
	switch (type)
	{
		case CONFUSED_MONSTER:
			ai = new ConfusedMonsterAi(0);
		break;
	}
	ai->load(zip);
	return ai;
}

void ConfusedMonsterAi::save(TCODZip &zip)
{
	zip.putInt(CONFUSED_MONSTER);
	TemporaryAi::save(zip);
}

void TemporaryAi::save(TCODZip &zip)
{
	zip.putInt(nbTurns);
	zip.putInt(oldAi != NULL);
	if (oldAi) oldAi->save(zip);
}

void TemporaryAi::load(TCODZip &zip)
{
	nbTurns = zip.getInt();
	bool hasOldAi = zip.getInt();
	if (hasOldAi)
		oldAi = Ai::create(zip);
}

void ConfusedMonsterAi::load(TCODZip &zip)
{
	TemporaryAi::load(zip);
}

void Equipment::save(TCODZip &zip)
{
	zip.putInt(slot);
	zip.putInt(equipped);
	zip.putFloat(powerBonus);
	zip.putFloat(defenseBonus);
	zip.putFloat(hpBonus);
}

void Equipment::load(TCODZip &zip)
{
	slot = (Slot) zip.getInt();
	equipped = zip.getInt();
	powerBonus = zip.getFloat();
	defenseBonus = zip.getFloat();
	hpBonus = zip.getFloat();
}

void Pickable::save(TCODZip &zip)
{
	zip.putInt(selector != NULL);
	zip.putInt(effect != NULL);
	if (selector) selector->save(zip);
	if (effect) effect->save(zip);
}

void Pickable::load(TCODZip &zip)
{
	bool hasSelector = zip.getInt();
	bool hasEffect = zip.getInt();
	if (hasSelector)
	{
		selector = new TargetSelector(TargetSelector::CLOSEST_MONSTER, 0);
		selector->load(zip);
	}
	if (hasEffect)
		effect = Effect::create(zip);
}

void TargetSelector::save(TCODZip &zip)
{
	zip.putInt(type);
	zip.putFloat(range);
}

void TargetSelector::load(TCODZip &zip)
{
	type = (SelectorType) zip.getInt();
	range = zip.getFloat();
}

Effect *Effect::create(TCODZip &zip)
{
	EffectType type = (EffectType) zip.getInt();
	Effect *effect = NULL;
	switch (type)
	{
		case HEALTH:
			effect = new HealthEffect(0, NULL);
		break;
		case AI_CHANGE:
			effect = new AiChangeEffect(NULL, NULL);
		break;
	}
	effect->load(zip);
	return effect;
}

void HealthEffect::save(TCODZip &zip)
{
	zip.putInt(HEALTH);
	zip.putFloat(amount);
	zip.putString(message);
}

void HealthEffect::load(TCODZip &zip)
{
	amount = zip.getFloat();
	message = strdup(zip.getString());
}

void AiChangeEffect::save(TCODZip &zip)
{
	zip.putInt(AI_CHANGE);
	zip.putString(message);
	zip.putInt(newAi != NULL);
	if (newAi) newAi->save(zip);
}

void AiChangeEffect::load(TCODZip &zip)
{
	message = strdup(zip.getString());
	bool hasNewAi = zip.getInt();
	if (hasNewAi)
		newAi = TemporaryAi::create(zip);
}