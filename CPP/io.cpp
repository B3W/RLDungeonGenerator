#include <iostream>
#include <ncurses.h>

#include "dungeon.h"
#include "io.h"

/*
 * Initialize terminal for ncurses
 */
void io_init_terminal(void)
{
  initscr();
  raw();
  noecho();
  curs_set(1);
  keypad(stdscr, TRUE);
  /*
  start_color();
  init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
  init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
  init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
  init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
  init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
  init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
  */
}

/*
 * Deinitialize ncurses
 */
void io_reset_terminal(void)
{
  endwin();
}

static char hardness_to_char[] =
  "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

/*
 * Display the hardness values in the terminal
 */
void io_display_hardness(dungeon *d)
{
  uint32_t y, x;
  
  clear();
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      /* Maximum hardness is 255.  We have 62 values to display it, but *
       * we only want one zero value, so we need to cover [1,255] with  *
       * 61 values, which gives us a divisor of 254 / 61 = 4.164.       *
       * Generally, we want to avoid floating point math, but this is   *
       * not gameplay, so we'll make an exception here to get maximal   *
       * hardness display resolution.                                   */
      mvaddch(y, x, (hmapxy(x, y)                                       ?
		     hardness_to_char[1 + (int)(hmapxy(x, y)/4.2)] : ' '));
    }
  }
  move((*d).get_cursy(), (*d).get_cursx());
  refresh();
}

/*
 * Display dungeon to the terminal
 */
void io_display(dungeon *d)
{
  uint8_t x, y, x_max, y_max;

  x_max = DUNGEON_X - 1;
  y_max = DUNGEON_Y - 1;
  
  clear();
  for(y = 0; y < DUNGEON_Y; y++) {
    for(x = 0; x < DUNGEON_X; x++) {
      switch(d->d_map[y][x])
	{
	case ter_wall:
	  mvaddch(y, x, WALL_CHAR);
	  break;
	case ter_wall_immutable:
	  if(y == 0 || y == y_max) {
	    mvaddch(y, x, HORIZ_BORDER_CHAR);
	  } else if(x == 0 || x == x_max) {
	    mvaddch(y, x, VERT_BORDER_CHAR);
	  }
	  break;
	case ter_floor_room:
	  mvaddch(y, x, ROOM_CHAR);
	  break;
	case ter_floor:
	case ter_floor_hall:
	  mvaddch(y, x, HALL_CHAR);
	  break;
	default:
	  mvaddch(y, x, UNKNOWN_CHAR);
	}
    }
  }
  move((*d).get_cursy(), (*d).get_cursx());
  refresh();
}

/*
 * Check if it is valid to move cursor in given direction
 * Updates the cursor position if valid
 */
bool valid_move(dungeon *d, int x_dir, int y_dir)
{
  uint32_t curs_x, curs_y;

  curs_x = (*d).get_cursx() + x_dir;
  curs_y = (*d).get_cursy() + y_dir;
  if((curs_x == 0) || (curs_x == DUNGEON_X - 1)) {
    return false;
  } else if ((curs_y == 0) || (curs_y == DUNGEON_Y - 1)) {
    return false;
  }
  (*d).set_cursx(curs_x);
  (*d).set_cursy(curs_y);
  return true;
}

/*
 * Handle user input to the program
 */
void io_mainloop(dungeon *d)
{
  uint8_t quit = 0;
  
  do {
    switch(getch())
      {
      case KEY_UP:
      case 'k':
	/* Move cursor up */
	if(valid_move(d, 0, -1)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  refresh();
	}
	break;
      case KEY_DOWN:
      case 'j':
	/* Move cursor down */
	if(valid_move(d, 0, 1)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  refresh();
	}
	break;
      case KEY_RIGHT:
      case 'l':
	/* Move cursor right */
	if(valid_move(d, 1, 0)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  refresh();
	}
	break;
      case KEY_LEFT:
      case 'h':
	/* Move cursor left */
	if(valid_move(d, -1, 0)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  refresh();
	}
	break;
      case 'y':
	/* Move cursor up left */
	if(valid_move(d, -1, -1)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  refresh();
	}
	break;
      case 'u':
	/* Move cursor up right */
	if(valid_move(d, 1, -1)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  refresh();
	}
	break;
      case 'b':
      /* Move cursor down left */
	if(valid_move(d, -1, 1)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  refresh();
	}
	break;
      case 'n':
	/* Move cursor down right */
	if(valid_move(d, 1, 1)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  refresh();
	}
	break;
      case 'N':
	/* Display the normal dungeon map */
	io_display(d);
	break;
      case 'H':
	/* Display the hardness map */
	io_display_hardness(d);
	break;
      case 'Q':
	/* Quit the dungeon generator */
	quit = 1;
	break;
      }
  } while(!quit);
}
