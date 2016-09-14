#include <algorithm>
#include <math.h>
#include <stdio.h>

#include "map.hpp"

#include "ai.hpp"
#include "engine.hpp"

static const int ROOM_MAX_SIZE = 12;
static const int ROOM_MIN_SIZE = 6;

class BspListener : public ITCODBspCallback
{
public:
	BspListener(Map &map) : map(map), roomNum(0) {}
	bool visitNode(TCODBsp *node, void *userData);

private:
	Map &map;
	int roomNum;
	int lastx; // center of the last room
	int lasty;
};

bool BspListener::visitNode(TCODBsp *node, void *userData)
{
	if (node->isLeaf())
	{
		bool withActors = (bool) userData;
		// dig a room
		int w = map.rng->getInt(ROOM_MIN_SIZE, node->w - 2);
		int h = map.rng->getInt(ROOM_MIN_SIZE, node->h - 2);
		int x = map.rng->getInt(node->x + 1, node->x + node->w - w - 1);
		int y = map.rng->getInt(node->y + 1, node->y + node->h - h - 1);
		
		map.createRoom(roomNum == 0, x, y, x + w - 1, y + h - 1, withActors);

		if (roomNum != 0)
		{
			// dig a corridor from last room
			map.dig(lastx, lasty, x + w / 2, lasty);
			map.dig(x + w / 2, lasty, x + w / 2, y + h / 2);
		}
		lastx = x + w / 2;
		lasty = y + h / 2;
		++roomNum;
	}
	return true;
}

Map::Map(int width, int height) : width(width), height(height)
{
	seed = TCODRandom::getInstance()->getInt(0,0x7FFFFFFF);
}

void Map::init(bool withActors)
{
	
	currentScentValue = SCENT_THRESHOLD;
	rng = new TCODRandom(seed, TCOD_RNG_CMWC);
	tiles = new Tile[width * height];
	map = new TCODMap(width, height);
	initItemOrMonsterChances();

	if (engine.level % 2 - 1)
		bspGeneration(withActors);
	else
		cellularAutomataGeneration(withActors);
}

Map::~Map()
{
	delete rng;
	delete [] tiles;
	delete map;
}

bool Map::isWall(int x, int y) const
{
	return !map->isWalkable(x,y);
}

bool Map::isExplored(int x, int y) const
{
	return tiles[x + y * width].explored;
}

void Map::bspGeneration(bool withActors)
{
	TCODBsp bsp(0, 0, width, height);
	bsp.splitRecursive(rng, 8, ROOM_MAX_SIZE, ROOM_MAX_SIZE, 1.5f, 1.5f);
	BspListener listener(*this);
	bsp.traverseInvertedLevelOrder(&listener, (void *) withActors);
}

void Map::dig(int x1, int y1, int x2, int y2)
{
	if (x2 < x1) std::swap(x2, x1);
	if (y2 < y1) std::swap(y2, y1);

	for (int tilex = x1; tilex <= x2; ++tilex)
		for (int tiley = y1; tiley <= y2; ++tiley)
			map->setProperties(tilex, tiley, true, true);
}

void Map::createRoom(bool first, int x1, int y1, int x2, int y2, bool withActors)
{
	dig(x1, y1, x2, y2);

	if (!withActors)
		return;

	if (first)
	{
		// put the player in the first room
		engine.player->x = (x1 + x2) / 2;
		engine.player->y = (y1 + y2) / 2;
	}
	else
	{
		engine.stairs->x = (x1 + x2) / 2;
		engine.stairs->y = (y1 + y2) / 2;

		TCODRandom *rng = TCODRandom::getInstance();
		int nbMonsters = rng->getInt(0, maxAmountRoomMonsters);
		while (nbMonsters > 0)
		{
			int x = rng->getInt(x1, x2);
			int y = rng->getInt(y1, y2);
			if (canWalk(x, y))
				addMonster(x, y);
			--nbMonsters;
		}

		int nbItems = rng->getInt(0, maxAmountRoomItems);
		while (nbItems > 0)
		{
			int x = rng->getInt(x1, x2);
			int y = rng->getInt(y1, y2);
			if (canWalk(x, y))
				addItem(x, y);
			--nbItems;
		}
	}
}

void Map::render() const
{
	static const TCODColor darkWall(0, 0, 100);
	static const TCODColor darkGround(50, 50, 150);
	static const TCODColor lightWall(130,110,50);
	static const TCODColor lightGround(200,180,50);
	for (int x = 0; x < engine.cameraWidth; ++x)
		for (int y = 0; y < engine.cameraHeight; ++y)
		{
			int mapx = x + engine.camerax;
			int mapy = y + engine.cameray;
			if (isInFov(mapx, mapy))
				TCODConsole::root->setCharBackground(x, y, isWall(mapx, mapy) ? lightWall : lightGround);
			else if (isExplored(mapx, mapy))
				TCODConsole::root->setCharBackground(x, y, isWall(mapx, mapy) ? darkWall : darkGround);
		}

	// show scent value 
	/*for (int x = 0; x < engine.cameraWidth; ++x)
		for (int y = 0; y < engine.cameraHeight; ++y)
		{
			int scent = SCENT_THRESHOLD - (currentScentValue - getScent(x, y));
			scent = CLAMP(0, 10, scent);
			int mapx = x + engine.camerax;
			int mapy = y + engine.cameray;
			float sc = scent * 0.1f;
			if (isInFov(mapx, mapy))
				TCODConsole::root->setCharBackground(x, y, isWall(mapx,mapy) ? 
					lightWall : TCODColor::lightGrey * sc);
			else if (isExplored(mapx, mapy))
				TCODConsole::root->setCharBackground(x, y, isWall(mapx,mapy) ? 
					darkWall : TCODColor::lightGrey * sc);
			else if (!isWall(mapx, mapy))
				TCODConsole::root->setCharBackground(x, y, TCODColor::white * sc);
		}*/
}

bool Map::isInFov(int x, int y) const
{
	if (x < 0 || x > width || y < 0 || y > height)
		return false;
	
	if (map->isInFov(x,y))
	{
		tiles[x + y * width].explored = true;
		return true;
	}
	return false;
}


// TODO при загрузке не учитывается старый scent
void Map::computeFov()
{
	map->computeFov(engine.player->x, engine.player->y, engine.fovRadius);

	// update scent
	for (int x = 0; x < width; ++x)
		for (int y = 0; y < height; ++y)
			if (isInFov(x,y))
			{
				unsigned int oldScent = getScent(x, y);
				int dx = x - engine.player->x;
				int dy = y - engine.player->y;
				int distance = (int)sqrt(dx * dx + dy * dy);
				unsigned int newScent = currentScentValue - distance;
				if (newScent > oldScent)
					tiles[x + y * width].scent = newScent;
			}
}

bool Map::canWalk(int x, int y) const
{
	if (isWall(x, y))
		return false;
	for (Actor **iterator = engine.actors.begin(); iterator != engine.actors.end(); ++iterator)
	{
		Actor *actor = *iterator;
		if (actor->blocks && actor->x == x && actor->y == y)
			return false;
	}
	return true;
}

Map::GeneratedActorType Map::rollActorType(const std::vector<Pair<GeneratedActorType, int>> &pairs) const
{
	int chancesSum = 0;
	for (auto i = pairs.begin(); i != pairs.end(); ++i)
		chancesSum += i->second;


	TCODRandom *rng = TCODRandom::getInstance();
	int roll = rng-> getInt(0, chancesSum);

	int sum = 0;
	for (auto i = pairs.begin(); i != pairs.end(); ++i)
	{
		GeneratedActorType choice = i->first;
		sum += i->second;
		if (roll <= sum)
			return choice;
	}
}

// Chance of monsters and item spawn depends on dungeon level
// Pair <int chance, int level>
int Map::getChance(const std::vector<Pair<int, int>> &pairs) const
{
	for (auto i = pairs.rbegin(); i != pairs.rend(); ++i)
		if (engine.level >= i->second)
			return i->first;
	return 0;
}

void Map::initItemOrMonsterChances()
{
	// TODO: use map here
	static const std::vector<Pair<int, int>> maxMonstersFromLevel = { {2, 1}, {3, 4}, {5, 6} };
	maxAmountRoomMonsters = getChance(maxMonstersFromLevel);

	static const std::vector<Pair<int, int>> maxItemsFromLevel = { {1, 1}, {2, 4} };
	maxAmountRoomItems = getChance(maxItemsFromLevel);

	static const std::vector<Pair<int, int>> trollChancesFromLevel = { {15, 3}, {30, 5}, {60, 7} };
	monsterChances = { {ORC, 80}, {TROLL, getChance(trollChancesFromLevel)} };

	static const std::vector<Pair<int, int>> lightningBoltChancesFromLevel = { {25, 4} };
	static const std::vector<Pair<int, int>> fireballChancesFromLevel = { {25, 6} };
	static const std::vector<Pair<int, int>> confuserChancesFromLevel = { {10, 2} };
	static const std::vector<Pair<int, int>> swordChancesFromLevel = { {5, 4} };
	static const std::vector<Pair<int, int>> shieldChancesFromLevel = { {15, 8} };
	itemChances = { {HEALER, 35}, {LIGHTNING_BOLT, getChance(lightningBoltChancesFromLevel)}, 
		{FIREBALL, getChance(fireballChancesFromLevel) }, {CONFUSER, getChance(confuserChancesFromLevel)}, 
		{SWORD, getChance(swordChancesFromLevel)}, {SHIELD, getChance(shieldChancesFromLevel)}};
}

void Map::addMonster(int x, int y)
{
	GeneratedActorType type = rollActorType(monsterChances);
	switch (type)
	{
		case ORC:
		{
			Actor *orc = new Actor(x, y, 'o', "orc", TCODColor::desaturatedGreen);
			orc->destructible = new MonsterDestructible(10, 0, "dead orc", 20);
			orc->attacker = new Attacker(3);
			orc->ai = new MonsterAi();
			engine.actors.push(orc);
		}
		break;
		case TROLL:
		{
			Actor *troll = new Actor(x, y, 'T', "troll", TCODColor::darkerGreen);
			troll->destructible = new MonsterDestructible(16, 1, "troll carcass", 40);
			troll->attacker = new Attacker(4);
			troll->ai = new MonsterAi();
			engine.actors.push(troll);
		}
		break;
		default:
		break;
	}
}

void Map::addItem(int x, int y)
{
	GeneratedActorType type = rollActorType(itemChances);
	switch (type)
	{
		case HEALER:
		{
			Actor *healthPotion = new Actor(x, y, '!', "health potion",
				TCODColor::violet);
			healthPotion->blocks = false;
			healthPotion->pickable = new Pickable(NULL, 
				new HealthEffect(4, "%s is healed for %g health"));
			engine.actors.push(healthPotion);
		}
		break;
		case LIGHTNING_BOLT:
		{
			Actor *scrollOfLightningBolt = new Actor(x, y, '#', "scroll of lightning bolt",
				TCODColor::lightYellow);
			scrollOfLightningBolt->blocks = false;
			scrollOfLightningBolt->pickable = new Pickable(
				new TargetSelector(TargetSelector::CLOSEST_MONSTER, 5), 
				new HealthEffect(-20, "A lighting bolt strikes the %s with a loud thunder!\n"
				"The damage is %g hit points."));
			engine.actors.push(scrollOfLightningBolt);
		}
		break;
		case FIREBALL:
		{
			Actor *scrollOfFireball = new Actor(x, y, '#', "scroll of fireball",
				TCODColor::lightYellow);
			scrollOfFireball->blocks = false;
			scrollOfFireball->pickable = new Pickable(
				new TargetSelector(TargetSelector::SELECTED_RANGE, 3), 
				new HealthEffect(-12, "The %s gets burned for %g hit points."));
			engine.actors.push(scrollOfFireball);
		}
		break;
		case CONFUSER:
		{
			Actor *scrollOfConfusion = new Actor(x, y, '#', "scroll of confusion",
				TCODColor::lightYellow);
			scrollOfConfusion->blocks = false;
			scrollOfConfusion->pickable = new Pickable(
				new TargetSelector(TargetSelector::SELECTED_MONSTER, 5), 
				new AiChangeEffect(new ConfusedMonsterAi(10), 
					"The eyes of the %s look vacant,\nas he starts to stumble around!"));
			engine.actors.push(scrollOfConfusion);
		}
		break;
		case SWORD:
		{
			Actor *sword = new Actor(x, y, '/', "sword", TCODColor::sky);
			sword->blocks = false;
			sword->equipment = new Equipment(Equipment::RIGHT_HAND, 3);
			sword->pickable = new Pickable();
			engine.actors.push(sword);
		}
		break;
		case SHIELD:
		{
			Actor *shield = new Actor(x, y, '[', "shield",TCODColor::darkerOrange);
			shield->blocks = false;
			shield->equipment = new Equipment(Equipment::LEFT_HAND, 0, 1);
			shield->pickable = new Pickable();
			engine.actors.push(shield);
		}
		break;
		default:
		break;
	}
}

unsigned int Map::getScent(int x, int y) const
{
	return tiles[x + y * width].scent;
}

void Map::cellularAutomataGeneration(bool withActors)
{
	do
	{
		randomFillMap();
		for (int i = 0; i < 4; ++i)
			makeCaverns(4, 2);
		for (int i = 0; i < 3; ++i)
			makeCaverns(4, -2);
	} while (!floodFill());

	if (withActors)
	{
		int x;
		int y;
		TCODRandom *rng = TCODRandom::getInstance();
		do
		{
			x = rng->getInt(1, width);
			y = rng->getInt(1, height);
		}
		while (!map->isWalkable(x, y));
		engine.player->x = x;
		engine.player->y = y;

		do
		{
			x = rng->getInt(1, width);
			y = rng->getInt(1, height);
		}
		while (!map->isWalkable(x, y));
		engine.stairs->x = x;
		engine.stairs->y = y;

		populateCaves();
	}
}

void Map::randomFillMap()
{
	const int chanceOfWall = 45;
	// Пустое место в центре, чтобы уменьшить выроятность генерации вертикальной стены
	// возможно не спасет от горизонтальных стен
	const int middleWallLength = width / 20; 
	const int middleWallHeight = height / 20;
	const int middlex = width / 2;
	const int middley = height / 2;
	// Границы карты пусть будут стенами
	for (int x = 1; x < width - 1; ++x)
		for (int y = 1; y < height - 1; ++y)
		{
			if ((x < middlex + middleWallLength && x > middlex - middleWallLength) && 
				(y < middley + middleWallHeight && y > middley - middleWallHeight))
				map->setProperties(x, y, true, true);
			else if (rng->getInt(1, 100) > chanceOfWall)
				map->setProperties(x, y, true, true);
		}
}

void Map::makeCaverns(int nbWalls1Tile, int nbWalls2Tile)
{
	bool isWall[width][height];
	for (int x = 0; x < width; ++x)
	{
		isWall[x][0] = true;
		isWall[x][height - 1] = true;
	}
	for (int y = 0; y < height; ++y)
	{
		isWall[0][y] = true;
		isWall[width - 1][y] = true;
	}

	for (int x = 1; x < width - 1; ++x)
		for (int y = 1; y < height - 1; ++y)
		{
			bool walkable = map->isWalkable(x, y);
			int nbWallsNear = getAmountOfWallsNear(x, y);
			int nbWalls2TilesNear = getAmountOfWallsNear(x, y, 2, 2);
			if ((!walkable && nbWallsNear >= nbWalls1Tile)
			|| (walkable && nbWallsNear >= nbWalls1Tile + 1)
			|| (nbWalls2TilesNear <= nbWalls2Tile))
				isWall[x][y] = true;
			else
				isWall[x][y] = false;
		}
	for (int x = 1; x < width - 1; ++x)
		for (int y = 1; y < height - 1; ++y)
			map->setProperties(x, y, !isWall[x][y], !isWall[x][y]);
}

int Map::getAmountOfWallsNear(int cx, int cy, int dx, int dy) const
{
	int i = 0;

	for (int x = cx - dx; x <= cx + dx; ++x)
		for (int y = cy - dy; y <= cy + dy; ++y)
			if (y != cy || x != cx)
				if (x < 0 || y < 0 || x >= width || y >= height || 
					!map->isWalkable(x, y))
					++i;

	return i;
}

bool Map::floodFill()
{	
	// TODO тяжело было передавать двумерный массив в функцию, надо переделать
	bool isWall[width * height];
	for (int x = 0; x < width; ++x)
		for (int y = 0; y < height; ++y)
			isWall[x + y * width] = !map->isWalkable(x, y);

	static const float WallThreshold = 55;
	// 3 раза пытаемся найти подходящую точку для заливки
	int counter = 3;
	do
	{
		int x;
		int y;
		TCODRandom *rng = TCODRandom::getInstance();
		do
		{
			x = rng->getInt(1, width);
			y = rng->getInt(1, height);
		}
		while (!map->isWalkable(x, y));
		fill(x, y, isWall);

		for (int x = 0; x < width; ++x)
			for (int y = 0; y < height; ++y)
				if (!isWall[x + y * width])
					map->setProperties(x, y, false, false);

		if (calcPercentOfWalls() <= WallThreshold)
			return true;
		--counter;
	} while (counter > 0);
	return false;

}

void Map::fill(int x, int y, bool *isWall)
{
	if (!isWall[x + y * width])
	{
		isWall[x + y * width] = true;
		fill(x + 1, y, isWall);
		fill(x - 1, y, isWall);
		fill(x, y + 1, isWall);
		fill(x, y - 1, isWall);
	}
}

float Map::calcPercentOfWalls() const
{
	int nbWalls = 0;
	int nbCells = width * height;
	for (int x = 0; x < width; ++x)
		for (int y = 0; y < height; ++y)
			if (!map->isWalkable(x, y))
				++nbWalls;
	return ((float)nbWalls) / nbCells;
}

void Map::populateCaves()
{
	float averageRoomLength = ((float)(ROOM_MAX_SIZE + ROOM_MIN_SIZE) / 2) * 2 / 3;
	float averageRoomSquare =  averageRoomLength * averageRoomLength;
	// Сколько примерно комнат было бы сгенерировано с помощью bsp генерации
	int nbRooms = (int) (width * height / averageRoomSquare) - 1;
	int stepx = (int) width / averageRoomLength;
	int stepy = (int) height / averageRoomLength;
	int x = 0;
	int y = 0;

	while (nbRooms)
	{
		TCODRandom *rng = TCODRandom::getInstance();
		int nbMonsters = rng->getInt(0, maxAmountRoomMonsters);
		while (nbMonsters > 0)
		{
			int cx = rng->getInt(x, x + stepx);
			int cy = rng->getInt(y, y + stepy);
			if (canWalk(cx, cy))
				addMonster(cx, cy);
			--nbMonsters;
		}

		int nbItems = rng->getInt(0, maxAmountRoomItems);
		while (nbItems > 0)
		{
			int cx = rng->getInt(x, x + stepx);
			int cy = rng->getInt(y, y + stepy);
			if (canWalk(cx, cy))
				addItem(cx, cy);
			--nbItems;
		}
		--nbRooms;
		x += stepx;
		if (x >= width)
		{
			x -= width;
			y += stepy;
		}
	}
}