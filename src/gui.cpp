#include "stdarg.h"
#include "stdio.h"

#include "gui.hpp"

#include "engine.hpp"

static const int BarWidth = 20;
static const int MessageX = BarWidth + 2;
static const int MessageHeight = PanelHeight - 1;

Gui::Gui()
{
	con = new TCODConsole(engine.getScreenWidth(), PanelHeight);
}

Gui::~Gui()
{
	delete con;
	clear();
}

void Gui::clear()
{
	log.clearAndDelete();
}

void Gui::render()
{
	con->setDefaultBackground(TCODColor::black);
	con->clear();

	renderBar(1, 1, BarWidth, "HP", engine.getPlayer()->getDestructible()->getHp(),
		engine.getPlayer()->getDestructible()->maxHp(),
		TCODColor::lightRed, TCODColor::darkerRed);
	renderLog();
	renderMouseLook();

	con->setDefaultForeground(TCODColor::white);
	con->print(3,3,"Dungeon level %d",engine.getLevel());

	// TODO может залагать, если ai игрока это TemporaryAi
	PlayerAi *ai = (PlayerAi *) engine.getPlayer()->getAi();
	char xpTxt[128];
	sprintf(xpTxt, "XP(%d)", ai->getXpLevel());
	renderBar(1, 5, BarWidth, xpTxt, engine.getPlayer()->getDestructible()->getXp(), ai->getNextLevelXp(),
		TCODColor::lightViolet, TCODColor::darkerViolet);

	TCODConsole::blit(con, 0, 0, engine.getScreenWidth(), PanelHeight, 
		TCODConsole::root, 0, engine.getScreenHeight() - PanelHeight);
}

void Gui::renderLog()
{
	int y = 1;
	float colorCoef = 0.4f;
	for (Message **it = log.begin(); it != log.end(); ++it)
	{
		Message *message = *it;
		con->setDefaultForeground(message->col * colorCoef);
		con->print(MessageX, y, message->text);
		++y;
		if (colorCoef < 1.0f) colorCoef += 0.3f;
	}

}

void Gui::renderBar(int x, int y, int width, const char *name,
		float value, float maxValue, const TCODColor &barColor,
		const TCODColor &backColor)
{
	con->setDefaultBackground(backColor);
	con->rect(x, y, width, 1, false, TCOD_BKGND_SET);

	int barWidth = (int)(value / maxValue * width);
	if (barWidth > 0)
	{
		con->setDefaultBackground(barColor);
		con->rect(x, y, barWidth, 1, false, TCOD_BKGND_SET);
	}
	con->setDefaultForeground(TCODColor::white);
	con->printEx(x + width / 2, y, TCOD_BKGND_NONE, TCOD_CENTER,
		"%s : %g/%g", name, value, maxValue);

}

Gui::Message::Message(const char *text, const TCODColor &col) : text(strdup(text)), col(col)
{

}

Gui::Message::~Message()
{
	free(text);
}

void Gui::message(const TCODColor &col, const char *text, ...)
{
	va_list ap;
	char buf[128];
	va_start(ap, text);
	vsprintf(buf, text, ap);
	va_end(ap);

	char *lineBegin = buf;
	char *lineEnd;

	do
	{
		if (log.size() == MessageHeight)
		{
			Message *toRemove = log.get(0);
			log.remove(toRemove);
			delete toRemove;
		}

		lineEnd = strchr(lineBegin, '\n');
		if (lineEnd)
			*lineEnd = '\0';

		Message *msg = new Message(lineBegin, col);
		log.push(msg);
		lineBegin = lineEnd + 1;
	}
	while (lineEnd);
}

void Gui::renderMouseLook()
{
	int mapx = engine.getMouse().cx + engine.getCamerax();
	int mapy = engine.getMouse().cy + engine.getCameray();

	if (engine.getMouse().cy >= engine.getCameraHeight() ||
		!engine.getMap()->isInFov(mapx, mapy))
		return;

	char buf[128] = "";
	bool first = true;
	for (Actor **it = engine.getActors().begin(); it != engine.getActors().end(); ++it)
	{
		Actor *actor = *it;
		if (actor->getX() == mapx && actor->getY() == mapy)
		{
			if (!first)
				strcat(buf, ", ");
			else
				first = false;
			strcat(buf, actor->getName());
		}
	}
	con->setDefaultForeground(TCODColor::lightGrey);
	con->print(1, 0, buf);
}

Menu::~Menu()
{
	clear();
}

void Menu::clear()
{
	items.clearAndDelete();
}

void Menu::addItem(MenuItemCode code, const char *label)
{
	MenuItem *item = new MenuItem();
	item->code = code;
	item->label = label;
	items.push(item);
}

Menu::MenuItemCode Menu::pick(DisplayMode mode)
{
	int selectedItem = 0;
	int menux;
	int menuy;
	if (mode == Pause)
	{	
		static const int PauseMenuWidth = 30;
		static const int PauseMenuHeight = 15;
		menux = engine.getScreenWidth() / 2 - PauseMenuWidth / 2;
		menuy = engine.getScreenHeight() / 2 - PauseMenuHeight / 2;
		TCODConsole::root->setDefaultForeground(TCODColor(200, 180, 50));
		TCODConsole::root->printFrame(menux, menuy, PauseMenuWidth, PauseMenuHeight, true,
			TCOD_BKGND_ALPHA(70), "menu");
		menux += 2;
		menuy += 3;
	}
	else
	{
		TCODConsole::root->setDefaultBackground(TCODColor::black);
		TCODConsole::root->clear();
		menux = 10;
		menuy = TCODConsole::root->getHeight() / 3;
	}

	while (!TCODConsole::isWindowClosed())
	{
		
		int currentItem = 0;
		for (MenuItem **i = items.begin(); i != items.end(); ++i)
		{
			if (currentItem == selectedItem)
				TCODConsole::root->setDefaultForeground(TCODColor::lightOrange);
			else
				TCODConsole::root->setDefaultForeground(TCODColor::lightGrey);

			TCODConsole::root->print(menux, menuy + currentItem * 3, (*i)->label);
			++currentItem;
		}
		TCODConsole::flush();

		TCOD_key_t key;
		TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS, &key, nullptr);
		switch (key.vk)
		{
			case TCODK_UP: case TCODK_KP8:
			{
				--selectedItem;
				if (selectedItem < 0)
					selectedItem = items.size() - 1;
			}
			break;
			case TCODK_DOWN: case TCODK_KP2:
				selectedItem = (selectedItem + 1) % items.size();
			break;
			case TCODK_ENTER:
				return items.get(selectedItem)->code;
			break;
			default:
			break;
		}
	}
	return None;
}