#ifndef IO_H
#define IO_H

const char WALL_CHAR = ' ';
const char ROOM_CHAR = '.';
const char HALL_CHAR = '#';
const char HORIZ_BORDER_CHAR = '_';
const char VERT_BORDER_CHAR = '|';
const char UNKNOWN_CHAR = 'x';

class dungeon;

void io_init_terminal(void);
void io_reset_terminal(void);
void io_display_hardness(dungeon *d);
void io_display(dungeon *d);
void io_mainloop(dungeon *d);

#endif
