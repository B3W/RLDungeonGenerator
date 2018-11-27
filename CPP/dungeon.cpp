#include "dungeon.h"
#include "room.h"

/*
 * Initialize dungeon map with immutable wall terrain
 * around the border.
 */
void init_dungeon(dungeon *d)
{
  uint8_t x, x_max, y, y_max;

  x_max = DUNGEON_X - 1;
  y_max = DUNGEON_Y - 1;
  for(y = 0; y < DUNGEON_Y; y++) {
    for(x = 0; x < DUNGEON_X; x++) {
      if((y == 0) || (y == y_max) ||
	 (x == 0) || (x == x_max)) {
	dmapxy(x, y) = ter_wall_immutable;
	hmapxy(x, y) = 255;
      } else {
	dmapxy(x, y) = ter_wall;
	hmapxy(x, y) = 0;
      }
    }
  }
  (*d).set_cursx(40);
  (*d).set_cursy(10);
}
