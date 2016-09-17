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
	if (player->getDestructible()->isDead())
		TCODSystem::deleteFile("game.sav");
	else
	{
		TCODZip zip;
		zip.putInt(map->getWidth());
		zip.putInt(map->getHeight());
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
	gui->getMenu().clear();
	gui->getMenu().addItem(Menu::NewGame, "New game");
	if (TCODSystem::fileExists("game.sav"))
		gui->getMenu().addItem(Menu::Continue, "Continue");
	gui->getMenu().addItem(Menu::Exit, "Exit");

	Menu::MenuItemCode menuItem = gui->getMenu().pick();
	if (menuItem == Menu::Exit || menuItem == Menu::None)
		exit(0);
	else if (menuItem == Menu::NewGame)
	{
		term();
		init();
	}
	else
	{
		TCODZip zip;
		term();
		zip.loadFromFile("game.sav");
		int width = zip.getInt();
		int height = zip.getInt();
		map = new Map(width, height);
		map->load(zip);
		player = new Actor(0, 0, 0, nullptr, TCODColor::white);
		player->load(zip);
		actors.push(player);
		stairs = new Actor(0, 0, 0, nullptr, TCODColor::white);
		stairs->load(zip);
		actors.push(stairs);
		int nbActors = zip.getInt();
		while (nbActors > 0)
		{
			Actor *actor = new Actor(0, 0, 0, nullptr, TCODColor::white);
			actor->load(zip);
			actors.push(actor);
			--nbActors;
		}
		gui->load(zip);
		gameStatus = StartUp;
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
	zip.putColor(&color);
	zip.putString(name);
	zip.putInt(blocks);
	zip.putInt(attacker != nullptr);
	zip.putInt(destructible != nullptr);
	zip.putInt(ai != nullptr);
	zip.putInt(pickable != nullptr);
	zip.putInt(container != nullptr);
	zip.putInt(equipment != nullptr);
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
	color = zip.getColor();
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
		attacker = new Attacker(this, 0.0f);
		attacker->load(zip);
	}
	if (hasDestructible)
	{
		destructible = Destructible::create(zip);
		destructible->setOwner(this);
	}
	if (hasAi)
	{
		ai = Ai::create(zip);
		ai->setOwner(this);
	}
	if (hasPickable)
	{
		pickable = new Pickable(this, nullptr, nullptr);
		pickable->load(zip);
	}
	if (hasContainer)
	{
		container = new Container(this, 0);
		container->load(zip);
	}
	if (hasEquipment)
	{
		equipment = new Equipment(this, Equipment::RightHand);
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
	zip.putInt(maxSize);
	zip.putInt(inventory.size());
	for (Actor **i = inventory.begin(); i != inventory.end(); ++i)
		(*i)->save(zip);
}

void Container::load(TCODZip &zip)
{
	maxSize = zip.getInt();
	int nbActors = zip.getInt();
	while (nbActors > 0)
	{
		Actor *actor = new Actor(0, 0, 0, nullptr, TCODColor::white);
		actor->load(zip);
		add(actor);
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
	zip.putInt(Player);
	Destructible::save(zip);
}

void MonsterDestructible::save(TCODZip &zip)
{
	zip.putInt(Monster);
	Destructible::save(zip);
}

Destructible *Destructible::create(TCODZip &zip)
{
	DestructibleType type = (DestructibleType) zip.getInt();
	Destructible *destructible = nullptr;
	switch (type)
	{
		case Monster:
			destructible = new MonsterDestructible(nullptr, 0, 0, nullptr, 0);
		break;
		case Player:
			destructible = new PlayerDestructible(nullptr, 0, 0, nullptr);
		break;
	}
	destructible->load(zip);
	return destructible;
}

void MonsterAi::save(TCODZip &zip)
{
	zip.putInt(Monster);
}

void MonsterAi::load(TCODZip &zip)
{
	
}

void PlayerAi::save(TCODZip &zip)
{
	zip.putInt(Player);
	zip.putInt(xpLevel);
}

void PlayerAi::load(TCODZip &zip)
{
	xpLevel = zip.getInt();
}

Ai *Ai::create(TCODZip &zip)
{
	AiType type = (AiType) zip.getInt();
	Ai *ai = nullptr;
	switch(type)
	{
		case Player: 
			ai = new PlayerAi(nullptr);
		break;
		case Monster: 
			ai = new MonsterAi(nullptr);
		break;
		case TemporaryAi:
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
	TemporaryAi *ai = nullptr;
	switch (type)
	{
		case ConfusedMonster:
			ai = new ConfusedMonsterAi(nullptr, 0);
		break;
	}
	ai->load(zip);
	return ai;
}

void ConfusedMonsterAi::save(TCODZip &zip)
{
	zip.putInt(ConfusedMonster);
	TemporaryAi::save(zip);
}

void TemporaryAi::save(TCODZip &zip)
{
	zip.putInt(nbTurns);
	zip.putInt(oldAi != nullptr);
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
	zip.putInt(selector != nullptr);
	zip.putInt(effect != nullptr);
	if (selector) selector->save(zip);
	if (effect) effect->save(zip);
}

void Pickable::load(TCODZip &zip)
{
	bool hasSelector = zip.getInt();
	bool hasEffect = zip.getInt();
	if (hasSelector)
	{
		selector = new TargetSelector(TargetSelector::ClosestMonster, 0);
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
	Effect *effect = nullptr;
	switch (type)
	{
		case Health:
			effect = new HealthEffect(0, nullptr);
		break;
		case AiChange:
			effect = new AiChangeEffect(nullptr, nullptr);
		break;
	}
	effect->load(zip);
	return effect;
}

void HealthEffect::save(TCODZip &zip)
{
	zip.putInt(Health);
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
	zip.putInt(AiChange);
	zip.putString(message);
	zip.putInt(newAi != nullptr);
	if (newAi) newAi->save(zip);
}

void AiChangeEffect::load(TCODZip &zip)
{
	message = strdup(zip.getString());
	bool hasNewAi = zip.getInt();
	if (hasNewAi)
		newAi = TemporaryAi::create(zip);
}