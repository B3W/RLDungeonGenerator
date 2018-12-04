#include "room.h"

/*
 * Initialize a room with given values
 */
room::room(uint8_t x, uint8_t y, uint8_t x_s, uint8_t y_s)
{
  x_pos = x;
  y_pos = y;
  x_size = x_s;
  y_size = y_s;
}

/*
 * Copy Constructor
 */
room::room(const room &r)
{
  x_pos = r.x_pos;
  y_pos = r.y_pos;
  x_size = r.x_size;
  y_size = r.y_size;
}

/*
 * Return 4 byte value for writing room data to disc.
 * MSB: x position
 * Next Byte: y position
 * Next Byte: x size
 * LSB: y size
 */
uint32_t room::byte_frmt(void)
{
  uint32_t x_tmp = x_pos;
  uint32_t y_tmp = y_pos;
  uint32_t xs_tmp = x_size;
  uint32_t bytes = 0;

  bytes = (x_tmp << 24) | (y_tmp << 16) | (xs_tmp << 8) | y_size;
  return bytes;
}

/*
 * Checks if the x, y coordinate is within the room parameters
 */
bool room::contains(uint8_t x, uint8_t y)
{
  if((x_pos <= x && x <= x_pos + x_size) &&
     (y_pos <= y && y <= y_pos + y_size)) {
    return true;
  }
  return false;
}
