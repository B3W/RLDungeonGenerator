#ifndef ROOM_H
#define ROOM_H

#include <stdint.h>

const uint32_t MIN_ROOM_XSIZE = 3;
const uint32_t MIN_ROOM_YSIZE = 2;

class room {
private:
  uint8_t x_pos, y_pos;
  uint8_t x_size, y_size;
public:  
  room() : x_pos(0), y_pos(0), x_size(0), y_size(0) {}
  room(uint8_t x, uint8_t y, uint8_t x_s, uint8_t y_s);
  room(const room &r);
  uint32_t byte_frmt(void);
  bool contains(uint8_t x, uint8_t y);
  
  uint8_t get_x(void)
  {
    return x_pos;
  }

  uint8_t get_y(void)
  {
    return y_pos;
  }
  
  uint8_t get_xsize(void)
  {
    return x_size;
  }

  uint8_t get_ysize(void)
  {
    return y_size;
  }
  
  bool operator==(const room &r)
  {
    return (x_pos == r.x_pos) && (y_pos == r.y_pos);
  }

  bool compare(const room &r)
  {
    return (x_pos == r.x_pos) && (y_pos == r.y_pos);
  }

};

#endif
