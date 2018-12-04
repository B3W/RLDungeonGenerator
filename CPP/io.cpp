#include <iostream>
#include <ncurses.h>
#include <string>
#include <unistd.h>

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
  start_color();
  init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
  init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
  init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
  init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
  init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
  init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
} // io_init_terminal

/*
 * Deinitialize ncurses
 */
void io_reset_terminal(void)
{
  endwin();
} // io_reset_terminal

/*
 * Prints out informational message 
 */
void print_message(const char *msg)
{
  attron(COLOR_PAIR(COLOR_CYAN));
  mvprintw(21, 1, "%s", msg);
  attroff(COLOR_PAIR(COLOR_CYAN));
} // print_message

/*
 * Prints out error message
 */
void print_error(const char *err)
{
  attron(COLOR_PAIR(COLOR_RED));
  mvprintw(21, 1, "%s", err);
  attroff(COLOR_PAIR(COLOR_RED));
} // print_message

/*
 * Clears displayed messages if any
 */
void clear_message(void)
{
  uint8_t x;

  for(x = 0; x < DUNGEON_X; x++) {
    mvaddch(21, x, ' ');
  }
} // clear_message

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
} // io_display_hardness

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
} // io_display

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
} // valid_move

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
} // valid_corridor_move

/*
 * Places a corridor character at the current cursor position
 * if the current position is not in a room
 */
void place_corridor(dungeon *d)
{
  uint8_t x, y;
  
  x = (*d).get_cursx();
  y = (*d).get_cursy();

  dmapxy(x, y) = ter_floor_hall;
  hmapxy(x, y) = 0;
  addch(HALL_CHAR);
  move(y, x);
  refresh();
} // place_corridor

/*
 * Moving cursor position causes corridor character to be placed
 * at that position. Cursor is not able to move through rooms. 
 */
void place_corridors(dungeon *d)
{
  uint8_t done = 0;

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
      case KEY_HOME:
	/* Place corridor up left */
	if(valid_corridor_move(d, -1, -1)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  place_corridor(d);
	}
	break;
      case 'u':
      case '9':
      case KEY_PPAGE:
	/* Place corridor up right */
	if(valid_corridor_move(d, 1, -1)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  place_corridor(d);
	}
	break;
      case 'b':
      case '1':
      case KEY_END:
      /* Place corridor down left */
	if(valid_corridor_move(d, -1, 1)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  place_corridor(d);
	}
	break;
      case 'n':
      case '3':
      case KEY_NPAGE:
	/* Place corridor down right */
	if(valid_corridor_move(d, 1, 1)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  place_corridor(d);
	}
	break;
      case 'C':
      case 'c':
	done = 1;
	break;
      }
  } while (!done);
} // place_corridors

/*
 * Places a wall character at the current cursor position
 * if the current position is not in a room
 */
uint8_t place_wall(dungeon *d)
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
    return 0;
  }
  return 1;
} // place_wall

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
} // room_present

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
	    for(i = (x - 1); i < (x + xsize + 1); i++) {
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
	    for(j = (y - 1); j < (y + ysize + 1); j++) {
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
	case 'r':
	  add = 1;
	  break;
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
} // place_room

/*
 * Checks if the x, y coordinate is within the room parameters
 */
bool room_containsxy(room *r, uint8_t x, uint8_t y)
{
  if(((*r).get_x() <= x && x <= ((*r).get_x() + (*r).get_xsize())) &&
     ((*r).get_y() <= y && y <= ((*r).get_y() + (*r).get_ysize()))) {
    return true;
  }
  return false;
} // room_constainsxy

/*
 * Deletes room currently hovered over by cursor
 */
void del_room(dungeon *d)
{
  /// TODO ///
  uint8_t cursx, cursy, x, y, i;
  int input;

  cursx = (*d).get_cursx();
  cursy = (*d).get_cursy();
  
  print_message("Delete Room? (y/n): ");
  input = getch();
  if(input == 'y' || input == 'Y') {
    x = cursx;
    y = cursy;
    for(i = 0; i < d->rooms.size(); i++) {
      if(room_containsxy(d->rooms[i], x, y)) {
	for(y = (*d->rooms[i]).get_y();
	    y < (*d->rooms[i]).get_y() + (*d->rooms[i]).get_ysize();
	    y++) {
	  for(x = (*d->rooms[i]).get_x();
	      x < (*d->rooms[i]).get_x() + (*d->rooms[i]).get_xsize();
	      x++) {
	    dmapxy(x, y) = ter_wall;
	    hmapxy(x, y) = rand_range(1, (MAX_HARDNESS_VALUE - 1));
	}
	}
	d->rooms.erase(d->rooms.begin() + i);
      }
    }
    io_display(d);
  } else {
    clear_message();
  }
  move(cursy, cursx);
} // del_room

/* Save the dungeon to disc */
int save_dungeon(dungeon *d)
{
  uint8_t win_y = 10;
  uint8_t win_x = 0;
  uint8_t win_width = DUNGEON_X - (win_x << 1);
  uint8_t win_height = DUNGEON_Y - win_y;

  /* Create save window */
  WINDOW *save_win;
  save_win = newwin(win_height, win_width, 1, win_x);
  keypad(save_win, TRUE);
  box(save_win, 0, 0);

  /* Save window title: */
  /* mvwprintw(WINDOW, y, x, Format, args) */
  mvwprintw(save_win, 0, 1, "%s", "Save Dungeon");

  /* Save prompt */
  mvwprintw(save_win, 3, 1, "%s", "Enter Filename (default=\'dungeon\'):");

  /* Exit instructions */
  mvwprintw(save_win, (win_height - 2), 1, "%s",
	    "F10 to save, ESC/F1 to cancel");

  /* Handle input */
  int input;
  uint8_t save, cncl;
  /* 260 is max character length for file path in NFTS */
  const uint32_t MAX_PATH = 260;
  char cwd[MAX_PATH];
  getcwd(cwd, sizeof(cwd));
  std::string path (cwd);
  path += '/';
  mvwprintw(save_win, 2, 1, "%s", path.c_str());
  
  uint8_t max_flen = MAX_PATH - path.length();
  std::string filename;
  save = cncl = 0;

  wmove(save_win, 4, 1);
  do {
    input = wgetch(save_win);
    if(input > 31 && input < 127 && input != '/') {
      /* Add character to filename */
      if((filename.length() < win_width) && (filename.length() < max_flen)) {
	filename += input;
	mvwaddch(save_win, 4, filename.length(), input);
      }
    } else if(input == KEY_BACKSPACE) {
      /* Delete last character of filename */
      if(filename.length() > 0) {
	mvwaddch(save_win, 4, filename.length(), ' ');
	wmove(save_win, 4, filename.length());
	filename.pop_back();      
      }
    } else if(input == 27 || input == KEY_F(1)) {
      cncl = 1;
    } else if(input == KEY_F(10)) {
      save = 1;
    }
  } while(!save && !cncl);

  if(save) {
    if(filename.length() > 0) {
      write_dungeon(d, filename.c_str());
    } else {
      write_dungeon(d, nullptr);
    }
  }
  /* Clear the border then deallocate memory for inventory window */
  wborder(save_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
  wrefresh(save_win);
  delwin(save_win);

  io_display(d);
  
  return 0;
} // save_dungeon

/*
 * If location valid, place the PC
 */
uint8_t place_pc(dungeon *d)
{
  uint8_t x, y;

  x = (*d).get_cursx();
  y = (*d).get_cursy();
  if(dmapxy(x, y) > ter_wall_immutable) {
    (*d).set_pcx(x);
    (*d).set_pcy(y);
    addch(PC_CHAR);
    refresh();
    return 0;
  }
  return 1;
} // place_pc

/*
 * Handle user input to the program
 */
void io_mainloop(dungeon *d)
{
  int input;
  uint8_t quit = 0;
  
  do {
    input = getch();
    if(mvinch(21, 1) != ' ') {
      clear_message();
    }
    move((*d).get_cursy(), (*d).get_cursx());
    switch(input)
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
      case KEY_HOME:
	/* Move cursor up left */
	if(valid_move(d, -1, -1)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  refresh();
	}
	break;
      case 'u':
      case '9':
      case KEY_PPAGE:
	/* Move cursor up right */
	if(valid_move(d, 1, -1)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  refresh();
	}
	break;
      case 'b':
      case '1':
      case KEY_END:
	/* Move cursor down left */
	if(valid_move(d, -1, 1)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  refresh();
	}
	break;
      case 'n':
      case '3':
      case KEY_NPAGE:
	/* Move cursor down right */
	if(valid_move(d, 1, 1)) {
	  move((*d).get_cursy(), (*d).get_cursx());
	  refresh();
	}
	break;
      case 'c':
	/* Place corrider tile at cursor location */
	if(dmapxy((*d).get_cursx(), (*d).get_cursy()) != ter_floor_room) {
	  place_corridor(d);
	} else {
	  print_error("Cannot place corridor over room tiles.");
	  move((*d).get_cursy(), (*d).get_cursx());
	}
	break;
      case 'C':
	/* Continuously place corridor tiles */
	if(dmapxy((*d).get_cursx(), (*d).get_cursy()) != ter_floor_room) {
	  place_corridors(d);
	} else {
	  print_error("Cannot place corridors over room tiles.");
	  move((*d).get_cursy(), (*d).get_cursx());
	}
	break;
      case 'w':
	/* Place wall tile at cursor location */
	if(place_wall(d)) {
	  print_error("Cannot place wall over room tile.");
	  move((*d).get_cursy(), (*d).get_cursx());
	}
	break;
      case 'r':
	/* Place room at cursor location */
	if (d->rooms.size() < MAX_ROOM_COUNT) {
	  place_room(d);
	} else {
	  print_error("Room limit reached. Delete rooms to add more.");
	  move((*d).get_cursy(), (*d).get_cursx());
	}
	break;
      case 'd':
	/* Delete room at cursor location */
	if(dmapxy((*d).get_cursx(), (*d).get_cursy()) == ter_floor_room) {
	  del_room(d);
	}
	break;
      case 'p':
	/* Place PC at cursor location */
	if(place_pc(d)) {
	  print_error("Place PC in room or corridor.");
	  move((*d).get_cursy(), (*d).get_cursx());
	}
	break;
      case 'D':
	/* Display the default dungeon map */
	io_display(d);
	break;
      case 'H':
	/* Display the hardness map */
	io_display_hardness(d);
	print_message("Displaying hardness values.");
	move((*d).get_cursy(), (*d).get_cursx());
	break;
      case 'Q':
	/* Quit the dungeon generator */
	quit = 1;
	break;
      case 'S':
	/* Save the dungeon */
	print_message("Saving...");
	save_dungeon(d);
	move((*d).get_cursy(), (*d).get_cursx());
	break;
      }
  } while(!quit);
} // io_mainloop
