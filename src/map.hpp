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
	Map(int width, int height);
	~Map();

	int getWidth() const { return width; };
	int getHeight() const { return height; };
	const TCODMap *getMap() { return map; };
	int getCurrentScentValue() const { return currentScentValue; };
	void incCurrentScentValue() { currentScentValue++; };
	bool isInFov(int x, int y) const;
	bool isExplored(int x, int y) const;
	bool canWalk(int x, int y) const;
	unsigned int getScent(int x, int y) const;

	bool isWall(int x, int y) const;
	void render() const;
	void computeFov();
	// withActors -- нужно ли заодно создавать объекты на карте, или они загружаются из файла
	void init(bool withActors);
	
	void load(TCODZip &zip);
	void save(TCODZip &zip); 

protected :
	Tile *tiles;
	TCODMap *map;
	long seed;
	TCODRandom *rng;
	int width;
	int height;
	unsigned int currentScentValue;

	void addMonster(int x, int y);
	void addItem(int x, int y);

	// TODO заменить на std::pair
	template<typename T1, typename T2>
	struct Pair
	{
		T1 first;
		T2 second;
	};
	// TODO убрать всё это в фабрику
	enum GeneratedActorType
	{
		Orc, Troll, Healer, LightningBolt, Fireball, Confuser, Sword, Shield
	};

	int maxAmountRoomMonsters;
	int maxAmountRoomItems;
	std::vector<Pair<GeneratedActorType, int>> monsterChances;
	std::vector<Pair<GeneratedActorType, int>> itemChances;

	// Нужно для использования TCODBsp
	friend class BspListener;

	// выкапывает прямоугольную область
	void dig(int x1, int y1, int x2, int y2);
	void createRoom(bool first, int x1, int y1, int x2, int y2, bool withActors);
	// TODO заменить на std::pair, а лучше рассмотреть замену на std::map
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
	// TODO объединить с createRoom (в ней тоже создаются итемы и монстры)
	void populateCaves();
};