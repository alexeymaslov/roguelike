#pragma once

#include "persistent.hpp"
#include "targetselector.hpp"
#include "effect.hpp"

class Actor;
class Container;

class Pickable : public Persistent
{
public:
	Pickable(Actor *owner, TargetSelector *selector = nullptr, Effect *effect = nullptr);
	~Pickable();

	void setContainer(Container *container);
	void eraseContainer() { this->container = nullptr; };

	// Возвращает true, если удалось успешно добавить в container
	bool pick(Actor *wearer);
	void drop();
	bool use();

	void load(TCODZip &zip);
	void save(TCODZip &zip);

private:
	Actor *owner;
	Container *container;
	TargetSelector *selector;
	Effect *effect;
};