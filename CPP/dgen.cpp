#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <ctime>

#include "dungeon.h"
#include "io.h"
#include "room.h"

void usage(char *name)
{
  std::fprintf(stderr,
	       "Usage: %s [-l|--load [<file>]]\n",
	       name);
  std::exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
  dungeon d;
  uint32_t long_arg;
  uint8_t i, load;
  const char *load_file;

  load = 0;
  
  if(argc > 1) {
    for (i = 1, long_arg = 0; i < argc; i++, long_arg = 0) {
      if(argv[i][0] == '-') { /* All switches start with a dash */
	if(argv[i][1] == '-') {
	  argv[i]++;    /* Make the argument have a single dash so we can */
	  long_arg = 1; /* handle long and short args at the same place.  */
	}
	switch(argv[i][1]) {
	case 'l':
	  if ((!long_arg && argv[i][2]) ||
	      (long_arg && strcmp(argv[i], "-load"))) {
	    usage(argv[0]);
	  }
	  load = 1;
	  if ((argc > i + 1) && argv[i + 1][0] != '-') {
	    /* There is another argument, and it's not a switch, so *
	     * we'll treat it as a save file and try to load it.    */
	    load_file = argv[++i];
	  } else {
	    load_file = nullptr;
	  }
	  break;
	default:
	  usage(argv[0]);
	}     
      } else { /* No dash */
	usage(argv[0]);
      }
    }
  }
  
  std::srand(std::time(nullptr));
  
  io_init_terminal();
  if(load) {
    read_dungeon(&d, load_file);
  } else {
    init_dungeon(&d);
  }
  
  io_display(&d);
  io_mainloop(&d);
  
  io_reset_terminal();
  del_dungeon(&d);
  
  return 0;
}
