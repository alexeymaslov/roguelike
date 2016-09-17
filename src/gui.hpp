#pragma once

#include "libtcod.hpp"
#include "persistent.hpp"

static const int PanelHeight = 7;

class Menu
{
public:
	enum MenuItemCode
	{
		None,
		NewGame,
		Continue,
		Exit,
		Constitution,
		Strength,
		Agility
	};

	enum DisplayMode
	{
		Main,
		Pause
	};

	~Menu();
	void clear();
	void addItem(MenuItemCode code, const char *label);
	MenuItemCode pick(DisplayMode mode = Main);

protected:
	struct MenuItem
	{
		MenuItemCode code;
		const char *label;
	};
	TCODList<MenuItem *> items;
};

class Gui : public Persistent
{
public:
	Menu &getMenu() { return menu; };

	Gui();
	~Gui();
	void render();
	// Работает как сишный printf
	void message(const TCODColor &col, const char *text, ...);
	void clear();

	void load(TCODZip &zip);
	void save(TCODZip &zip);

protected:
	Menu menu;
	TCODConsole *con;
	void renderBar(int x, int y, int width, const char *name,
		float value, float maxValue, const TCODColor &barColor,
		const TCODColor &backColor);
	void renderLog();
	void renderMouseLook();

	struct Message
	{
		char *text;
		TCODColor col;
		Message(const char *text, const TCODColor &col);
		~Message();
	};
	TCODList<Message *> log;
};