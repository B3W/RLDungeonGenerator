#include "dungeon.h"
#include "io.h"
#include "room.h"

int main(int argc, char *argv[])
{
  dungeon d;

  io_init_terminal();
  init_dungeon(&d);

  io_display(&d);
  io_mainloop(&d);

  io_reset_terminal();
  
  return 0;
}
