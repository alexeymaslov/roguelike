#pragma once

#include <vector>

#include "libtcod.hpp"

#include "persistent.hpp"

struct Tile
{
	bool explored;
	unsigned int scent;
	Tile() : explored(false), scent(0) {}
};

class Map : public Persistent
{
public :
	int width;
	int height;

	Map(int width, int height);
	~Map();

	bool isWall(int x, int y) const;
	void render() const;

	bool isInFov(int x, int y) const;
	bool isExplored(int x, int y) const;
	void computeFov();

	bool canWalk(int x, int y) const;
	void addMonster(int x, int y);
	void addItem(int x, int y);

	unsigned int currentScentValue;
	unsigned int getScent(int x, int y) const;

	const TCODMap *getMap() const { return map; }
	
	void init(bool withActors);

	void load(TCODZip &zip);
	void save(TCODZip &zip); 

protected :
	Tile *tiles;
	TCODMap *map;
	long seed;
	TCODRandom *rng;

	template<typename T1, typename T2>
	struct Pair
	{
		T1 first;
		T2 second;
	};

	enum GeneratedActorType
	{
		ORC, TROLL, HEALER, LIGHTNING_BOLT, FIREBALL, CONFUSER, SWORD, SHIELD
	};

	int maxAmountRoomMonsters;
	int maxAmountRoomItems;
	std::vector<Pair<GeneratedActorType, int>> monsterChances;
	std::vector<Pair<GeneratedActorType, int>> itemChances;

	friend class BspListener;

	void dig(int x1, int y1, int x2, int y2);
	void createRoom(bool first, int x1, int y1, int x2, int y2, bool withActors);

	GeneratedActorType rollActorType(const std::vector<Pair<GeneratedActorType, int>> &v) const;
	int getChance(const std::vector<Pair<int, int>> &v) const;
	void initItemOrMonsterChances();

	void bspGeneration(bool withActors);

	// TODO убрать в отдельный класс
	void cellularAutomataGeneration(bool withActors);
	void randomFillMap();
	void makeCaverns(int nbWalls1Tile, int nbWalls2Tile);
	int getAmountOfWallsNear(int x, int y, int dx = 1, int dy = 1) const;

	// возвращает false, если необходимо перегенерировать карту
	bool floodFill();
	void fill(int x, int y, bool *wall);
	float calcPercentOfWalls() const;
};