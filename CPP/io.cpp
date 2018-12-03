#include <iostream>
#include <ncurses.h>

#include "dungeon.h"
#include "io.h"
#include "room.h"
#include "utils.h"

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
  uint8_t x, y, x_max, y_max, pcx, pcy;

  pcx = (*d).get_pcx();
  pcy = (*d).get_pcy();
  x_max = DUNGEON_X - 1;
  y_max = DUNGEON_Y - 1;
  
  clear();
  for(y = 0; y < DUNGEON_Y; y++) {
    for(x = 0; x < DUNGEON_X; x++) {
      if(x && y && (x == pcx) && (y == pcy)) {
	mvaddch(y, x, PC_CHAR);
      } else {
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
  if(dmapxy(curs_x, curs_y) == ter_wall_immutable) {
    return false;
  }
  (*d).set_cursx(curs_x);
  (*d).set_cursy(curs_y);
  return true;
}

bool valid_corridor_move(dungeon *d, int x_dir, int y_dir)
{
  uint32_t curs_x, curs_y;

  curs_x = (*d).get_cursx() + x_dir;
  curs_y = (*d).get_cursy() + y_dir;
  if((dmapxy(curs_x, curs_y) == ter_wall_immutable) ||
     (dmapxy(curs_x, curs_y) == ter_floor_room)) {
    return false;
  }
  (*d).set_cursx(curs_x);
  (*d).set_cursy(curs_y);
  return true;
}

/*
 * Places a corridor character at the current cursor position
 * if the current position is not in a room
 */
uint8_t place_corridor(dungeon *d)
{
  uint8_t x, y;
  
  x = (*d).get_cursx();
  y = (*d).get_cursy();
  if(dmapxy(x, y) != ter_floor_room) {
    dmapxy(x, y) = ter_floor_hall;
    hmapxy(x, y) = 0;
    addch(HALL_CHAR);
    move(y, x);
    refresh();
    return 0;
  }
  return 1;
}

/*
 * Moving cursor position causes corridor character to be placed
 * at that position. Cursor is not able to move through rooms. 
 */
void place_corridors(dungeon *d)
{
  uint8_t quit = 0;

  place_corridor(d);
  do {
    switch(getch())
      {
      case KEY_UP:
      case 'k':
      case '8':
	/* Place corridor up */
	if(valid_corridor_move(d, 0, -1)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  place_corridor(d);
	}
	break;
      case KEY_DOWN:
      case 'j':
      case '2':
	/* Place corridor down */
	if(valid_corridor_move(d, 0, 1)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  place_corridor(d);
	}
	break;
      case KEY_RIGHT:
      case 'l':
      case '6':
	/* Place corridor right */
	if(valid_corridor_move(d, 1, 0)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  place_corridor(d);
	}
	break;
      case KEY_LEFT:
      case 'h':
      case '4':
	/* Place corridor left */
	if(valid_corridor_move(d, -1, 0)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  place_corridor(d);
	}
	break;
      case 'y':
      case '7':
	/* Place corridor up left */
	if(valid_corridor_move(d, -1, -1)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  place_corridor(d);
	}
	break;
      case 'u':
      case '9':
	/* Place corridor up right */
	if(valid_corridor_move(d, 1, -1)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  place_corridor(d);
	}
	break;
      case 'b':
      case '1':
      /* Place corridor down left */
	if(valid_corridor_move(d, -1, 1)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  place_corridor(d);
	}
	break;
      case 'n':
      case '3':
	/* Place corridor down right */
	if(valid_corridor_move(d, 1, 1)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  place_corridor(d);
	}
	break;
      case 'C':
      case 'c':
      case 's':
	quit = 1;
	break;
      }
  } while (!quit);
}

/*
 * Places a wall character at the current cursor position
 * if the current position is not in a room
 */
void place_wall(dungeon *d)
{
  uint8_t x, y;
  
  x = (*d).get_cursx();
  y = (*d).get_cursy();
  if(dmapxy(x, y) != ter_floor_room) {
    dmapxy(x, y) = ter_wall;
    hmapxy(x, y) = rand_range(1, (MAX_HARDNESS_VALUE - 1));
    addch(WALL_CHAR);
    move(y, x);
    refresh();
  }
}

/*
 * Check if there is a room in the given area
 * There must be a padding of 1 tile between rooms
 */
bool room_present(dungeon *d, uint8_t x, uint8_t y, uint8_t xrng, uint8_t yrng)
{
  uint8_t i, j;

  for(j = (y - 1); j < (y + yrng + 1); j++) {
    for(i = (x - 1); i < (x + xrng + 1); i++) {
      if(dmapxy(i, j) == ter_floor_room) {
	return true;
      }
    }
  }
  return false;
}

/*
 * Place a room at the cursor location
 */
void place_room(dungeon *d)
{
  uint8_t x, y, xsize, ysize;
  uint8_t i, j, inval, add, quit;

  add = quit = 0;
  x = (*d).get_cursx();
  y = (*d).get_cursy();
  xsize = MIN_ROOM_XSIZE;
  ysize = MIN_ROOM_YSIZE;
  if((x + xsize) < DUNGEON_X && (y + ysize) < DUNGEON_Y &&
     !room_present(d, x, y, xsize, ysize)) {
    /* Setup minimum room */
    for(j = y; j < y + ysize; j++) {
      for(i = x; i < x + xsize; i++) {
	mvaddch(j, i, ROOM_CHAR);
      }
    }
    move(y, x);
    refresh();
    /* Allow size alteration */
    do {
      inval = 0;
      switch(getch())
	{
	case KEY_UP:
	case 'k':
	case '8':
	  /* Decrease Y Size */
	  if(ysize > MIN_ROOM_YSIZE) {
	    ysize--;
	    j = y + ysize;
	    for(i = x; i < x + xsize; i++) {
	      mvaddch(j, i, WALL_CHAR);
	    }
	    move(y, x);
	    refresh();
	  }
	  break;
	case KEY_DOWN:
	case 'j':
	case '2':
	  /* Increase Y Size */
	  j = y + ysize + 1;
	  if(j < DUNGEON_Y) {
	    for(i = x; i < (x + xsize + 1); i++) {
	      if(dmapxy(i, j) == ter_floor_room) {
		inval = 1;
		break;
	      }
	    }
	    if(!inval) {
	      j = y + ysize;
	      for(i = x; i < x + xsize; i++) {
		mvaddch(j, i, ROOM_CHAR);
	      }
	      ysize++;
	      move(y, x);
	      refresh();
	    }
	  }
	  break;
	case KEY_RIGHT:
	case 'l':
	case '6':
	  /* Increase X Size */
	  i = x + xsize + 1;
	  if(i < DUNGEON_X) {
	    for(j = y; j < (y + ysize + 1); j++) {
	      if(dmapxy(i, j) == ter_floor_room) {
		inval = 1;
		break;
	      }
	    }
	    if(!inval) {
	      i = x + xsize;
	      for(j = y; j < y + ysize; j++) {
		mvaddch(j, i, ROOM_CHAR);
	      }
	      xsize++;
	      move(y, x);
	      refresh();
	    }
	  }
	  break;
	case KEY_LEFT:
	case 'h':
	case '4':
	  /* Decrease X Size */
	  if(xsize > MIN_ROOM_XSIZE) {
	    xsize--;
	    i = x + xsize;
	    for(j = y; j < y + ysize; j++) {
	      mvaddch(j, i, WALL_CHAR);
	    }
	    move(y, x);
	    refresh();
	  }
	  break;
	case 's':
	  add = 1;
	  break;
	case 'Q':
	case 'q':
	  quit = 1;
	}
    } while (!quit && !add);
    if(add) {
      d->rooms.push_back(new room(x, y, xsize, ysize));
      for(j = y; j < y + ysize; j++) {
	for(i = x; i < x + xsize; i++) {
	  dmapxy(i, j) = ter_floor_room;
	  hmapxy(i, j) = 0;
	}
      }
    }
    io_display(d);
  }
}

/*
 * If location valid, place the PC
 */
void place_pc(dungeon *d)
{
  uint8_t x, y;

  x = (*d).get_cursx();
  y = (*d).get_cursy();
  if(dmapxy(x, y) > ter_wall_immutable) {
    (*d).set_pcx(x);
    (*d).set_pcy(y);
    io_display(d);
  }
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
      case '8':
	/* Move cursor up */
	if(valid_move(d, 0, -1)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  refresh();
	}
	break;
      case KEY_DOWN:
      case 'j':
      case '2':
	/* Move cursor down */
	if(valid_move(d, 0, 1)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  refresh();
	}
	break;
      case KEY_RIGHT:
      case 'l':
      case '6':
	/* Move cursor right */
	if(valid_move(d, 1, 0)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  refresh();
	}
	break;
      case KEY_LEFT:
      case 'h':
      case '4':
	/* Move cursor left */
	if(valid_move(d, -1, 0)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  refresh();
	}
	break;
      case 'y':
      case '7':
	/* Move cursor up left */
	if(valid_move(d, -1, -1)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  refresh();
	}
	break;
      case 'u':
      case '9':
	/* Move cursor up right */
	if(valid_move(d, 1, -1)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  refresh();
	}
	break;
      case 'b':
      case '1':
      /* Move cursor down left */
	if(valid_move(d, -1, 1)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  refresh();
	}
	break;
      case 'n':
      case '3':
	/* Move cursor down right */
	if(valid_move(d, 1, 1)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  refresh();
	}
	break;
      case 'c':
	/* Place corrider tile at cursor location */
	place_corridor(d);
	break;
      case 'C':
	if(dmapxy((*d).get_cursx(), (*d).get_cursy()) != ter_floor_room) {
	  place_corridors(d);
	}
	break;
      case 'w':
	/* Place wall tile at cursor location */
	place_wall(d);
	break;
      case 'r':
	/* Place room at cursor location */
	place_room(d);
	break;
      case 'p':
	/* Place PC at cursor location */
	place_pc(d);
	break;
      case 'D':
	/* Display the default dungeon map */
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
