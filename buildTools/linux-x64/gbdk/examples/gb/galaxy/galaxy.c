/*
 * C version of the 'space' assembly demo.
 *
 * Little demo illustrating how to use the graphical possibilities
 *  of the GB (background, window and animated sprite)
 * I have used fixed-point values for both the position and
 *  speed of objects to get smooth movements
 * 
 * OBJ data             : 0x8000 -> 0x8FFF (unsigned)
 * Window data          : 0x8800 -> 0x97FF (unsigned)
 * Background data      : 0x8800 -> 0x97FF (signed)
 * 
 * Tiled 0xFC -> 0xFF are standard tiles (all black -> all white)
 * 
 * Keys:
 * Arrow keys           : Change the speed (and direction) of the sprite
 * Arrow keys + A       : Change the speed (and direction) of the window
 * Arrow keys + B       : Change the speed (and direction) of the background
 * START                : Open/close the door
 * SELECT               : Basic fading effect
 * 
 * Note that the window is kept in the lower right part of the screen
 * since it can't be made transparent
 */

#include <gb/gb.h>
#include <stdint.h>

const unsigned char std_data[] = {

  /* Basic tiles (0xFC to 0xFF) */
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

const unsigned char earth_data[] = {

  /* Tile 0x00 */
  0x07,0x07,0x18,0x1F,0x32,0x2D,0x71,0x4E,0x70,0x4F,0xF8,0x87,0xF8,0x87,0xF8,0x87,
  0xFC,0x83,0xFE,0x81,0x7F,0x40,0x7F,0x40,0x3F,0x20,0x1F,0x18,0x07,0x07,0x00,0x00,
  0xC0,0xC0,0xF0,0x30,0x78,0x88,0x3C,0xC4,0x5C,0xA4,0x9E,0x62,0x3E,0xC2,0x3E,0xC2,
  0x5E,0xA2,0x7E,0x82,0x0C,0xF4,0x0C,0xF4,0x98,0x68,0xB0,0x70,0xC0,0xC0,0x00,0x00,
  0x07,0x07,0x1F,0x18,0x2F,0x30,0x4F,0x70,0x6F,0x50,0x9F,0xE0,0x9F,0xE0,0xBF,0xC0,
  0xFF,0x80,0xB7,0xC8,0x63,0x5C,0x43,0x7C,0x3F,0x20,0x1F,0x18,0x07,0x07,0x00,0x00,
  0xC0,0xC0,0xB0,0x70,0x18,0xE8,0x0C,0xF4,0x0C,0xF4,0x82,0x7E,0x82,0x7E,0x86,0x7A,
  0xC6,0x3A,0xE6,0x1A,0xF4,0x0C,0xFC,0x04,0xF8,0x08,0xF0,0x30,0xC0,0xC0,0x00,0x00,

  /* Tile 0x08 */
  0x07,0x07,0x1E,0x19,0x20,0x3F,0x40,0x7F,0x42,0x7D,0x81,0xFE,0x81,0xFE,0x83,0xFC,
  0xD7,0xA8,0xBB,0xC4,0x6E,0x51,0x7C,0x43,0x3F,0x20,0x1F,0x18,0x07,0x07,0x00,0x00,
  0xC0,0xC0,0x70,0xB0,0xE8,0x18,0xF4,0x0C,0xF4,0x0C,0xFE,0x02,0xFE,0x02,0xFE,0x02,
  0xFE,0x02,0x7E,0x82,0x3C,0xC4,0x3C,0xC4,0xF8,0x08,0xF0,0x30,0xC0,0xC0,0x00,0x00,
  0x07,0x07,0x1B,0x1C,0x20,0x3F,0x40,0x7F,0x40,0x7F,0xE0,0x9F,0x90,0xEF,0x89,0xF6,
  0x8D,0xF2,0x9F,0xE0,0x5E,0x61,0x6F,0x50,0x3F,0x20,0x1F,0x18,0x07,0x07,0x00,0x00,
  0xC0,0xC0,0xB0,0x70,0x28,0xD8,0x04,0xFC,0x2C,0xD4,0x1E,0xE2,0x1E,0xE2,0x3E,0xC2,
  0x7E,0x82,0xB6,0x4A,0xE4,0x1C,0xC4,0x3C,0xF8,0x08,0xF0,0x30,0xC0,0xC0,0x00,0x00,

  /* Tile 0x10 */
  0x07,0x07,0x18,0x1F,0x20,0x3F,0x40,0x7F,0x40,0x7F,0xEE,0x91,0xF1,0x8E,0xE0,0x9F,
  0xE0,0x9F,0xF1,0x8E,0x71,0x4E,0x72,0x4D,0x3F,0x20,0x1F,0x18,0x07,0x07,0x00,0x00,
  0xC0,0xC0,0xF0,0x30,0x08,0xF8,0x04,0xFC,0x04,0xFC,0x02,0xFE,0x02,0xFE,0x92,0x6E,
  0xD6,0x2A,0xFE,0x02,0xEC,0x14,0xFC,0x04,0xF8,0x08,0xF0,0x30,0xC0,0xC0,0x00,0x00,
  0x07,0x07,0x1D,0x1A,0x36,0x29,0x5C,0x63,0x6C,0x53,0xCE,0xB1,0x9F,0xE0,0x9E,0xE1,
  0xAE,0xD1,0xBF,0xC0,0x47,0x78,0x47,0x78,0x2F,0x30,0x1F,0x18,0x07,0x07,0x00,0x00,
  0xC0,0xC0,0x70,0xB0,0x08,0xF8,0x04,0xFC,0x04,0xFC,0xE2,0x1E,0x32,0xCE,0x0E,0xF2,
  0x0E,0xF2,0x1E,0xE2,0x1C,0xE4,0x2C,0xD4,0xF8,0x08,0xF0,0x30,0xC0,0xC0,0x00,0x00,

  /* Tile 0x18 */
  0x07,0x07,0x1E,0x19,0x33,0x2C,0x49,0x76,0x42,0x7D,0xC4,0xBB,0xC1,0xBE,0xC1,0xBE,
  0xE2,0x9D,0xF3,0x8C,0x78,0x47,0x78,0x47,0x3C,0x23,0x1C,0x1B,0x07,0x07,0x00,0x00,
  0xC0,0xC0,0x70,0xB0,0x68,0x98,0xC4,0x3C,0xC4,0x3C,0xEE,0x12,0xF2,0x0E,0xE2,0x1E,
  0xE2,0x1E,0xF2,0x0E,0x7C,0x84,0x7C,0x84,0xF8,0x08,0xF0,0x30,0xC0,0xC0,0x00,0x00
};

const unsigned char frame_data[] = {

  /* Tile 0x00 */
  0xFF,0x00,0x80,0x7F,0x80,0x7F,0x80,0x7F,0x80,0x7F,0x80,0x7F,0x80,0x7F,0x80,0x7F,
  0xFF,0x00,0x01,0xFE,0x03,0xFC,0x07,0xF8,0x0F,0xF0,0x1F,0xE0,0x3F,0xC0,0x7F,0x80,
  0xFF,0x00,0xFE,0x01,0xFC,0x03,0xF8,0x07,0xF0,0x0F,0xE0,0x1F,0xC0,0x3F,0x80,0x7F,
  0xFF,0x00,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,
  0xFF,0x00,0xFF,0x01,0xFD,0x03,0xF9,0x07,0xF1,0x0F,0xE1,0x1F,0xC1,0x3F,0x81,0x7F,
  0x80,0x7F,0x81,0x7E,0x83,0x7C,0x87,0x78,0x8F,0x70,0x9F,0x60,0xBF,0x40,0xFF,0x00,
  0xFF,0x70,0xFF,0x98,0xEF,0xB8,0xCF,0xF8,0xFF,0x70,0xFF,0x00,0xFF,0x00,0xFF,0x01,
  0xFF,0x00,0xFE,0x01,0xFC,0x03,0xF8,0x07,0xF0,0x0F,0xE0,0x1F,0xC0,0x3F,0xFF,0xFF,

  /* Tile 0x08 */
  0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0xFF,0xFF,
  0x00,0xFF,0x01,0xFE,0x03,0xFC,0x07,0xF8,0x0F,0xF0,0x1F,0xE0,0x3F,0xC0,0xFF,0xFF,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0xFF,
  0xFF,0x0E,0xFF,0x13,0xFD,0x17,0xF9,0x1F,0xFE,0x0F,0xE0,0x1F,0xC0,0x3F,0x80,0xFF,
  0x01,0xFF,0x01,0xFF,0x01,0xFF,0x01,0xFF,0x01,0xFF,0x01,0xFF,0x01,0xFF,0x01,0xFF,
  0xFF,0x01,0xFF,0x01,0xFD,0x03,0xF9,0x07,0xF1,0x0F,0xE1,0x1F,0xC1,0x3F,0x81,0x7F,
  0x80,0x7F,0x80,0x7F,0x80,0x7F,0x80,0x7F,0x80,0x7F,0x80,0x7F,0x80,0x7F,0x80,0x7F,
  0x01,0xFF,0x01,0xFF,0x03,0xFD,0x07,0xF9,0x0F,0xF1,0x1F,0xE1,0x3F,0xC1,0x7F,0x81,

  /* Tile 0x10 */
  0xFF,0x01,0xFF,0x01,0xFF,0x01,0xFF,0x01,0xFF,0x01,0xFF,0x01,0xFF,0x01,0xFF,0x01,
  0x01,0xFF,0x01,0xFE,0x03,0xFC,0x77,0xF8,0xFF,0x98,0xEF,0xB8,0xCF,0xF8,0x7F,0xF0,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x0E,0xFF,0x13,0xFD,0x17,0xF9,0x1F,0xFF,0x0E,
  0x80,0x7F,0x81,0x7E,0x83,0x7C,0x87,0x78,0x8F,0x70,0x9F,0x60,0xBF,0x40,0xFF,0x7F,
  0x01,0xFF,0x01,0xFF,0x01,0xFF,0x01,0xFF,0x01,0xFF,0x01,0xFF,0x01,0xFF,0xFF,0xFF,

  /* Door1 */

  /* Tile 0x15 */
  0xFF,0x00,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0xFF,
  0x00,0xFF,0x00,0xFF,0x00,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0xFF,0x00,0xFF,0x00,0xFF,
  0x00,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0xFF,0xFF,

  /* Door2 */

  /* Tile 0x18 */
  0x00,0xFF,0x00,0xFF,0x00,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0xFF,0x00,0xFF,0x00,0xFF,
  0x00,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0xFF,0xFF,
  0xFF,0x00,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0xFF,
  0xFF,0x00,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,

  /* Door3 */

  /* Tile 0x1C */
  0x00,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0xFF,0xFF,
  0xFF,0x00,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0xFF,
  0x00,0xFF,0x00,0xFF,0x00,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0xFF,0x00,0xFF,0x00,0xFF,
  0x00,0xFF,0x00,0xFF,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,

  /* Door4 */

  /* Tile 0x20 */
  0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
};

const unsigned char bkg_data[] = {

  /* Tile 0x00 */
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xEF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xDF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFB,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF7,0xF7,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xDF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF7,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xDF,0xFF,0xEF,0xFF,0xFF,0xF7,0xFF,0xFB,0xFF,0xFD,0xFF,0xFE,0xFE,0xFF,

  /* Tile 0x08 */
  0xFF,0xFF,0xFF,0xFF,0xF7,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7D,0xFE,0x7C,0x39,
  0xFF,0xFF,0xF7,0xFF,0xEF,0xFF,0xFF,0xDF,0xFF,0xBF,0xFF,0x7F,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xDF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xFE,0xFF,0xFD,
  0xBB,0x01,0xC7,0x83,0xC7,0x83,0xC7,0x83,0xBB,0x01,0x7C,0x39,0x7D,0xFE,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFD,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,
  0xFF,0xFF,0xFF,0xFF,0xFD,0xFF,0xFF,0xFE,0xFF,0xFF,0xFF,0x7F,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFD,0xFF,0xFF,0xFB,0xAF,0x77,0x27,0x8F,0xDF,0x8F,0x27,0x8F,

  /* Tile 0x10 */
  0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFB,0xFF,0xF7,0xEF,0xFF,0xDF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0xFF,0xFF,0xFB,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xBF,0xFF,0xDF,0xEF,0xFF,0xF7,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFE,0xFD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xAF,0x77,0xFF,0xFB,0xFD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFB,0xFF,

  /* Tile 0x18 */
  0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF7,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFB,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0x7D,0xFE,0x7C,0x39,
  0xFF,0xFF,0xF7,0xFF,0xEF,0xFF,0xFF,0xDF,0xFF,0xBF,0xFF,0x7F,0x7F,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFD,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xEF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0xFD,

  /* Tile 0x20 */
  0xFF,0xFF,0xDF,0xFF,0xEF,0xFF,0xFF,0xF7,0xFF,0xFB,0xFE,0xFD,0xFD,0xFE,0xFE,0xFF,
  0xAB,0x11,0xC7,0x83,0x83,0xC7,0xC7,0x83,0xAB,0x11,0x7C,0x39,0x7D,0xFE,0xFE,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFB,0xDF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0xFF,0xFF,0x7F,
  0xFB,0xFF,0xFF,0xFD,0xFE,0xFE,0xFE,0xFF,0xFE,0xFE,0xFF,0xFD,0xFB,0xFF,0xFF,0xFF,
  0xEF,0xFF,0xFF,0xDF,0x3F,0xBF,0x3F,0x7F,0x3F,0xBF,0xFF,0xDF,0xEF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xEF,0xFB,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFD,0xFE,0xFE,0xFD,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFB,0xFF,0xFF,

  /* Tile 0x28 */
  0xF7,0xFF,0xFB,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
};

/*
 * Image size: 0x40 x 0x40
 * Number of tiles (total - unique): 0x40 - 0x2D
 */

const unsigned char bkg_tiles[] = {
  0x00,0x01,0x02,0x03,0xFC,0xFC,0x04,0xFC,
  0xFC,0x05,0x06,0xFC,0x07,0x08,0x09,0x0A,
  0xFC,0xFC,0xFC,0x02,0x0B,0x0C,0x0D,0xFC,
  0x0E,0x0F,0x10,0xFC,0x11,0x12,0x13,0x14,
  0x15,0x16,0x17,0xFC,0x18,0x19,0x1A,0xFC,
  0x1B,0x1C,0x1D,0xFC,0xFC,0x1E,0x1F,0x20,
  0x21,0x22,0xFC,0x23,0x24,0x25,0xFC,0x26,
  0x27,0x13,0x28,0x29,0x2A,0x2B,0x2C,0x11
};

/*
 * Image size: 0x10 x 0x70
 * Number of tiles (total - unique): 0x1C - 0x1C
 */

const unsigned char earth_tiles[] = {
  0x00,0x02,
  0x04,0x06,
  0x08,0x0A,
  0x0C,0x0E,
  0x10,0x12,
  0x14,0x16,
  0x18,0x1A
};

/*
 * Image size: 0x80 x 0x50
 * Number of tiles (total - unique): 0xA0 - 0x15
 */

const unsigned char frame_tiles[] = {
  0x80,0x81,0xFD,0x82,0x83,0x81,0xFD,0x82,0x83,0x81,0xFD,0x82,0x83,0x81,0xFD,0x84,
  0x85,0x86,0x87,0x88,0x89,0x8A,0x87,0x88,0x89,0x8A,0x87,0x88,0x89,0x8A,0x8B,0x8C,
  0xFD,0x8D,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x8E,0x8F,
  0x82,0x8C,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x85,0x90,
  0x8E,0x8F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFD,0x8D,
  0x85,0x90,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x82,0x8C,
  0xFD,0x8D,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x8E,0x8F,
  0x82,0x8C,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x85,0x90,
  0x8E,0x91,0xFD,0x82,0x83,0x81,0xFD,0x82,0x83,0x81,0xFD,0x82,0x83,0x81,0x92,0x8D,
  0x93,0x8A,0x87,0x88,0x89,0x8A,0x87,0x88,0x89,0x8A,0x87,0x88,0x89,0x8A,0x87,0x94
};

/*
 * Image size: 0x60 x 0x30
 * Number of tiles (total - unique): 0x48 - 0x03
 */

const unsigned char door1_tiles[] = {
  0x95,0x95,0x95,0x95,0x95,0x95,0x95,0x95,0x95,0x95,0x95,0x95,
  0x96,0x96,0x96,0x96,0x96,0x96,0x96,0x96,0x96,0x96,0x96,0x96,
  0x97,0x97,0x97,0x97,0x97,0x97,0x97,0x97,0x97,0x97,0x97,0x97,
  0x95,0x95,0x95,0x95,0x95,0x95,0x95,0x95,0x95,0x95,0x95,0x95,
  0x96,0x96,0x96,0x96,0x96,0x96,0x96,0x96,0x96,0x96,0x96,0x96,
  0x97,0x97,0x97,0x97,0x97,0x97,0x97,0x97,0x97,0x97,0x97,0x97,

  0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,
  0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,
  0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,
  0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,
  0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,
  0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC
};

/*
 * Image size: 0x60 x 0x30
 * Number of tiles (total - unique): 0x48 - 0x04
 */

const unsigned char door2_tiles[] = {
  0x98,0x98,0x98,0x98,0x98,0x98,0x98,0x98,0x98,0x98,0x98,0x98,
  0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,
  0x9A,0x9A,0x9A,0x9A,0x9A,0x9A,0x9A,0x9A,0x9A,0x9A,0x9A,0x9A,
  0x98,0x98,0x98,0x98,0x98,0x98,0x98,0x98,0x98,0x98,0x98,0x98,
  0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,
  0x9B,0x9B,0x9B,0x9B,0x9B,0x9B,0x9B,0x9B,0x9B,0x9B,0x9B,0x9B,

  0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,
  0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,
  0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,
  0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,
  0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,
};

/*
 * Image size: 0x60 x 0x30
 * Number of tiles (total - unique): 0x48 - 0x04
 */

const unsigned char door3_tiles[] = {
  0x9C,0x9C,0x9C,0x9C,0x9C,0x9C,0x9C,0x9C,0x9C,0x9C,0x9C,0x9C,
  0x9D,0x9D,0x9D,0x9D,0x9D,0x9D,0x9D,0x9D,0x9D,0x9D,0x9D,0x9D,
  0x9E,0x9E,0x9E,0x9E,0x9E,0x9E,0x9E,0x9E,0x9E,0x9E,0x9E,0x9E,
  0x9C,0x9C,0x9C,0x9C,0x9C,0x9C,0x9C,0x9C,0x9C,0x9C,0x9C,0x9C,
  0x9D,0x9D,0x9D,0x9D,0x9D,0x9D,0x9D,0x9D,0x9D,0x9D,0x9D,0x9D,
  0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,0x9F,

  0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,
  0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,
  0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,
  0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,
  0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC
};

/*
 * Image size: 0x60 x 0x30
 * Number of tiles (total - unique): 0x48 - 0x01
 */

const unsigned char door4_tiles[] = {
  0x95,0x95,0x95,0x95,0x95,0x95,0x95,0x95,0x95,0x95,0x95,0x95,
  0x96,0x96,0x96,0x96,0x96,0x96,0x96,0x96,0x96,0x96,0x96,0x96,
  0x97,0x97,0x97,0x97,0x97,0x97,0x97,0x97,0x97,0x97,0x97,0x97,
  0x95,0x95,0x95,0x95,0x95,0x95,0x95,0x95,0x95,0x95,0x95,0x95,
  0x96,0x96,0x96,0x96,0x96,0x96,0x96,0x96,0x96,0x96,0x96,0x96,
  0xA0,0xA0,0xA0,0xA0,0xA0,0xA0,0xA0,0xA0,0xA0,0xA0,0xA0,0xA0,

  0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,
  0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,
  0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,
  0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,
  0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC
};

/* Should really be const, but sdcc doesnt yet support it. */
const unsigned char * const film[] = {
  &door1_tiles[0x0C*0],
  &door2_tiles[0x0C*0],
  &door3_tiles[0x0C*0],
  &door4_tiles[0x0C*0],
  &door1_tiles[0x0C*1],
  &door2_tiles[0x0C*1],
  &door3_tiles[0x0C*1],
  &door4_tiles[0x0C*1],
  &door1_tiles[0x0C*2],
  &door2_tiles[0x0C*2],
  &door3_tiles[0x0C*2],
  &door4_tiles[0x0C*2],
  &door1_tiles[0x0C*3],
  &door2_tiles[0x0C*3],
  &door3_tiles[0x0C*3],
  &door4_tiles[0x0C*3],
  &door1_tiles[0x0C*4],
  &door2_tiles[0x0C*4],
  &door3_tiles[0x0C*4],
  &door4_tiles[0x0C*4],
  &door1_tiles[0x0C*5],
  &door2_tiles[0x0C*5],
  &door3_tiles[0x0C*5],
  &door4_tiles[0x0C*5],
  &door1_tiles[0x0C*6]
};

#define NBDFRAMES 0x18                  /* Nb frames for the door */
#define NBSFRAMES 0x07                  /* Nb frames for the sprite */
#define WINSZX    0x80u                  /* Size of the picture in the window */
#define WINSZY    0x50u
#define MINWINX   ((uint8_t)MAXWNDPOSX-WINSZX + 1u) /* Bounds of the window origin */
#define MINWINY   ((uint8_t)MAXWNDPOSY-WINSZY + 1u)
#define MAXWINX   ((uint8_t)MAXWNDPOSX)
#define MAXWINY   ((uint8_t)MAXWNDPOSY)
#define FADESTEP  0x10                  /* Nb steps for the fading effect */
#define STARTFADE (0x06*FADESTEP)       /* Initial value for the fading effect */

#define CLOSED  0x00
#define OPENING 0x01
#define OPENED  0x02
#define CLOSING 0x03

static uint8_t time = 0;  /* Global "time" value (counter) */
uint8_t doorstate = 0;    /* State of the door (OPENED, CLOSED...) */
uint8_t doorpos = 0;      /* Current position in the door animation */
static uint8_t color = 0; /* Current color for fading effect */
uint8_t sframe = 0;       /* Current frame of the sprite */
fixed bposx, bposy; /* Background position (fixed point) */
fixed bspx, bspy;   /* Background speed (fixed point) */
fixed wposx, wposy; /* Window position (fixed point) */
fixed wspx, wspy;   /* Window speed (fixed point) */
fixed sposx, sposy; /* Sprite position (fixed point) */
fixed sspx, sspy;   /* Sprite speed (fixed point) */

void fade(void);
void scroll(void);
void door(void);
void animate_sprite(void);
void tile_sprite(void);
void place_sprite(void);

/* Fade the screen (off and on) */
void fade(void)
{
  if(color == 0)
    return;
  switch(color)
    {
    case STARTFADE:
    case STARTFADE-4*FADESTEP:
      BGP_REG = 0xF9U;
      break;
    case STARTFADE-FADESTEP:
    case STARTFADE-3*FADESTEP:
      BGP_REG = 0xFEU;
      break;
    case STARTFADE-2*FADESTEP:
      BGP_REG = 0xFFU;
      break;
    case STARTFADE-5*FADESTEP:
      BGP_REG = 0xE4U;
      break;
    }
  color--;
}

/* Scroll the background, the window and the sprite */
void scroll(void)
{
  /* Update background */
  bposx.w += bspx.w;
  bposy.w += bspy.w;
  SCX_REG = bposx.b.h;
  SCY_REG = bposy.b.h;

  /* Update window */
  wposx.w += wspx.w ;
  wposy.w += wspy.w ;
  /* X position */
  if(wposx.b.h >= MAXWINX) {
    wposx.b.h = MAXWINX;
    /* Invert speed */
    wspx.w = -(int16_t)wspx.w;
  } else if(wposx.b.h <= MINWINX) {
    wposx.b.h = MINWINX;
    /* Invert speed */
    wspx.w = -(int16_t)wspx.w;
  }
  WX_REG = wposx.b.h;
  /* Y position */
  if(wposy.b.h >= MAXWINY) {
    wposy.b.h = MAXWINY;
    /* Invert speed */
    wspy.w = -(int16_t)wspy.w;
  } else if(wposy.b.h <= MINWINY) {
    wposy.b.h = MINWINY;
    /* Invert speed */
    wspy.w = -(int16_t)wspy.w;
  }
  WY_REG = wposy.b.h;

  /* Update sprite */
  sposx.w += sspx.w;
  sposy.w += sspy.w;
  place_sprite();
}

/* Open and close the door */
void door(void)
{
  if(doorstate == OPENING) {
    doorpos++;
    /* Draw the door in the window */
    set_win_tiles(2, 2, 12, 6, (unsigned char*)film[doorpos]);
    if(doorpos == NBDFRAMES)
      doorstate = OPENED;
  } else if(doorstate == CLOSING) {
    doorpos--;
    /* Draw the door in the window */
    set_win_tiles(2, 2, 12, 6, (unsigned char*)film[doorpos]);
    if(doorpos == 0)
      doorstate = CLOSED;
  }
}

/* Animate sprite */
void animate_sprite(void)
{
  if((time&0x07) == 0) {
    sframe++;
    if(sframe == NBSFRAMES)
      sframe = 0;
    tile_sprite();
  }
}

/* Set sprite tiles */
void tile_sprite(void)
{
  uint8_t s;

  s = sframe<<1;
  set_sprite_tile(0, earth_tiles[s]);
  set_sprite_tile(1, earth_tiles[s+1]);
}

/* Place sprite */
void place_sprite(void)
{
  move_sprite(0, sposx.b.h, sposy.b.h);
  move_sprite(1, sposx.b.h+8, sposy.b.h);
}

void main(void)
{
  uint8_t i, j;

  disable_interrupts();
  DISPLAY_OFF;
  LCDC_REG = LCDCF_OFF | LCDCF_WIN9C00 | LCDCF_WINON | LCDCF_BG8800 | LCDCF_BG9800 | LCDCF_OBJ16 | LCDCF_OBJON | LCDCF_BGON;
  /*
   * LCD        = Off
   * WindowBank = 0x9C00
   * Window     = On
   * BG Chr     = 0x8800
   * BG Bank    = 0x9800
   * OBJ        = 8x16
   * OBJ        = On
   * BG         = On
   */

  doorstate = CLOSED;

  /* Set palettes */
  BGP_REG = OBP0_REG = OBP1_REG = 0xE4U;

  /* Initialize the background */
  set_bkg_data(0xFC, 0x04, std_data);
  set_bkg_data(0x00, 0x2D, bkg_data);
  /*
   * Draw the background
   *
   * Width  = 0x100 = 0x20 * 8
   * Height = 0x100 = 0x20 * 8
   */
  for(i = 0; i < 32; i+=8)
    for(j = 0; j < 32; j+=8)
      set_bkg_tiles(i, j, 8, 8, bkg_tiles);
  bposx.w = 0;
  SCX_REG = 0;
  bposy.w = 0;
  SCY_REG = 0;
  bspx.w = 0xFF00;
  bspy.w = 0x0080;

  /* Initialize the window */
  set_win_data(0x80, 0x21, frame_data);
  /*
   * Draw the frame in the window
   *
   * Width  = 0x80 = 0x10 * 8
   * Height = 0x50 = 0x0A * 8
   */
  set_win_tiles(0, 0, 16, 10, frame_tiles);
  /*
   * Draw the door in the window
   *
   * Width  = 0x60 = 0x20 * 12
   * Height = 0x30 = 0x20 * 6
   */
  set_win_tiles(2, 2, 12, 6, door1_tiles);
  wposx.b.h = MAXWINX;
  wposx.b.l = 0;
  WX_REG = MAXWINX;
  wposy.b.h = MAXWINY;
  wposy.b.l = 0;
  WY_REG = MAXWINY;
  wspx.w = 0xFF80;
  wspy.w = 0xFFC0;

  /* Initialize the sprite */
  set_sprite_data(0x00, 0x1C, earth_data);
  set_sprite_prop(0, 0x00);
  set_sprite_prop(1, 0x00);
  sframe = 0;
  sposx.w  = 0x1000;
  sposy.w  = 0x1000;
  sspx.w  = 0x0040;
  sspy.w  = 0x0040;
  tile_sprite();
  place_sprite();

  DISPLAY_ON;
  enable_interrupts();

  while(1) {
    /* Skip four VBLs (slow down animation) */
    for(i = 0; i < 4; i++)
      vsync();
    time++;
    fade();
    door();
    scroll();
    animate_sprite();
    i = joypad();
    if(i & J_B) {
      if(i & J_UP)
    bspy.w -= 0x0010;
      if(i & J_DOWN)
    bspy.w += 0x0010;
      if(i & J_LEFT)
    bspx.w -= 0x0010;
      if(i & J_RIGHT)
    bspx.w += 0x0010;
    } else if(i & J_A) {
      if(i & J_UP)
    wspy.w -= 0x0010;
      if(i & J_DOWN)
    wspy.w += 0x0010;
      if(i & J_LEFT)
    wspx.w -= 0x0010;
      if(i & J_RIGHT)
    wspx.w += 0x0010;
    } else {
      if(i & J_SELECT)
    color = STARTFADE;
      if(i & J_START)
    if(doorstate == CLOSED) {
      doorstate = OPENING;
      doorpos = 0;
    } else if(doorstate == OPENED) {
      doorstate = CLOSING;
      doorpos = NBDFRAMES;
    }
      if(i & J_UP)
    sspy.w -= 0x0010;
      if(i & J_DOWN)
    sspy.w += 0x0010;
      if(i & J_LEFT)
    sspx.w -= 0x0010;
      if(i & J_RIGHT)
    sspx.w += 0x0010;
    }
  }
}
