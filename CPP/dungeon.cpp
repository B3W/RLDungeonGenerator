#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <endian.h>

#include "dungeon.h"
#include "room.h"
#include "utils.h"

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
	hmapxy(x, y) = MAX_HARDNESS_VALUE;
      } else {
	dmapxy(x, y) = ter_wall;
	hmapxy(x, y) = rand_range(1, (MAX_HARDNESS_VALUE - 1));
      }
    }
  }
  (*d).set_cursx(40);
  (*d).set_cursy(10);
}

/*
 * Cleans up the dungeon data stucture
 */
void del_dungeon(dungeon *d)
{
  uint8_t i;

  for(i = 0; i < d->rooms.size(); i++) {
    delete d->rooms[i];
  }
}

/*
 * Writes hardness values to the dungeon
 */
int write_dungeon_map(dungeon *d, FILE *f)
{
  uint32_t x, y;

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      fwrite(&hmapxy(x, y), sizeof (unsigned char), 1, f);
    }
  }
  return 0;
}

/*
 * Writes room data to the file given
 */
int write_rooms(dungeon *d, FILE *f)
{
  uint32_t i;
  uint8_t p;

  for (i = 0; i < d->rooms.size(); i++) {
    /* write order is xpos, ypos, width, height */
    p = (*d->rooms[i]).get_x();
    fwrite(&p, 1, 1, f);
    p = (*d->rooms[i]).get_y();
    fwrite(&p, 1, 1, f);
    p = (*d->rooms[i]).get_xsize();
    fwrite(&p, 1, 1, f);
    p = (*d->rooms[i]).get_ysize();
    fwrite(&p, 1, 1, f);
  }
  return 0;
}

/*
 * Calculates size to write to file
 */
uint32_t calculate_dungeon_size(dungeon *d)
{
  return (22 /* The semantic, version, size, and PC position*/ +
          (DUNGEON_X * DUNGEON_Y) /* The hardnesses         */ +
          (d->rooms.size() * 4)   /* Four bytes per room    */ );
}

/*
 * Writes dungeon data to disc
 */
int write_dungeon(dungeon *d, const char *file)
{
  char *home;
  char *filename;
  FILE *f;
  size_t len;
  uint32_t be32;
  uint8_t i;

  if (!file) {
    home = (char *) ".";
    len = (strlen(home) + strlen(DUNGEON_SAVE_FILE) +
           1 /* The NULL terminator */              +
           2 /* The slashes */);

    filename = (char *) std::malloc(len * sizeof (*filename));
    sprintf(filename, "%s/%s", home, DUNGEON_SAVE_FILE);

    if (!(f = fopen(filename, "w"))) {
      std::perror(filename);
      std::free(filename);

      return 1;
    }
    std::free(filename);
  } else {
    if (!(f = fopen(file, "w"))) {
      std::perror(file);
      std::exit(EXIT_FAILURE);
    }
  }

  /* The semantic, which is 6 bytes, 0-11 */
  fwrite(DUNGEON_SAVE_SEMANTIC, 1, sizeof (DUNGEON_SAVE_SEMANTIC) - 1, f);

  /* The version, 4 bytes, 12-15 */
  be32 = htobe32(DUNGEON_SAVE_VERSION);
  fwrite(&be32, sizeof (be32), 1, f);

  /* The size of the file, 4 bytes, 16-19 */
  be32 = htobe32(calculate_dungeon_size(d));
  fwrite(&be32, sizeof (be32), 1, f);

  /* The PC position, 2 bytes, 20-21 */
  i = (*d).get_pcx();
  fwrite(&i, 1, 1, f);
  i = (*d).get_pcy();
  fwrite(&i, 1, 1, f);

  /* The dungeon map, 1680 bytes, 22-1702 */
  write_dungeon_map(d, f);

  /* And the rooms, num_rooms * 4 bytes, 1703-end */
  write_rooms(d, f);

  fclose(f);

  return 0;
}
