#include "libtcod.hpp"

#include "engine.hpp"

// TODO убрать эти параметры в параметры main
Engine engine(80, 50, 100, 100);

int main()
{
	engine.load();
	while (!TCODConsole::isWindowClosed()) {

		engine.update();
		engine.render();
		TCODConsole::flush();
	}
	engine.save();
	return 0;
}
