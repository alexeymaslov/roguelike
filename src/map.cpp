#include <algorithm>
#include <math.h>
#include <stdio.h>

#include "map.hpp"

#include "ai.hpp"
#include "engine.hpp"

static const int RoomMaxSize = 12;
static const int RoomMinSize = 6;

class BspListener : public ITCODBspCallback
{
public:
	BspListener(Map &map) : map(map), roomNum(0) {}
	bool visitNode(TCODBsp *node, void *userData);

private:
	Map &map;
	int roomNum;
	// центр предыдущей комнаты для прокапывания тоннеля 
	int lastx; 
	int lasty;
};

bool BspListener::visitNode(TCODBsp *node, void *userData)
{
	if (node->isLeaf())
	{
		bool withActors = (bool) userData;
		// dig a room
		int w = map.rng->getInt(RoomMinSize, node->w - 2);
		int h = map.rng->getInt(RoomMinSize, node->h - 2);
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
	
	currentScentValue = ScentThreshold;
	rng = new TCODRandom(seed, TCOD_RNG_CMWC);
	tiles = new Tile[width * height];
	map = new TCODMap(width, height);
	initItemOrMonsterChances();

	// Каждый четный уровень генерируется с помощью bsp
	// а нечетный с помощию клеточного автомата
	if (engine.getLevel() % 2 - 1)
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
	bsp.splitRecursive(rng, 8, RoomMaxSize, RoomMaxSize, 1.5f, 1.5f);
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
	// В первую комнату помещаем игрока
	if (first)
		engine.getPlayer()->setCoords((x1 + x2) / 2, (y1 + y2) / 2);
	else
	{
		engine.getStairs()->setCoords((x1 + x2) / 2, (y1 + y2) / 2);
		// Размещаем монстров
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
		// Размещаем предметы
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
	static const TCODColor darkWall = TCODColor::darkerHan;
	static const TCODColor darkGround = TCODColor::desaturatedHan;
	static const TCODColor lightWall = TCODColor::darkestGreen;
	static const TCODColor lightGround = TCODColor::desaturatedGreen;
	for (int x = 0; x < engine.getCameraWidth(); ++x)
		for (int y = 0; y < engine.getCameraHeight(); ++y)
		{
			int mapx = x + engine.getCamerax();
			int mapy = y + engine.getCameray();
			if (isInFov(mapx, mapy))
				TCODConsole::root->setCharBackground(x, y, isWall(mapx, mapy) ? lightWall : lightGround);
			else if (isExplored(mapx, mapy))
				TCODConsole::root->setCharBackground(x, y, isWall(mapx, mapy) ? darkWall : darkGround);
		}

	// Тут можно включить показ показателя scent на клетках
	/*for (int x = 0; x < engine.cameraWidth(); ++x)
		for (int y = 0; y < engine.cameraHeight(); ++y)
		{
			int mapx = x + engine.getCamerax();
			int mapy = y + engine.getCameray();
			int scent = ScentThreshold - (currentScentValue - getScent(mapx, mapy));
			scent = CLAMP(0, 10, scent);
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
	map->computeFov(engine.getPlayer()->getX(), engine.getPlayer()->getY(), engine.getFovRadius());

	// обновляем scent
	for (int x = 0; x < width; ++x)
		for (int y = 0; y < height; ++y)
			if (isInFov(x,y))
			{
				unsigned int oldScent = getScent(x, y);
				int dx = x - engine.getPlayer()->getX();
				int dy = y - engine.getPlayer()->getY();
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
	for (Actor **iterator = engine.getActors().begin(); iterator != engine.getActors().end(); ++iterator)
	{
		Actor *actor = *iterator;
		if (actor->getBlocks() && actor->getX() == x && actor->getY() == y)
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

// Шансы на появление монстра или предмета завясят от уровня
// Pair <int chance, int level>
int Map::getChance(const std::vector<Pair<int, int>> &pairs) const
{
	for (auto i = pairs.rbegin(); i != pairs.rend(); ++i)
		if (engine.getLevel() >= i->second)
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
	monsterChances = { {Orc, 80}, {Troll, getChance(trollChancesFromLevel)} };

	static const std::vector<Pair<int, int>> lightningBoltChancesFromLevel = { {25, 4} };
	static const std::vector<Pair<int, int>> fireballChancesFromLevel = { {25, 6} };
	static const std::vector<Pair<int, int>> confuserChancesFromLevel = { {10, 2} };
	static const std::vector<Pair<int, int>> swordChancesFromLevel = { {5, 4} };
	static const std::vector<Pair<int, int>> shieldChancesFromLevel = { {15, 8} };
	itemChances = { {Healer, 35}, {LightningBolt, getChance(lightningBoltChancesFromLevel)}, 
		{Fireball, getChance(fireballChancesFromLevel) }, {Confuser, getChance(confuserChancesFromLevel)}, 
		{Sword, getChance(swordChancesFromLevel)}, {Shield, getChance(shieldChancesFromLevel)}};
}

void Map::addMonster(int x, int y)
{
	GeneratedActorType type = rollActorType(monsterChances);
	switch (type)
	{
		case Orc:
		{
			Actor *orc = new Actor(x, y, 'o', "orc", TCODColor::brass);
			orc->setDestructible(new MonsterDestructible(orc, 10, 0, "dead orc", 20));
			orc->setAttacker(new Attacker(orc, 3));
			orc->setAi(new MonsterAi(orc));
			engine.getActors().push(orc);
		}
		break;
		case Troll:
		{
			Actor *troll = new Actor(x, y, 'T', "troll", TCODColor::darkerCrimson);
			troll->setDestructible(new MonsterDestructible(troll, 16, 1, "troll carcass", 40));
			troll->setAttacker(new Attacker(troll, 4));
			troll->setAi(new MonsterAi(troll));
			engine.getActors().push(troll);
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
		case Healer:
		{
			Actor *healthPotion = new Actor(x, y, '!', "health potion",
				TCODColor::lightLime);
			healthPotion->setBlocks(false);
			healthPotion->setPickable(new Pickable(healthPotion, nullptr, 
				new HealthEffect(4, "%s is healed for %g health")));
			engine.getActors().push(healthPotion);
		}
		break;
		case LightningBolt:
		{
			Actor *scrollOfLightningBolt = new Actor(x, y, '#', "scroll of lightning bolt",
				TCODColor::gold);
			scrollOfLightningBolt->setBlocks(false);
			scrollOfLightningBolt->setPickable(new Pickable(scrollOfLightningBolt,
				new TargetSelector(TargetSelector::ClosestMonster, 5), 
				new HealthEffect(-20, "A lighting bolt strikes the %s with a loud thunder!\n"
				"The damage is %g hit points.")));
			engine.getActors().push(scrollOfLightningBolt);
		}
		break;
		case Fireball:
		{
			Actor *scrollOfFireball = new Actor(x, y, '#', "scroll of fireball",
				TCODColor::peach);
			scrollOfFireball->setBlocks(false);
			scrollOfFireball->setPickable(new Pickable(scrollOfFireball,
				new TargetSelector(TargetSelector::SelectedRange, 3), 
				new HealthEffect(-12, "The %s gets burned for %g hit points.")));
			engine.getActors().push(scrollOfFireball);
		}
		break;
		case Confuser:
		{
			Actor *scrollOfConfusion = new Actor(x, y, '#', "scroll of confusion",
				TCODColor::celadon);
			scrollOfConfusion->setBlocks(false);
			scrollOfConfusion->setPickable(new Pickable(scrollOfConfusion,
				new TargetSelector(TargetSelector::SelectedMonster, 5), 
				new AiChangeEffect(new ConfusedMonsterAi(nullptr, 10), 
					"The eyes of the %s look vacant,\nas he starts to stumble around!")));
			engine.getActors().push(scrollOfConfusion);
		}
		break;
		case Sword:
		{
			Actor *sword = new Actor(x, y, '/', "sword", TCODColor::sky);
			sword->setBlocks(false);
			sword->setEquipment(new Equipment(sword, Equipment::RightHand, 3));
			sword->setPickable(new Pickable(sword));
			engine.getActors().push(sword);
		}
		break;
		case Shield:
		{
			Actor *shield = new Actor(x, y, '[', "shield",TCODColor::darkerOrange);
			shield->setBlocks(false);
			shield->setEquipment(new Equipment(shield, Equipment::LeftHand, 0, 1));
			shield->setPickable(new Pickable(shield));
			engine.getActors().push(shield);
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
		engine.getPlayer()->setCoords(x, y);

		do
		{
			x = rng->getInt(1, width);
			y = rng->getInt(1, height);
		}
		while (!map->isWalkable(x, y));
		engine.getStairs()->setCoords(x, y);

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
	static const int nbTriesToFill = 3;
	int counter = nbTriesToFill;
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
	float nbWalls = 0;
	int nbCells = width * height;
	for (int x = 0; x < width; ++x)
		for (int y = 0; y < height; ++y)
			if (!map->isWalkable(x, y))
				++nbWalls;
	return nbWalls / nbCells;
}

void Map::populateCaves()
{
	float averageRoomLength = ((float)(RoomMaxSize + RoomMinSize) / 2) * 2 / 3;
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