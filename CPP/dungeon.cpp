#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <endian.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

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

  /* The semantic, which is 12 bytes, 0-11 */
  fwrite(DUNGEON_SAVE_SEMANTIC, 1, strlen(DUNGEON_SAVE_SEMANTIC), f);
  
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

/*
 * Read in the stored hardness values
 */
int read_dungeon_map(dungeon *d, FILE *f)
{
  uint32_t x, y;

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      fread(&hmapxy(x, y), sizeof (hmapxy(x, y)), 1, f);
      if (hmapxy(x, y) == 0) {
        /* Mark it as a corridor.  We can't recognize room cells until *
         * after we've read the room array, which we haven't done yet. */
        dmapxy(x, y) = ter_floor_hall;
      } else if (hmapxy(x, y) == 255) {
        dmapxy(x, y) = ter_wall_immutable;
      } else {
        dmapxy(x, y) = ter_wall;
      }
    }
  }
  return 0;
}

/*
 * Read in the room data and error check
 */
int read_rooms(dungeon *d, uint8_t num_rooms, FILE *f)
{
  uint32_t i;
  uint8_t x, y, xsize, ysize;
 
  for (i = 0; i < num_rooms; i++) {
    fread(&x, 1, 1, f);
    fread(&y, 1, 1, f);
    fread(&xsize, 1, 1, f);
    fread(&ysize, 1, 1, f);
    d->rooms.push_back(new room(x, y, xsize, ysize));
    
    if (xsize < MIN_ROOM_XSIZE   ||
        ysize < MIN_ROOM_YSIZE   ||
        xsize > DUNGEON_X - 1    ||
        ysize > DUNGEON_Y - 1)    {
      std::fprintf(stderr, "Invalid room size in restored dungeon.\n");

      del_dungeon(d);
      std::exit(EXIT_FAILURE);
    }

    if (x < 1                     ||
        y < 1                     ||
        x > DUNGEON_X - 1         ||
        y > DUNGEON_Y - 1         ||
        x + xsize > DUNGEON_X - 1 ||
        x + xsize < 1             ||
        y + ysize > DUNGEON_Y - 1 ||
        y + ysize < 1)             {
      std::fprintf(stderr, "Invalid room position in restored dungeon.\n");

      del_dungeon(d);
      std::exit(EXIT_FAILURE);
    }
        
    /* After reading each room, we need to reconstruct them in the dungeon. */
    for (y = (*d->rooms[i]).get_y();
         y < (*d->rooms[i]).get_y() + (*d->rooms[i]).get_ysize();
         y++) {
      for (x = (*d->rooms[i]).get_x();
	   x < (*d->rooms[i]).get_x() + (*d->rooms[i]).get_xsize();
	   x++) {
        dmapxy(x, y) = ter_floor_room;
      }
    }
  }
  return 0;
}


/*
 * Calculate the number of rooms written to file
 */
int calculate_num_rooms(uint32_t dungeon_bytes)
{
  return ((dungeon_bytes -
          (20 /* The semantic, version, and size */       +
           (DUNGEON_X * DUNGEON_Y) /* The hardnesses */)) /
          4 /* Four bytes per room */);
}

/*
 * Read in dungeon information from disc
 */
int read_dungeon(dungeon *d, const char *file)
{
  char semantic[strlen(DUNGEON_SAVE_SEMANTIC) + 1];
  uint32_t be32;
  FILE *f;
  char *home;
  size_t len;
  char *filename;
  struct stat buf;
  uint8_t num_rooms, pcx, pcy;

  if (!file) {
    home = (char *) ".";
    len = (strlen(home) + strlen(DUNGEON_SAVE_FILE) +
           1 /* The NULL terminator */              +
           2 /* The slashes */);

    filename = (char *) std::malloc(len * sizeof (*filename));
    sprintf(filename, "%s/%s", home, DUNGEON_SAVE_FILE);

    if (!(f = fopen(filename, "r"))) {
      std::perror(filename);
      std::free(filename);
      std::exit(EXIT_FAILURE);
    }

    if (stat(filename, &buf)) {
      std::perror(filename);
      std::exit(EXIT_FAILURE);
    }

    std::free(filename);
  } else {
    if (!(f = fopen(file, "r"))) {
      std::perror(file);
      std::exit(EXIT_FAILURE);
    }
    if (stat(file, &buf)) {
      std::perror(file);
      std::exit(EXIT_FAILURE);
    }
  }

  num_rooms = 0;

  fread(semantic, strlen(DUNGEON_SAVE_SEMANTIC), 1, f);
  semantic[strlen(DUNGEON_SAVE_SEMANTIC)] = '\0';
  if (strncmp(semantic, DUNGEON_SAVE_SEMANTIC,
	      strlen(DUNGEON_SAVE_SEMANTIC))) {
    std::fprintf(stderr, "Not an RLG327 save file.\n");
    std::exit(EXIT_FAILURE);
  }
  fread(&be32, sizeof (be32), 1, f);
  if (be32toh(be32) != 0) { /* Since we expect zero, be32toh() is a no-op. */
    std::fprintf(stderr, "File version mismatch.\n");
    std::exit(EXIT_FAILURE);
  }
  fread(&be32, sizeof (be32), 1, f);
  if (buf.st_size != be32toh(be32)) {
    std::fprintf(stderr, "File size mismatch.\n");
    std::exit(EXIT_FAILURE);
  }

  fread(&pcx, 1, 1, f);
  (*d).set_pcx(pcx);
  fread(&pcy, 1, 1, f);
  (*d).set_pcy(pcy);
  if(pcx && pcy) {
    (*d).set_cursx(pcx);
    (*d).set_cursy(pcy);
  } else {
    (*d).set_cursx(40);
    (*d).set_cursy(10);
  }
  
  read_dungeon_map(d, f);
  num_rooms = calculate_num_rooms(buf.st_size);
  
  read_rooms(d, num_rooms, f);

  fclose(f);

  return 0;
}
