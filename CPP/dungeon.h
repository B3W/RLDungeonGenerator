#ifndef DUNGEON_H
#define DUNGEON_H

#include <stdint.h>
#include <vector>

#include "room.h"

#define dmapxy(x, y) (d->d_map[y][x])
#define hmapxy(x, y) (d->h_map[y][x])

const uint32_t DUNGEON_X = 80;
const uint32_t DUNGEON_Y = 21;
const uint32_t MAX_HARDNESS_VALUE = 255;
const uint32_t MIN_ROOM_COUNT = 5;
const uint32_t MAX_ROOM_COUNT = 15;
const char* const DUNGEON_SAVE_FILE = "dungeon";
const char* const DUNGEON_SAVE_SEMANTIC = "RLG327-F2018";
const uint32_t DUNGEON_SAVE_VERSION = 0;
//const char* const MONSTER_DESC_FILE = "monster_desc.txt";
//const char* const OBJECT_DESC_FILE = "object_desc.txt";

enum __attribute__ ((__packed__)) terrain_type {
  ter_debug,
  ter_unknown,
  ter_wall,
  ter_wall_immutable,
  ter_floor,
  ter_floor_room,
  ter_floor_hall,
  ter_stairs,
  ter_stairs_up,
  ter_stairs_down
};

class dungeon {
 private:
  uint8_t pc_x, pc_y;
  uint32_t curs_x, curs_y;
 public:
  std::vector<room*> rooms;
  terrain_type d_map[DUNGEON_Y][DUNGEON_X];
  uint8_t h_map[DUNGEON_Y][DUNGEON_X];
  dungeon() : pc_x(0), pc_y(0), curs_x(0), curs_y(0),
              rooms(), d_map{ter_wall}, h_map{0} {}

  uint8_t get_pcx(void)
  {
    return pc_x;
  }

  uint8_t get_pcy(void)
  {
    return pc_y;
  }

  uint32_t get_cursx(void)
  {
    return curs_x;
  }

  uint32_t get_cursy(void)
  {
    return curs_y;
  }

  void set_pcx(uint8_t x)
  {
    pc_x = x;
  }

  void set_pcy(uint8_t y)
  {
    pc_y = y;
  }

  void set_pc(uint8_t x, uint8_t y)
  {
    pc_x = x;
    pc_y = y;
  }

  void set_cursx(uint32_t x)
  {
    curs_x = x;
  }

  void set_cursy(uint32_t y)
  {
    curs_y = y;
  }

  void set_curs(uint32_t x, uint32_t y)
  {
    curs_x = x;
    curs_y = y;
  }
};

void init_dungeon(dungeon *d);
void del_dungeon(dungeon *d);
void get_warnings(dungeon *d, std::vector<const char *>& warnings);
int write_dungeon(dungeon *d, const char *file);
int read_dungeon(dungeon *d, const char *file);

#endif
