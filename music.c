
/*
A simple music player.
*/

#include <string.h>
#include <nes.h>

#include "neslib.h"

// VRAM buffer module
#include "vrambuf.h"
//#link "vrambuf.c"

#include "apu.h"
//#link "apu.c"

// link the pattern table into CHR ROM
//#link "chr_generic.s"

//
// MUSIC ROUTINES
//

// Namespace(bias=1.0, freq=111860.8, length=64, maxbits=13.0, upper=41)
// 439.0 0.943191918851 41
const int note_table_41[64] = {
4318, 4076, 3847, 3631, 3427, 3235, 3053, 2882, 2720, 2567, 2423, 2287, 2159, 2037, 1923, 1815, 1713, 1617, 1526, 1440, 1360, 1283, 1211, 1143, 1079, 1018, 961, 907, 856, 808, 763, 720, 679, 641, 605, 571, 539, 509, 480, 453, 428, 403, 381, 359, 339, 320, 302, 285, 269, 254, 240, 226, 213, 201, 190, 179, 169, 160, 151, 142, 134, 126, 119, 113, };

// Namespace(bias=1.0, freq=111860.8, length=64, maxbits=13, upper=49)
// 440.5 1.79281159771 49
const int note_table_49[64] = {
4304, 4062, 3834, 3619, 3416, 3224, 3043, 2872, 2711, 2559, 2415, 2279, 2151, 2031, 1917, 1809, 1707, 1611, 1521, 1436, 1355, 1279, 1207, 1139, 1075, 1015, 958, 904, 853, 805, 760, 717, 677, 639, 603, 569, 537, 507, 478, 451, 426, 402, 379, 358, 338, 319, 301, 284, 268, 253, 239, 225, 213, 201, 189, 179, 168, 159, 150, 142, 134, 126, 119, 112, };

// Namespace(bias=1.0, freq=111860.8, length=64, maxbits=12, upper=63)
// 443.6 14.2328382554 63
const int note_table_63[64] = {
2137, 4034, 3807, 3593, 3392, 3201, 3022, 2852, 2692, 2541, 2398, 2263, 2136, 2016, 1903, 1796, 1695, 1600, 1510, 1425, 1345, 1270, 1199, 1131, 1068, 1008, 951, 898, 847, 800, 755, 712, 672, 634, 599, 565, 533, 503, 475, 448, 423, 399, 377, 356, 336, 317, 299, 282, 266, 251, 237, 224, 211, 199, 188, 177, 167, 158, 149, 141, 133, 125, 118, 111, };

// Namespace(bias=-1.0, freq=55930.4, length=64, maxbits=12, upper=53)
// 443.7 8.47550713772 53
const int note_table_tri[64] = {
2138, 2018, 1905, 1798, 1697, 1602, 1512, 1427, 1347, 1272, 1200, 1133, 1069, 1009, 953, 899, 849, 801, 756, 714, 674, 636, 601, 567, 535, 505, 477, 450, 425, 401, 379, 358, 338, 319, 301, 284, 268, 253, 239, 226, 213, 201, 190, 179, 169, 160, 151, 142, 135, 127, 120, 113, 107, 101, 95, 90, 85, 80, 76, 72, 68, 64, 60, 57, };

#define NOTE_TABLE note_table_49
#define BASS_NOTE 36

byte music_index = 0;
byte cur_duration = 0;

const byte music1[]; // music data -- see end of file
const byte music2[]; // music data -- see end of file
const byte music3[]; // music data -- see end of file
const byte music4[]; // music data -- see end of file
const byte music5[]; // music data -- see end of file
const byte music6[]; // music data -- see end of file
const byte music7[]; // music data -- see end of file
const byte music8[]; // music data -- see end of file
const byte music9[]; // music data -- see end of file
const byte music10[]; // music data -- see end of file
const byte music11[]; // music data -- see end of file
const byte music12[]; // music data -- see end of file
const byte* music_ptr = music1;

byte next_music_byte() {
  return *music_ptr++;
}

void play_music() {
  static byte chs = 0;
  if (music_ptr) {
    // run out duration timer yet?
    while (cur_duration == 0) {
      // fetch next byte in score
      byte note = next_music_byte();
      // is this a note?
      if ((note & 0x80) == 0) {
        // pulse plays higher notes, triangle for lower if it's free
        if (note >= BASS_NOTE || (chs & 4)) {
          int period = NOTE_TABLE[note & 63];
          // see which pulse generator is free
          if (!(chs & 1)) {
            APU_PULSE_DECAY(0, period, DUTY_50, 2, 15);
            chs |= 1;
          } else if (!(chs & 2)) {
            APU_PULSE_DECAY(1, period, DUTY_50, 2, 10);
            chs |= 2;
          }
        } else {
          int period = note_table_tri[note & 63];
          APU_TRIANGLE_LENGTH(period, 15);
          chs |= 4;
        }
      } else {
        // end of score marker
        if (note == 0xff)
          music_ptr = NULL;
        // set duration until next note
        cur_duration = note & 63;
        // reset channel used mask
        chs = 0;
      }
    }
    cur_duration--;
  }
}

void start_music(const byte* music) {
  music_ptr = music;
  cur_duration = 0;
}

void main(void)
{
  apu_init();
  music_ptr = 0;
  while (1) {
    // set palette colors
    pal_col(0,0x02);	// set screen to dark blue
    pal_col(1,0x14);	// fuchsia
    pal_col(2,0x20);	// grey
    pal_col(3,0x30);	// white

    // write text to name table
    waitvsync();
    vram_adr(NTADR_A(2,2));		// set address
    vram_write("HELLO, WORLD!", 13);	// write bytes to video RAM

    // clear vram buffer
    vrambuf_clear();
  
    // set NMI handler
    set_vram_update(updbuf);

    // enable PPU rendering (turn on screen)
    ppu_on_all();
    
    if (!music_ptr) start_music(music5);
    play_music();
  }
}

//
// MUSIC DATA 1 -- "Celina" by Kult - Round 1 - City
//
const byte music1[] = {
0x2c,0x86,0x2f,0x8c,0x31,0x87,0x33,0x20,0x14,0x8c,0x33,0x81,0x20,0x14,0x86,0x2f,0x20,0x81,0x14,0x8b,0x2f,0x81,0x20,0x14,0x87,0x2c,0x20,0x14,0x8c,0x2c,0x20,0x81,0x14,0x86,0x2f,0x25,0x19,0x86,0x27,0x1b,0x86,0x31,0x81,0x2a,0x1e,0x86,0x33,0x20,0x14,0x8c,0x33,0x81,0x20,0x14,0x86,0x2f,0x20,0x14,0x8c,0x2f,0x81,0x20,0x14,0x86,0x2c,0x20,0x81,0x14,0x8b,0x2c,0x81,0x20,0x14,0x87,0x2f,0x25,0x19,0x86,0x27,0x1b,0x86,0x31,0x2a,0x81,0x1e,0x86,0x32,0x20,0x14,0x8c,0x33,0x81,0x20,0x14,0x86,0x20,0x14,0x8c,0x2f,0x81,0x20,0x14,0x86,0x31,0x20,0x14,0x8d,0x33,0x20,0x14,0x86,0x25,0x19,0x87,0x27,0x1b,0x86,0x33,0x2a,0x1e,0x87,0x20,0x14,0x8c,0x33,0x20,0x81,0x14,0x86,0x2f,0x20,0x14,0x8c,0x2c,0x81,0x20,0x14,0x86,0x20,0x14,0x8d,0x20,0x14,0x86,0x25,0x19,0x87,0x27,0x1b,0x85,0x33,0x81,0x2a,0x1e,0x86,0x34,0x25,0x81,0x19,0x89,0x34,0x83,0x25,0x19,0x87,0x31,0x25,0x19,0x88,0x31,0x84,0x25,0x81,0x19,0x86,0x2c,0x25,0x19,0x8d,0x25,0x19,0x86,0x2a,0x1e,0x86,0x2c,0x81,0x20,0x85,0x33,0x81,0x2f,0x23,0x86,0x34,0x20,0x14,0x89,0x34,0x84,0x20,0x14,0x86,0x31,0x20,0x81,0x14,0x8b,0x31,0x81,0x20,0x14,0x87,0x2c,0x25,0x19,0x8b,0x2c,0x81,0x25,0x81,0x19,0x86,0x2f,0x2a,0x1e,0x86,0x2c,0x20,0x86,0x31,0x81,0x2f,0x23,0x85,0x32,0x81,0x20,0x14,0x8d,0x20,0x14,0x33,0x86,0x20,0x14,0x81,0x2f,0x8b,0x2f,0x81,0x20,0x14,0x86,0x20,0x81,0x14,0x31,0x88,0x33,0x84,0x20,0x14,0x86,0x2c,0x81,0x25,0x19,0x86,0x2f,0x27,0x1b,0x86,0x2a,0x81,0x1e,0x86,0x33,0x20,0x14,0x89,0x33,0x84,0x20,0x14,0x86,0x2f,0x20,0x14,0x86,0x2f,0x87,0x20,0x14,0x85,0x2c,0x81,0x20,0x14,0x8d,0x20,0x14,0x81,0x2e,0x83,0x2f,0x82,0x2e,0x25,0x19,0x87,0x27,0x1b,0x85,0x2e,0x81,0x2a,0x1e,0x86,0x33,0x81,0x27,0x2e,0x33,0x8d,0x1b,0x85,0x31,0x81,0x27,0x2e,0x33,0x8d,0x1b,0x85,0x2e,0x81,0x27,0x2e,0x1b,0x88,0x2f,0x83,0x2e,0x82,0x1b,0x85,0x2e,0x81,0x27,0x2e,0x1b,0x8b,0x2e,0x82,0x1b,0x86,0x33,0x27,0x2e,0x81,0x1b,0x8c,0x33,0x1b,0x87,0x27,0x2e,0x1b,0x87,0x33,0x86,0x1b,0x86,0x2e,0x27,0x2e,0x8d,0x2e,0x1b,0x86,0x2c,0x27,0x2e,0x89,0x2b,0x84,0x1b,0x86,0x20,0x27,0x14,0x87,0x2c,0x86,0x14,0x86,0x20,0x27,0x81,0x14,0x8c,0x14,0x87,0x20,0x27,0x14,0x8d,0x14,0x86,0x20,0x27,0x14,0x86,0x33,0x87,0x14,0x81,0x33,0x8f,0x25,0x2c,0x83,0x19,0x83,0x26,0x2d,0x83,0x1a,0x8d,0x27,0x2e,0x1b,0xad,0x20,0xff
};

//
// MUSIC DATA 2 -- "Tacy Sami" by Lady Pank - Round 2 - Water
//
const byte music2[] = {
0x1d,0xa4,0x35,0x8c,0x3c,0x98,0x3a,0x8b,0x38,0x8c,0x11,0x35,0x8c,0x1d,0x92,0x16,0x86,0x18,0x8c,0x1b,0x97,0x18,0x8c,0x14,0x98,0x14,0x8c,0x16,0x8c,0x18,0x8c,0x1b,0x8b,0x17,0x3a,0x8c,0x16,0x3a,0x88,0x39,0x84,0x14,0x38,0x8c,0x11,0x3a,0x8c,0x1d,0x8c,0x3a,0x86,0x16,0x85,0x18,0x8c,0x1b,0x3a,0x8c,0x3a,0x8c,0x18,0x8c,0x14,0x3a,0x98,0x14,0x38,0x8b,0x16,0x8c,0x18,0x8c,0x1b,0x3a,0x8c,0x18,0x3c,0x8c,0x1d,0x3a,0x8c,0x1d,0x38,0x8c,0x11,0x35,0x8b,0x1d,0x92,0x16,0x86,0x18,0x8c,0x1b,0x98,0x18,0x8c,0x14,0x97,0x14,0x8c,0x16,0x8c,0x18,0x35,0x8c,0x1b,0x3c,0x8c,0x17,0x8b,0x16,0x3a,0x8c,0x14,0x38,0x8c,0x11,0x3a,0x8c,0x1d,0x8c,0x3a,0x86,0x16,0x86,0x18,0x8c,0x1b,0x3a,0x8b,0x3c,0x8c,0x18,0x8c,0x14,0x3a,0x98,0x14,0x38,0x8c,0x16,0x8c,0x18,0x35,0x8b,0x1b,0x3c,0x8c,0x18,0x8c,0x1d,0x3a,0x8c,0x1d,0x38,0x8c,0x11,0x35,0x8c,0x1d,0x3a,0x8c,0x3a,0x85,0x16,0x86,0x18,0x38,0x8c,0x1b,0x3a,0x8c,0x3a,0x8c,0x18,0x3a,0x8c,0x14,0x38,0x97,0x14,0x3a,0x8c,0x16,0x3a,0x8c,0x18,0x38,0x8c,0x1b,0x8c,0x17,0x3c,0x8c,0x16,0x3a,0x8c,0x14,0x38,0x8b,0x11,0x3a,0x8c,0x1d,0x8c,0x3a,0x86,0x16,0x86,0x18,0x8c,0x1b,0x3a,0x8c,0x3a,0x8c,0x18,0x8b,0x14,0x3a,0x98,0x14,0x3c,0x3a,0x8c,0x16,0x3a,0x8c,0x18,0x35,0x38,0x8c,0x1b,0x3c,0x8c,0x18,0x8b,0x1d,0x3a,0x8c,0x1d,0x38,0x8c,0x11,0x35,0x8c,0x1d,0x3a,0x8c,0x3a,0x86,0x16,0x86,0x18,0x38,0x8b,0x1b,0x3a,0x8c,0x3a,0x8c,0x18,0x3a,0x8c,0x14,0x38,0x98,0x14,0x3a,0x8c,0x16,0x3a,0x8b,0x18,0x38,0x8c,0x1b,0x3c,0x3a,0x8c,0x17,0x3c,0x3a,0x8c,0x16,0x3a,0x3a,0x8c,0x14,0x38,0x38,0x8c,0x11,0x3a,0x8c,0x1d,0x3a,0x8b,0x3a,0x3a,0x86,0x16,0x86,0x18,0x38,0x8c,0x1b,0x3a,0x8c,0x3c,0x3a,0x8c,0x18,0x3a,0x8c,0x14,0x3a,0x38,0x97,0x14,0x38,0x8c,0x16,0x8c,0x18,0x8c,0x1b,0x8c,0x18,0x8c,0x20,0x97,0x1d,0x98,0x19,0xbb,0x1b,0xb0,0x35,0x8c,0x3c,0x97,0x3a,0x8c,0x38,0x8c,0x11,0x35,0x8c,0x1d,0x92,0x16,0x86,0x18,0x8b,0x1b,0x98,0x18,0x8c,0x14,0x98,0x14,0x8b,0x16,0x8c,0x18,0x8c,0x1b,0x3a,0x8c,0x17,0x3c,0x8c,0x16,0x3a,0x8c,0x14,0x38,0x8c,0x11,0x3a,0x8b,0x1d,0x3a,0x3a,0x8c,0x3a,0x86,0x16,0x86,0x18,0x3a,0x38,0x8c,0x1b,0x3a,0x8c,0x3a,0x3a,0x8c,0x18,0x3a,0x8c,0x14,0x3a,0x38,0x97,0x14,0x38,0x3a,0x8c,0x16,0x3a,0x8c,0x18,0x38,0x8c,0x1b,0x3a,0x3a,0x8c,0x18,0x3c,0x3a,0x8c,0x1d,0x3a,0x3a,0x8b,0x1d,0x38,0x38,0x8c,0x11,0x35,0x8c,0x1d,0x92,0x16,0x86,0x18,0x8c,0x1b,0x97,0x18,0x8c,0x14,0x98,0x14,0x8c,0x16,0x8c,0x18,0x86,0x35,0x85,0x1b,0x3c,0x8c,0x17,0x8c,0x16,0x3a,0x8c,0x14,0x38,0x8c,0x11,0x3a,0x8c,0x1d,0x3a,0x3a,0x8c,0x3a,0x85,0x16,0x86,0x18,0x3a,0x38,0x8c,0x1b,0x3a,0x8c,0x3a,0x3a,0x8c,0x18,0x3a,0x8c,0x14,0x3a,0x38,0x98,0x14,0x38,0x3a,0x8b,0x16,0x3a,0x8c,0x18,0x35,0x38,0x8c,0x1b,0x3c,0x3a,0x8c,0x18,0x3a,0x8c,0x1d,0x3a,0x3a,0x8c,0x1d,0x38,0x38,0x8b,0x11,0x35,0x8c,0x1d,0x92,0x16,0x86,0x18,0x8c,0x1b,0x98,0x18,0x8b,0x14,0x98,0x14,0x8c,0x16,0x8c,0x18,0x8c,0x1b,0x8c,0x17,0x8b,0x16,0x3a,0x8c,0x14,0x38,0x8c,0x11,0x3a,0x8c,0x1d,0x3a,0x3a,0x8c,0x3a,0x86,0x16,0x86,0x18,0x3a,0x38,0x8c,0x1b,0x3a,0x8b,0x3a,0x3a,0x8c,0x18,0x3a,0x8c,0x14,0x3a,0x38,0x98,0x14,0x38,0x3a,0x8c,0x16,0x3a,0x8b,0x18,0x35,0x38,0x8c,0x1b,0x3c,0x3a,0x8c,0x18,0x3a,0x8c,0x1d,0x3a,0x3a,0x8c,0x1d,0x38,0x38,0x8c,0x11,0x35,0x8c,0x1d,0x91,0x16,0x86,0x18,0x8c,0x1b,0x98,0x18,0x8c,0x14,0x97,0x14,0x8c,0x16,0x8c,0x18,0x8c,0x1b,0x8c,0x17,0x8c,0x16,0x3a,0x8c,0x14,0x38,0x8b,0x11,0x3a,0x8c,0x1d,0x3a,0x3a,0x8c,0x3a,0x86,0x16,0x86,0x18,0x3a,0x38,0x8c,0x1b,0x3a,0x8c,0x3a,0x3a,0x8b,0x18,0x3a,0x8c,0x14,0x3a,0x38,0x98,0x14,0x38,0x3a,0x8c,0x16,0x3a,0x8c,0x18,0x38,0x8c,0x1b,0x3a,0x8b,0x18,0x3a,0x8c,0x20,0x3a,0x8c,0x38,0x8c,0x19,0xa4,0x16,0xbb,0x1b,0x8c,0x1b,0x8c,0x1b,0x8b,0x1b,0x8c,0x1b,0xb0,0x1d,0x38,0x29,0x8b,0x1b,0x27,0x2e,0x8c,0x1d,0x35,0x30,0x8c,0x19,0x25,0x2c,0x8c,0x19,0x38,0x98,0x19,0x35,0x2c,0x8c,0x1b,0x25,0x2c,0x8b,0x14,0x27,0x20,0x8c,0x14,0x3c,0x20,0x8c,0x3c,0x20,0x27,0x8c,0x14,0x3d,0x20,0x8c,0x1b,0x3c,0x2e,0x8c,0x3a,0x27,0x2e,0x8c,0x18,0x38,0x2e,0x8b,0x1b,0x37,0x27,0x8c,0x1d,0x38,0x30,0x8c,0x1b,0x27,0x2e,0x8c,0x1d,0x35,0x30,0x8c,0x19,0x25,0x2c,0x8c,0x19,0x38,0x97,0x19,0x35,0x2c,0x8c,0x1b,0x25,0x2c,0x8c,0x14,0x20,0x27,0x8c,0x14,0x27,0x20,0x8c,0x20,0x27,0x8c,0x14,0x20,0x27,0x8b,0x1b,0x2e,0x27,0x8c,0x2e,0x27,0x8c,0x1b,0x2e,0x27,0x8c,0x1b,0x27,0x2e,0x8c,0x1d,0x38,0x30,0x8c,0x1b,0x2e,0x27,0x8c,0x1d,0x35,0x30,0x8b,0x19,0x25,0x2c,0x8c,0x19,0x38,0x98,0x19,0x35,0x25,0x8c,0x19,0x25,0x2c,0x8c,0x14,0x20,0x27,0x8c,0x14,0x3c,0x27,0x8b,0x3c,0x27,0x20,0x8c,0x14,0x3d,0x20,0x8c,0x1b,0x3c,0x2e,0x8c,0x3a,0x2e,0x27,0x8c,0x18,0x38,0x27,0x8c,0x1b,0x37,0x2e,0x8c,0x1d,0x38,0x30,0x8b,0x1b,0x2e,0x27,0x8c,0x1d,0x35,0x29,0x8c,0x19,0x2c,0x25,0x8c,0x19,0x38,0x98,0x19,0x35,0x25,0x8b,0x19,0x2c,0x25,0x8c,0x1b,0x2e,0x27,0x8c,0x1b,0x27,0x2e,0x8c,0x1b,0x27,0x2e,0x8c,0x1b,0x27,0x2e,0x8c,0x1b,0x27,0x2e,0x8c,0x1b,0x2e,0x27,0x8b,0x1b,0x27,0x2e,0x8c,0x1b,0x2e,0x27,0x8c,0x18,0x27,0x2e,0x8c,0x1b,0x2e,0x27,0xff
};

//
// MUSIC DATA 3 -- "Temptation" by Arash - Round 3 - Desert
//
const byte music3[] = {
0x3f,0x33,0x8d,0x42,0x2c,0x36,0x87,0x3f,0x33,0x8d,0x3f,0x2c,0x33,0x87,0x27,0x8d,0x3b,0x2a,0x2f,0x8d,0x3d,0x25,0x31,0x94,0x3b,0x25,0x2f,0x87,0x20,0x8d,0x3b,0x23,0x2f,0x8e,0x3d,0x25,0x31,0x94,0x3d,0x25,0x31,0x86,0x20,0x8e,0x3d,0x23,0x31,0x8d,0x3b,0x2a,0x2f,0x8d,0x3a,0x2e,0x87,0x2a,0x87,0x38,0x23,0x2c,0x8d,0x36,0x22,0x2a,0x87,0x36,0x2a,0x86,0x38,0x2c,0x2c,0x94,0x38,0x2c,0x2c,0x87,0x27,0x8d,0x3a,0x2a,0x2e,0x8e,0x3b,0x28,0x2f,0x94,0x3d,0x28,0x31,0x86,0x23,0x8e,0x3b,0x2f,0x8d,0x3b,0x2a,0x2f,0x8d,0x3a,0x2e,0x87,0x2a,0x87,0x38,0x25,0x2c,0x8d,0x36,0x2a,0x8d,0x38,0x2c,0x2c,0x94,0x2c,0x87,0x27,0x8d,0x3d,0x2a,0x31,0x8e,0x3f,0x2c,0x33,0x94,0x3f,0x2c,0x33,0x86,0x27,0x8e,0x3b,0x2a,0x2f,0x8d,0x3d,0x25,0x31,0x94,0x3b,0x25,0x2f,0x87,0x20,0x8d,0x3b,0x23,0x2f,0x8d,0x3d,0x25,0x31,0x94,0x3d,0x25,0x31,0x87,0x20,0x8d,0x3d,0x23,0x31,0x8e,0x3b,0x2a,0x2f,0x8d,0x3a,0x2e,0x87,0x2a,0x86,0x38,0x23,0x2c,0x8e,0x36,0x22,0x2a,0x86,0x36,0x2a,0x87,0x38,0x2c,0x2c,0x94,0x38,0x2c,0x2c,0x87,0x27,0x8d,0x3a,0x2a,0x2e,0x8d,0x3b,0x28,0x2f,0x94,0x3d,0x28,0x31,0x87,0x23,0x8d,0x3b,0x2f,0x8e,0x3b,0x2a,0x2f,0x8d,0x3a,0x2e,0x87,0x2a,0x86,0x38,0x25,0x2c,0x8e,0x36,0x2a,0x8d,0x38,0x2c,0x2c,0xff
};

//
// MUSIC DATA 4 -- "Figlarka" by Akcent - Round 4 - Jungle
//
const byte music4[] = {
0x30,0x95,0x2e,0x95,0x2c,0x89,0x31,0x8c,0x16,0x16,0x8b,0x22,0x8a,0x3a,0x16,0x8b,0x22,0x8a,0x3c,0x16,0x8b,0x22,0x8a,0x3d,0x16,0x8b,0x33,0x22,0x8b,0x0f,0x8a,0x1b,0x8b,0x0f,0x2e,0x8a,0x1b,0x8b,0x2c,0x0f,0x8b,0x1b,0x8a,0x2b,0x0f,0x8b,0x1b,0x30,0x8a,0x14,0x8b,0x20,0x8b,0x3c,0x14,0x8a,0x20,0x8b,0x3d,0x14,0x8a,0x20,0x8b,0x3f,0x14,0x8a,0x38,0x20,0x8b,0x14,0x8b,0x20,0x8a,0x14,0x8b,0x20,0x8a,0x14,0x8b,0x20,0x85,0x20,0x86,0x14,0x8a,0x20,0x8b,0x16,0x8a,0x22,0x8b,0x3a,0x16,0x8b,0x22,0x8a,0x3c,0x16,0x8b,0x22,0x8a,0x3d,0x16,0x8b,0x22,0x8a,0x3f,0x0f,0x8b,0x1b,0x8b,0x0f,0x8a,0x1b,0x8b,0x0f,0x8a,0x1b,0x8b,0x0f,0x8b,0x1b,0x8a,0x3c,0x11,0x8b,0x1d,0x8a,0x11,0x8b,0x1d,0x8b,0x11,0x8a,0x1d,0x8b,0x3a,0x11,0x8a,0x1d,0x8b,0x38,0x11,0x8b,0x1d,0x8a,0x11,0x30,0x8b,0x1d,0x8a,0x12,0x2e,0x8b,0x1e,0x8a,0x14,0x81,0x2c,0x89,0x31,0x81,0x20,0x8b,0x16,0x8a,0x22,0x8b,0x3a,0x16,0x8a,0x22,0x8b,0x3c,0x16,0x8b,0x22,0x8a,0x3d,0x16,0x8b,0x3f,0x22,0x8a,0x0f,0x8b,0x1b,0x8b,0x0f,0x2e,0x8a,0x1b,0x8b,0x2c,0x0f,0x8a,0x1b,0x8b,0x2b,0x0f,0x8a,0x1b,0x81,0x30,0x8a,0x14,0x8b,0x20,0x8a,0x3c,0x14,0x8b,0x20,0x8a,0x3d,0x14,0x8b,0x20,0x8b,0x3f,0x14,0x8a,0x38,0x20,0x8b,0x14,0x8a,0x20,0x8b,0x14,0x8b,0x20,0x8a,0x14,0x8b,0x20,0x8a,0x14,0x8b,0x20,0x8a,0x16,0x8b,0x22,0x8b,0x3a,0x16,0x8a,0x22,0x8b,0x3c,0x16,0x8a,0x22,0x8b,0x3d,0x16,0x8b,0x22,0x8a,0x3c,0x18,0x8b,0x24,0x8a,0x18,0x8b,0x24,0x8b,0x3a,0x18,0x8a,0x24,0x8b,0x18,0x8a,0x24,0x8b,0x38,0x11,0x8b,0x1d,0x8a,0x11,0x8b,0x1d,0x8a,0x11,0x8b,0x1d,0x8a,0x11,0x8b,0x1d,0x8b,0x18,0x8a,0x24,0x8b,0x16,0x8a,0x22,0x8b,0x14,0x8b,0x20,0x8a,0x13,0x8b,0x1e,0x8a,0x11,0x8b,0x1d,0x8b,0x11,0x8a,0x1d,0x8b,0x11,0x8a,0x1d,0x8b,0x11,0x8a,0x1d,0x8b,0x0f,0x8b,0x1b,0x8a,0x0f,0x8b,0x1b,0x8a,0x0f,0x8b,0x1b,0x8b,0x0f,0x8a,0x1b,0x8b,0x11,0x8a,0x44,0x1d,0x8b,0x11,0x43,0x8b,0x1d,0x41,0x8a,0x11,0x81,0x3c,0x89,0x3a,0x81,0x1d,0x8a,0x11,0x38,0x8b,0x1d,0x35,0x8a,0x11,0x8b,0x1d,0x8b,0x11,0x8a,0x1d,0x8b,0x11,0x8a,0x1d,0x8b,0x11,0x8b,0x1d,0x8a,0x16,0x22,0x8b,0x22,0x8a,0x16,0x8b,0x22,0x8b,0x16,0x8a,0x22,0x8b,0x16,0x8a,0x22,0x8a,0x27,0x81,0x0f,0x8a,0x1b,0x8b,0x0f,0x8b,0x1b,0x89,0x29,0x81,0x0f,0x8b,0x1b,0x8a,0x0f,0x8b,0x1b,0x89,0x24,0x82,0x14,0x8a,0x20,0x8b,0x14,0x8a,0x20,0x86,0x14,0x90,0x20,0x8a,0x14,0x8b,0x20,0x8a,0x18,0x30,0x8b,0x24,0x8b,0x18,0x8a,0x24,0x8b,0x18,0x8a,0x24,0x8b,0x18,0x8a,0x24,0x8b,0x11,0x8b,0x1d,0x8a,0x11,0x8b,0x1d,0x8a,0x11,0x8b,0x1d,0x8b,0x11,0x8a,0x1d,0x8b,0x0f,0x8a,0x1b,0x8b,0x0f,0x8b,0x1b,0x8a,0x0f,0x8b,0x1b,0x8a,0x0f,0x8b,0x1b,0x8a,0x11,0x8b,0x44,0x1d,0x8b,0x11,0x43,0x8a,0x1d,0x41,0x8b,0x11,0x3c,0x8a,0x3a,0x1d,0x8b,0x11,0x38,0x8b,0x1d,0x35,0x8a,0x11,0x8b,0x1d,0x8a,0x11,0x8b,0x1d,0x8b,0x11,0x8a,0x1d,0x8b,0x11,0x8a,0x1d,0x8b,0x16,0x8a,0x22,0x8b,0x16,0x8b,0x22,0x8a,0x16,0x8b,0x22,0x8a,0x16,0x8b,0x22,0x8b,0x18,0x8a,0x24,0x8b,0x18,0x8a,0x24,0x8b,0x18,0x8b,0x24,0x8a,0x18,0x8b,0x24,0x8a,0x11,0x8b,0x1d,0x8b,0x11,0x8a,0x1d,0x8b,0x11,0x8a,0x1d,0x8b,0x11,0x8a,0x1d,0x8b,0x11,0x8b,0x1d,0x8a,0x11,0x8b,0x1d,0x8a,0x11,0x8b,0x1d,0x8b,0x11,0x8a,0x1d,0x8b,0x16,0x8a,0x22,0x8b,0x16,0x8b,0x22,0x8a,0x16,0x8b,0x22,0x8a,0x16,0x8b,0x22,0x8a,0x0f,0x8b,0x1b,0x8b,0x0f,0x8a,0x1b,0x8b,0x0f,0x8a,0x1b,0x8b,0x0f,0x8b,0x1b,0x8a,0x14,0x38,0x8b,0x20,0x8a,0x14,0x8b,0x20,0x8b,0x14,0x8a,0x20,0x38,0x8b,0x14,0x3a,0x8a,0x20,0x3c,0x8b,0x11,0x8a,0x1d,0x8b,0x11,0x8b,0x1d,0x8a,0x11,0x8b,0x1d,0x8a,0x11,0x8b,0x1d,0x8b,0x16,0x8a,0x22,0x8b,0x16,0x8a,0x22,0x8b,0x16,0x8b,0x22,0x8a,0x16,0x8b,0x22,0x8a,0x0f,0x8b,0x1b,0x8b,0x0f,0x8a,0x1b,0x8b,0x0f,0x8a,0x1b,0x86,0x0f,0x8f,0x1b,0x8b,0x14,0x38,0x8b,0x20,0x44,0x8a,0x14,0x43,0x8b,0x20,0x3f,0x8a,0x14,0x41,0x8b,0x20,0x3c,0x8b,0x14,0x3f,0x8a,0x20,0x41,0x8b,0x11,0x8a,0x1d,0x8b,0x11,0x8b,0x1d,0x8a,0x11,0x8b,0x1d,0x8a,0x11,0x8b,0x1d,0x8a,0x16,0x8b,0x22,0x8b,0x16,0x8a,0x22,0x8b,0x16,0x8a,0x22,0x8b,0x16,0x8b,0x22,0x8a,0x0f,0x8b,0x1b,0x8a,0x0f,0x8b,0x1b,0x8b,0x0f,0x8a,0x1b,0x8b,0x0f,0x8a,0x1b,0x8b,0x14,0x38,0x8a,0x20,0x8b,0x14,0x8b,0x20,0x8a,0x14,0x8b,0x20,0x38,0x8a,0x14,0x3a,0x8b,0x20,0x3c,0x8b,0x11,0x8a,0x1d,0x8b,0x11,0x8a,0x1d,0x8b,0x11,0x8b,0x1d,0x8a,0x11,0x8b,0x1d,0x8a,0x16,0x8b,0x22,0x8a,0x16,0x8b,0x22,0x8b,0x16,0x8a,0x22,0x8b,0x16,0x8a,0x22,0x8b,0x18,0x8b,0x24,0x8a,0x18,0x8b,0x24,0x8a,0x18,0x8b,0x24,0x8b,0x18,0x8a,0x24,0x8b,0x19,0x8a,0x25,0x8b,0x19,0x8b,0x25,0x8a,0x19,0x8b,0x25,0x8a,0x19,0x8b,0x25,0x8a,0x18,0x3c,0x8b,0x24,0x8b,0x18,0x8a,0x24,0x8b,0x18,0x8a,0x24,0x8b,0x18,0x8b,0xff
};

//
// MUSIC DATA 5 -- "Stardust Speedway Past" by Naofumi Hayata - Round 5 - Japan
//
const byte music5[] = {
0x2d,0x88,0x2f,0x84,0x36,0x8d,0x32,0x8c,0x36,0x95,0x2d,0x84,0x36,0x2f,0x95,0x36,0x2d,0x84,0x36,0x2f,0x8d,0x2d,0x88,0x2f,0x84,0x36,0x8d,0x32,0x8c,0x36,0x95,0x2d,0x84,0x36,0x2f,0x95,0x36,0x2d,0x84,0x36,0x2f,0x8d,0x17,0x2d,0x88,0x2f,0x84,0x1a,0x36,0x8d,0x32,0x8c,0x36,0x89,0x1a,0x84,0x17,0x88,0x2d,0x84,0x12,0x36,0x2f,0x8d,0x15,0x88,0x36,0x2d,0x85,0x1a,0x36,0x2f,0x8c,0x1c,0x2d,0x88,0x2f,0x85,0x36,0x8c,0x32,0x8d,0x1a,0x36,0x94,0x1c,0x2d,0x85,0x17,0x36,0x2f,0x8c,0x1a,0x89,0x36,0x2d,0x84,0x14,0x36,0x2f,0x8c,0x17,0x8d,0x1a,0x36,0x99,0x36,0x88,0x1a,0x84,0x17,0x8d,0x12,0x36,0x8c,0x15,0x89,0x36,0x84,0x1a,0x36,0x8c,0x1c,0x8d,0x36,0x99,0x1a,0x36,0x95,0x1c,0x84,0x17,0x36,0x8d,0x1a,0x88,0x36,0x84,0x14,0x36,0x8d,0x17,0x8c,0x1a,0x36,0x99,0x36,0x88,0x1a,0x85,0x17,0x8c,0x12,0x36,0x8d,0x15,0x88,0x36,0x84,0x1a,0x36,0x8d,0x1c,0x8c,0x36,0x99,0x1a,0x36,0x95,0x1c,0x84,0x17,0x36,0x8d,0x1a,0x2d,0x84,0x2f,0x84,0x36,0x32,0x84,0x14,0x36,0x34,0x85,0x36,0x84,0x39,0x84,0x17,0x3b,0x88,0x3b,0x84,0x1a,0x36,0x8d,0x39,0x88,0x39,0x84,0x36,0x89,0x1a,0x84,0x17,0x36,0x88,0x36,0x85,0x12,0x36,0x8c,0x15,0x35,0x88,0x36,0x35,0x85,0x1a,0x36,0x8c,0x1c,0x34,0x88,0x32,0x85,0x36,0x34,0x88,0x32,0x84,0x32,0x8d,0x1a,0x36,0x95,0x1c,0x84,0x17,0x36,0x8c,0x1a,0x2d,0x89,0x36,0x2f,0x84,0x14,0x36,0x2d,0x88,0x2f,0x84,0x17,0x32,0x89,0x2f,0x84,0x1a,0x36,0x2d,0x88,0x2f,0x84,0x2f,0x8d,0x36,0x88,0x1a,0x84,0x17,0x8d,0x12,0x36,0x8c,0x15,0x89,0x36,0x84,0x1a,0x36,0x8c,0x1c,0x8d,0x36,0x99,0x1a,0x36,0x95,0x1c,0x84,0x17,0x36,0x8d,0x1a,0x88,0x36,0x84,0x14,0x36,0x8d,0x17,0x3b,0x88,0x3b,0x84,0x1a,0x36,0x8d,0x39,0x88,0x39,0x84,0x36,0x89,0x1a,0x84,0x17,0x36,0x88,0x36,0x84,0x12,0x36,0x8d,0x15,0x35,0x88,0x36,0x35,0x84,0x1a,0x36,0x8d,0x1c,0x34,0x88,0x32,0x84,0x36,0x34,0x89,0x32,0x84,0x32,0x8c,0x1a,0x36,0x95,0x1c,0x84,0x17,0x36,0x8d,0x1a,0x2d,0x88,0x36,0x2f,0x84,0x14,0x36,0x2d,0x89,0x2f,0x84,0x17,0x32,0x88,0x2f,0x84,0x1a,0x36,0x2d,0x89,0x2f,0x84,0x2f,0x8d,0x36,0x88,0x1a,0x84,0x17,0x8d,0x12,0x36,0x8c,0x15,0x88,0x36,0x85,0x1a,0x36,0x8c,0x1c,0x8d,0x36,0x99,0x1a,0x36,0x95,0x1c,0x84,0x17,0x36,0x8c,0x1a,0x89,0x36,0x84,0x14,0x36,0x8c,0x17,0x8d,0x1a,0x36,0x99,0x36,0x88,0x1a,0x84,0x17,0x8d,0x12,0x36,0x8c,0x15,0x89,0x36,0x84,0x1a,0x36,0x8c,0x1c,0x8d,0x36,0x99,0x1a,0x36,0x95,0x1c,0x84,0x17,0x36,0x8d,0x1a,0x88,0x36,0x84,0x14,0x36,0x8d,0x17,0x8c,0x1a,0x36,0x99,0x36,0x89,0x1a,0x84,0x17,0x8c,0x12,0x36,0x8d,0x15,0x88,0x36,0x84,0x1a,0x36,0x8d,0x1c,0x8c,0x36,0x99,0x1a,0x36,0x95,0x1c,0x84,0x17,0x36,0x8d,0x1a,0x88,0x36,0x84,0x14,0x36,0x8d,0x17,0x8d,0x1a,0x36,0x99,0x36,0x88,0x1a,0x84,0x17,0x8d,0x12,0x36,0x8c,0x15,0x88,0x36,0x85,0x1a,0x36,0x8c,0x1c,0x8d,0x36,0x99,0x1a,0x36,0x95,0x1c,0x84,0x17,0x36,0x8c,0x1a,0x89,0x36,0x84,0x14,0x36,0x8c,0x17,0x8d,0x1a,0x36,0x99,0x36,0x88,0x1a,0x84,0x17,0x8d,0x12,0x36,0x8c,0x15,0x89,0x36,0x84,0x1a,0x36,0x8d,0x1c,0x8c,0x36,0x99,0x1a,0x36,0x95,0x1c,0x84,0x17,0x36,0x8d,0x1a,0x88,0x36,0x84,0x14,0x36,0x8d,0x13,0x8c,0x13,0x8d,0x13,0x8c,0x13,0x8d,0x15,0x8c,0x15,0x8d,0x15,0x8c,0x15,0x8d,0x17,0x8c,0x17,0x8d,0x17,0x88,0x1e,0x84,0x17,0x8d,0x15,0x8c,0x15,0x8d,0x15,0x88,0x1c,0x85,0x15,0x88,0x15,0x84,0x13,0x8d,0x13,0x8c,0x13,0x8d,0x13,0x8c,0x15,0x8d,0x15,0x8c,0x15,0x8d,0x15,0x8c,0x17,0x8d,0x17,0x8c,0x17,0x89,0x1e,0x84,0x17,0x8c,0x15,0x8d,0x15,0x8c,0x15,0x89,0x1c,0x84,0x15,0x88,0x15,0x84,0x13,0x8d,0x13,0x8c,0x13,0x8d,0x13,0x8c,0x15,0x8d,0x15,0x8d,0x15,0x8c,0x15,0x8d,0x17,0x8c,0x17,0x8d,0x17,0x88,0x1e,0x84,0x17,0x8d,0x15,0x8c,0x15,0x8d,0x15,0x88,0x1c,0x84,0x15,0x89,0x15,0x84,0x13,0x8c,0x13,0x8d,0x13,0x8c,0x13,0x8d,0x15,0x8c,0x15,0x8d,0x15,0x8c,0x15,0x8d,0x17,0x8c,0x17,0x8d,0x17,0x88,0x1e,0x84,0x17,0x8d,0x15,0x8d,0x15,0x8c,0x15,0x88,0x1c,0x85,0x15,0x88,0x15,0x84,0x13,0x8d,0x13,0x8c,0x13,0x8d,0x13,0x8c,0x15,0x8d,0x15,0x8c,0x15,0x8d,0x15,0x8c,0x17,0x8d,0x17,0x8c,0x17,0x89,0x1e,0x84,0x17,0x8c,0x15,0x8d,0x15,0x8c,0x15,0x89,0x1c,0x84,0x15,0x88,0x15,0x84,0x13,0x8d,0x13,0x8c,0x13,0x8d,0x13,0x8d,0x15,0x8c,0x15,0x8d,0x15,0x8c,0x15,0x8d,0x17,0x8c,0x17,0x8d,0x17,0x88,0x1e,0x84,0x17,0x8d,0x15,0x8c,0x15,0x8d,0x15,0x88,0x1c,0x84,0x15,0x89,0x15,0x84,0x13,0x8c,0x13,0x8d,0x13,0x8c,0x13,0x8d,0x15,0x8c,0x15,0x8d,0x15,0x8c,0x15,0x8d,0x17,0x8c,0x17,0x8d,0x17,0x88,0x1e,0x85,0x17,0x8c,0x15,0x8d,0x15,0x8c,0x15,0x88,0x1c,0x85,0x15,0x88,0x15,0x84,0x13,0x8d,0x13,0x8c,0x13,0x8d,0x13,0x8c,0x15,0x8d,0x15,0x8c,0x15,0x8d,0x15,0x8c,0x17,0x8d,0x17,0x8c,0x17,0x89,0x1e,0x84,0x17,0x8c,0x17,0x89,0x17,0x84,0x19,0x8c,0x1a,0x8d,0x1c,0x88,0x1e,0x84,0x17,0x3b,0x89,0x3b,0x84,0x1a,0x36,0x8d,0x39,0x88,0x39,0x84,0x36,0x88,0x1a,0x85,0x17,0x36,0x88,0x36,0x84,0x12,0x36,0x8d,0x15,0x35,0x88,0x36,0x35,0x84,0x1a,0x36,0x8d,0x1c,0x34,0x88,0x32,0x84,0x36,0x34,0x89,0x32,0x84,0x32,0x8c,0x1a,0x36,0x95,0x1c,0x84,0x17,0x36,0x8d,0x1a,0x2d,0x88,0x36,0x2f,0x84,0x14,0x36,0x2d,0x89,0x2f,0x84,0x17,0x32,0x88,0x2f,0x84,0x1a,0x36,0x2d,0x89,0x2f,0x84,0x2f,0x8c,0x36,0x89,0x1a,0x84,0x17,0x8c,0x12,0x36,0x8d,0x15,0x88,0x36,0x84,0x1a,0x36,0xff  
};

//
// MUSIC DATA 6 -- "Around the World" by ATC - Round 6 - Winter
//
const byte music6[] = {
0x38,0x8a,0x31,0x8b,0x34,0x8b,0x38,0x8b,0x33,0x8b,0x2c,0x8b,0x2f,0x8b,0x33,0x8b,0x34,0x8b,0x2d,0x8b,0x31,0x8b,0x34,0x8a,0x36,0x8b,0x2f,0x8b,0x33,0x8b,0x36,0x8b,0x38,0x38,0x8b,0x31,0x8b,0x34,0x8b,0x38,0x8b,0x33,0x8b,0x2c,0x8b,0x2f,0x8a,0x33,0x8b,0x34,0x8b,0x2d,0x8b,0x31,0x8b,0x34,0x8b,0x36,0x8b,0x2f,0x8b,0x33,0x8b,0x36,0x8b,0x38,0x38,0x25,0x8b,0x31,0x8a,0x34,0x8b,0x38,0x8b,0x33,0x20,0x23,0x8b,0x2c,0x8b,0x2f,0x8b,0x33,0x8b,0x34,0x21,0x25,0x8b,0x2d,0x8b,0x31,0x8b,0x34,0x8b,0x36,0x23,0x27,0x8a,0x2f,0x8b,0x33,0x8b,0x36,0x8b,0x2c,0x38,0x38,0x8b,0x31,0x8b,0x34,0x8b,0x38,0x8b,0x33,0x20,0x23,0x8b,0x2c,0x8b,0x2f,0x8b,0x33,0x8a,0x34,0x21,0x25,0x8b,0x2d,0x8b,0x31,0x8b,0x34,0x8b,0x36,0x23,0x27,0x8b,0x2f,0x8b,0x33,0x8b,0x36,0x8b,0x2c,0x38,0x38,0x8b,0x31,0x8b,0x34,0x8a,0x38,0x8b,0x33,0x20,0x23,0x8b,0x2c,0x8b,0x2f,0x8b,0x33,0x8b,0x34,0x21,0x25,0x8b,0x2d,0x8b,0x31,0x8b,0x34,0x8b,0x36,0x23,0x27,0x8b,0x2f,0x8a,0x33,0x8b,0x36,0x8b,0x2c,0x38,0x38,0x8b,0x31,0x8b,0x34,0x8b,0x38,0x8b,0x33,0x20,0x23,0x8b,0x2c,0x8b,0x2f,0x8b,0x33,0x8b,0x34,0x21,0x25,0x8a,0x2d,0x8b,0x31,0x8b,0x34,0x8b,0x36,0x23,0x27,0x8b,0x2f,0x8b,0x33,0x8b,0x36,0x8b,0x2c,0x38,0x38,0x8b,0x31,0x8b,0x34,0x8b,0x38,0x8a,0x33,0x20,0x23,0x8b,0x2c,0x8b,0x2f,0x8b,0x33,0x8b,0x34,0x21,0x25,0x8b,0x2d,0x8b,0x31,0x8b,0x34,0x8b,0x36,0x23,0x27,0x8b,0x2f,0x8b,0x33,0x8a,0x36,0x8b,0x2c,0x38,0x38,0x8b,0x31,0x8b,0x34,0x8b,0x38,0x8b,0x33,0x20,0x23,0x8b,0x2c,0x8b,0x2f,0x8b,0x33,0x8b,0x34,0x21,0x25,0x8b,0x2d,0x8a,0x31,0x8b,0x34,0x8b,0x36,0x23,0x27,0x8b,0x2f,0x8b,0x33,0x8b,0x36,0x8b,0x2c,0x38,0x38,0x8b,0x31,0x8b,0x34,0x8b,0x38,0x8b,0x33,0x20,0x23,0x8a,0x2c,0x8b,0x2f,0x8b,0x33,0x8b,0x34,0x21,0x25,0x8b,0x2d,0x8b,0x31,0x8b,0x34,0x8b,0x36,0x23,0x27,0x8b,0x2f,0x8b,0x33,0x8b,0x36,0x8a,0x2c,0x38,0x38,0x8b,0x31,0x8b,0x34,0x8b,0x38,0x8b,0x33,0x20,0x23,0x8b,0x2c,0x8b,0x2f,0x8b,0x33,0x8b,0x34,0x21,0x25,0x8b,0x2d,0x8b,0x31,0x8a,0x34,0x8b,0x36,0x23,0x27,0x8b,0x2f,0x8b,0x33,0x8b,0x36,0x8b,0x2c,0x38,0x44,0x8b,0x3d,0x8b,0x40,0x8b,0x44,0x8b,0x3f,0x20,0x23,0x8b,0x38,0x8a,0x3b,0x8b,0x3f,0x8b,0x40,0x21,0x25,0x8b,0x39,0x8b,0x3d,0x8b,0x40,0x8b,0x42,0x23,0x27,0x8b,0x3b,0x8b,0x3f,0x8b,0x42,0x8a,0x2c,0x81,0x44,0x25,0x28,0x8a,0x3d,0x8b,0x40,0x8b,0x44,0x8b,0x3f,0x20,0x23,0x8b,0x38,0x8b,0x3b,0x8b,0x3f,0x8b,0x40,0x21,0x25,0x8b,0x39,0x8b,0x3d,0x8b,0x40,0x8a,0x42,0x23,0x27,0x8b,0x3b,0x8b,0x3f,0x8b,0x42,0x8b,0x2c,0x44,0x25,0x8b,0x3d,0x8b,0x40,0x8b,0x44,0x8b,0x3f,0x20,0x23,0x8b,0x38,0x8b,0x3b,0x8a,0x3f,0x8b,0x40,0x21,0x25,0x8b,0x39,0x8b,0x3d,0x8b,0x40,0x8b,0x42,0x23,0x27,0x8b,0x3b,0x8b,0x3f,0x8b,0x42,0x8a,0x2c,0x81,0x44,0x25,0x28,0x8b,0x3d,0x8a,0x40,0x8b,0x44,0x8b,0x3f,0x20,0x23,0x8b,0x38,0x8b,0x3b,0x8b,0x3f,0x8b,0x40,0x21,0x25,0x8b,0x39,0x8b,0x3d,0x8b,0x40,0x8b,0x42,0x23,0x27,0x8a,0x3b,0x8b,0x3f,0x8b,0x42,0x8b,0x2c,0x38,0x25,0x8b,0x31,0x8b,0x34,0x8b,0x38,0x8b,0x33,0x20,0x23,0x8b,0x2c,0x8b,0x2f,0x8b,0x33,0x8a,0x34,0x21,0x25,0x8b,0x2d,0x8b,0x31,0x8b,0x34,0x8b,0x36,0x23,0x27,0x8b,0x2f,0x8b,0x33,0x8b,0x36,0x8b,0x2c,0x38,0x38,0x8b,0x31,0x8b,0x34,0x8a,0x38,0x8b,0x33,0x20,0x23,0x8b,0x2c,0x8b,0x2f,0x8b,0x33,0x8b,0x34,0x21,0x25,0x8b,0x2d,0x8b,0x31,0x8b,0x34,0x8b,0x36,0x23,0x27,0x8b,0x2f,0x8a,0x33,0x8b,0x36,0x8b,0x2c,0x38,0x38,0x8b,0x31,0x8b,0x34,0x8b,0x38,0x8b,0x33,0x20,0x23,0x8b,0x2c,0x8b,0x2f,0x8b,0x33,0x8b,0x34,0x21,0x25,0x8a,0x2d,0x8b,0x31,0x8b,0x34,0x8b,0x36,0x23,0x27,0x8b,0x2f,0x8b,0x33,0x8b,0x36,0x8b,0x2c,0x38,0x38,0x8b,0x31,0x8b,0x34,0x8b,0x38,0x8a,0x33,0x20,0x23,0x8b,0x2c,0x8b,0x2f,0x8b,0x33,0x8b,0x34,0x21,0x25,0x8b,0x2d,0x8b,0x31,0x8b,0x34,0x8b,0x36,0x23,0x27,0x8b,0x2f,0x8b,0x33,0x8a,0x36,0x8b,0x2c,0x25,0x28,0xff  
};

//
// MUSIC DATA 7 -- "Sweet Dreams" by Eurythmics - Round 7 - Mars
//
const byte music7[] = {
0x0d,0x0d,0x8c,0x0d,0x8c,0x25,0x19,0x8b,0x25,0x19,0x8c,0x1c,0x8b,0x28,0x8c,0x19,0x8c,0x25,0x8b,0x15,0x8c,0x21,0x8c,0x21,0x8b,0x25,0x8c,0x14,0x8b,0x14,0x8c,0x20,0x8c,0x25,0x8b,0x0d,0x0d,0x8c,0x0d,0x8b,0x25,0x19,0x8c,0x25,0x19,0x8c,0x1c,0x8b,0x28,0x8c,0x19,0x8c,0x25,0x8b,0x15,0x8c,0x21,0x8b,0x21,0x8c,0x25,0x8c,0x14,0x8b,0x14,0x8c,0x20,0x8b,0x25,0x8c,0x0d,0x0d,0x8c,0x0d,0x8b,0x25,0x19,0x34,0x8c,0x25,0x19,0x8b,0x1c,0x81,0x34,0x8b,0x28,0x8b,0x31,0x81,0x19,0x8b,0x34,0x25,0x8c,0x15,0x8b,0x34,0x81,0x21,0x8b,0x21,0x8c,0x25,0x34,0x8b,0x14,0x81,0x33,0x8b,0x14,0x8c,0x20,0x8b,0x25,0x8c,0x34,0x0d,0x0d,0x8b,0x34,0x0d,0x8c,0x31,0x25,0x19,0x8c,0x25,0x19,0x34,0x8b,0x1c,0x8c,0x28,0x8b,0x31,0x81,0x19,0x8b,0x25,0x8b,0x34,0x81,0x15,0x8b,0x36,0x21,0x8c,0x21,0x8c,0x25,0x8b,0x34,0x14,0x8c,0x33,0x14,0x8b,0x20,0x8c,0x25,0x31,0x8b,0x34,0x81,0x0d,0x0d,0x8b,0x0d,0x34,0x8c,0x31,0x25,0x19,0x8b,0x25,0x19,0x81,0x34,0x8b,0x1c,0x8c,0x28,0x8b,0x19,0x81,0x34,0x8b,0x25,0x31,0x8c,0x15,0x34,0x8b,0x36,0x21,0x8c,0x21,0x8b,0x25,0x8c,0x34,0x14,0x8b,0x33,0x81,0x14,0x8b,0x20,0x8c,0x25,0x8b,0x0d,0x0d,0x34,0x8c,0x0d,0x8b,0x31,0x81,0x25,0x19,0x8b,0x34,0x25,0x19,0x8c,0x1c,0x8c,0x28,0x31,0x8b,0x19,0x8c,0x25,0x8b,0x34,0x15,0x8c,0x34,0x21,0x8b,0x36,0x81,0x21,0x8b,0x25,0x34,0x8c,0x14,0x8b,0x14,0x81,0x33,0x8b,0x20,0x8c,0x25,0x8b,0x0d,0x34,0x2c,0x8c,0x0d,0x34,0x2c,0x8b,0x28,0x19,0x81,0x31,0x8b,0x19,0x8c,0x1c,0x34,0x2c,0x97,0x31,0x28,0x19,0x8c,0x34,0x2c,0x8b,0x15,0x8c,0x21,0x2a,0x31,0xa3,0x14,0x8b,0x14,0xa3,0x2c,0x0d,0x34,0x8b,0x34,0x2c,0x81,0x0d,0x8b,0x31,0x28,0x19,0x8c,0x19,0x8c,0x1c,0x2c,0x34,0x8b,0x2c,0x34,0x8b,0x31,0x28,0x81,0x19,0x8b,0x2c,0x34,0x8c,0x15,0x8c,0x21,0x2d,0x36,0xa2,0x34,0x2c,0x14,0x8c,0x33,0x14,0x2a,0xa3,0x0d,0x34,0x2c,0x8b,0x34,0x0d,0x81,0x2c,0x8b,0x31,0x28,0x19,0x8c,0x19,0x8b,0x1c,0x81,0x34,0x2c,0x8b,0x2c,0x34,0x8b,0x28,0x31,0x81,0x19,0x8b,0x2c,0x34,0x8c,0x15,0x8b,0x2a,0x21,0x81,0x31,0xa2,0x14,0x8c,0x14,0xa2,0x34,0x2c,0x81,0x0d,0x8b,0x34,0x2c,0x0d,0x8c,0x28,0x19,0x31,0x8c,0x19,0x8b,0x1c,0x34,0x2c,0x97,0x31,0x28,0x19,0x8c,0x34,0x2c,0x8c,0x15,0x8b,0x21,0x2d,0x36,0xa3,0x34,0x2c,0x14,0x8b,0x33,0x2a,0x81,0x14,0xff  
};

//
// MUSIC DATA 8 -- "I Will Survive" by Gloria Gaynor - Round 8 - Darkness World
//
const byte music8[] = {
0x3d,0x0d,0x38,0x8d,0x38,0x8c,0x0d,0x38,0x87,0x0d,0x38,0x8c,0x0d,0x38,0x86,0x38,0x8d,0x3d,0x38,0x86,0x0d,0x38,0x86,0x3d,0x10,0x38,0x86,0x38,0x86,0x3d,0x11,0x87,0x3d,0x38,0x86,0x12,0x39,0x8c,0x39,0x8d,0x12,0x39,0x86,0x12,0x39,0x8c,0x12,0x39,0x86,0x39,0x8d,0x3d,0x39,0x86,0x12,0x39,0x86,0x3d,0x15,0x39,0x86,0x39,0x87,0x3d,0x16,0x86,0x39,0x86,0x3d,0x17,0x36,0x8d,0x3b,0x0b,0x36,0x8c,0x3b,0x0b,0x36,0x86,0x36,0x86,0x3b,0x0b,0x87,0x3d,0x36,0x86,0x0b,0x36,0x8c,0x3b,0x36,0x86,0x0b,0x36,0x87,0x3b,0x0d,0x36,0x86,0x36,0x86,0x3b,0x0f,0x86,0x3d,0x36,0x86,0x10,0x38,0x38,0x8d,0x3b,0x38,0x8c,0x3b,0x10,0x38,0x86,0x10,0x38,0x87,0x3b,0x86,0x10,0x38,0x86,0x3d,0x38,0x8c,0x38,0x38,0x87,0x38,0x86,0x38,0x12,0x38,0x86,0x38,0x38,0x86,0x38,0x14,0x87,0x38,0x86,0x15,0x34,0x8c,0x38,0x09,0x34,0x8d,0x38,0x34,0x86,0x38,0x09,0x34,0x86,0x38,0x86,0x34,0x86,0x09,0x34,0x8d,0x38,0x34,0x86,0x34,0x86,0x38,0x0d,0x34,0x86,0x38,0x34,0x87,0x38,0x0e,0x86,0x34,0x86,0x0f,0x36,0x8c,0x36,0x1b,0x36,0x8d,0x36,0x0f,0x36,0x86,0x0f,0x36,0x86,0x36,0x86,0x36,0x87,0x38,0x0f,0x36,0x8c,0x36,0x86,0x0f,0x36,0x87,0x38,0x12,0x36,0x86,0x36,0x86,0x38,0x13,0x86,0x38,0x36,0x86,0x14,0x33,0x8d,0x38,0x08,0x33,0x8c,0x38,0x08,0x33,0x86,0x33,0x87,0x38,0x86,0x08,0x33,0x86,0x38,0x33,0x86,0x38,0x86,0x08,0x33,0x87,0x33,0x86,0x38,0x12,0x33,0x86,0x33,0x86,0x38,0x13,0x86,0x33,0x87,0x3c,0x14,0x30,0x8c,0x3c,0x08,0x30,0x8c,0x3c,0x08,0x30,0x87,0x30,0x86,0x3d,0x86,0x3f,0x08,0x30,0x86,0x0a,0x30,0x8d,0x3c,0x30,0x86,0x30,0x86,0x3c,0x0c,0x30,0x86,0x30,0x87,0x3c,0x86,0x30,0x86,0x3d,0x0d,0x38,0x8c,0x38,0x8d,0x0d,0x38,0x86,0x0d,0x38,0x8c,0x0d,0x38,0x87,0x38,0x8c,0x3d,0x38,0x86,0x0d,0x38,0x86,0x3f,0x10,0x38,0x87,0x38,0x86,0x40,0x11,0x86,0x3f,0x38,0x86,0x12,0x39,0x8d,0x3d,0x39,0x8c,0x12,0x39,0x86,0x12,0x39,0x8d,0x12,0x39,0x86,0x39,0x8c,0x3d,0x39,0x87,0x12,0x39,0x86,0x3d,0x15,0x39,0x86,0x39,0x86,0x3d,0x16,0x86,0x3d,0x39,0x87,0x17,0x36,0x8c,0x3b,0x0b,0x36,0x8c,0x3b,0x0b,0x36,0x87,0x36,0x86,0x3b,0x0b,0x86,0x3d,0x36,0x86,0x0b,0x36,0x8d,0x3b,0x36,0x86,0x0b,0x36,0x86,0x3b,0x0d,0x36,0x86,0x36,0x86,0x39,0x0f,0x87,0x39,0x36,0x86,0x10,0x38,0x38,0x8c,0x38,0x38,0x8d,0x38,0x10,0x38,0x86,0x10,0x38,0x8c,0x10,0x38,0x87,0x38,0x8c,0x38,0x38,0x86,0x38,0x86,0x38,0x12,0x38,0x87,0x38,0x86,0x38,0x14,0x86,0x38,0x38,0x86,0x15,0x34,0x8d,0x38,0x09,0x34,0x8c,0x38,0x34,0x86,0x38,0x09,0x34,0x86,0x38,0x87,0x34,0x86,0x09,0x34,0x8c,0x38,0x34,0x86,0x34,0x87,0x38,0x0d,0x34,0x86,0x38,0x34,0x86,0x38,0x0e,0x86,0x34,0x87,0x0f,0x36,0x8c,0x36,0x1b,0x36,0x8c,0x36,0x0f,0x36,0x87,0x0f,0x36,0x86,0x36,0x86,0x36,0x86,0x38,0x0f,0x36,0x8d,0x36,0x86,0x0f,0x36,0x86,0x38,0x12,0x36,0x86,0x38,0x36,0x86,0x13,0x87,0x38,0x36,0x86,0x14,0x33,0x8c,0x08,0x33,0x8d,0x08,0x33,0x86,0x33,0x8c,0x08,0x33,0x86,0x33,0x8d,0x08,0x33,0x86,0x33,0x86,0x12,0x33,0x87,0x33,0x86,0x38,0x13,0x86,0x33,0x86,0x3c,0x14,0x30,0x8d,0x3c,0x08,0x30,0x8c,0x3c,0x08,0x30,0x86,0x30,0x86,0x3d,0x87,0x3f,0x08,0x30,0x86,0x0a,0x30,0x8c,0x3c,0x30,0x86,0x30,0x87,0x3c,0x0c,0x30,0x86,0x30,0x86,0x3c,0x86,0x30,0xff
};

//
// MUSIC DATA 9 -- "Mortal Kombat" Movie Theme - Boss Fight
//
const byte music9[] = {
0x31,0x85,0x25,0x85,0x31,0x85,0x25,0x85,0x34,0x85,0x28,0x85,0x31,0x86,0x25,0x85,0x36,0x85,0x2a,0x85,0x31,0x85,0x25,0x85,0x38,0x85,0x2c,0x86,0x36,0x85,0x2a,0x85,0x34,0x85,0x28,0x85,0x34,0x85,0x28,0x85,0x38,0x86,0x2c,0x85,0x34,0x85,0x28,0x85,0x3b,0x85,0x2f,0x85,0x34,0x85,0x28,0x86,0x39,0x85,0x2d,0x85,0x38,0x85,0x2c,0x85,0x2f,0x85,0x23,0x85,0x2f,0x86,0x23,0x85,0x33,0x85,0x27,0x85,0x2f,0x85,0x23,0x85,0x34,0x85,0x28,0x86,0x2f,0x85,0x23,0x85,0x36,0x85,0x2a,0x85,0x34,0x85,0x28,0x85,0x2d,0x86,0x21,0x85,0x2d,0x85,0x21,0x85,0x31,0x85,0x25,0x85,0x2d,0x85,0x21,0x86,0x34,0x85,0x28,0x85,0x2d,0x85,0x21,0x85,0x34,0x85,0x28,0x85,0x33,0x86,0x27,0x85,0x31,0x31,0x19,0x8f,0x31,0x31,0x19,0x90,0x31,0x31,0x19,0x94,0x31,0x31,0x19,0x8a,0x2f,0x2f,0x17,0x8b,0x34,0x34,0x1c,0x8a,0x31,0x31,0x19,0x8f,0x31,0x31,0x19,0x90,0x31,0x31,0x19,0x94,0x31,0x31,0x19,0x8b,0x2f,0x2f,0x17,0x8a,0x2c,0x2c,0x14,0x8a,0x31,0x31,0x19,0x90,0x31,0x31,0x19,0x8f,0x31,0x31,0x19,0x95,0x31,0x31,0x19,0x8a,0x2f,0x2f,0x17,0x8a,0x34,0x34,0x1c,0x8b,0x31,0x31,0x19,0x8f,0x31,0x31,0x19,0x8f,0x31,0x31,0x19,0x90,0x31,0x31,0x19,0x8a,0x31,0x31,0x19,0x85,0x2f,0x2f,0x17,0x8b,0x2c,0x2c,0x14,0x8a,0x25,0x31,0x19,0x85,0x2c,0x38,0x8a,0x25,0x31,0x19,0x85,0x28,0x34,0x19,0x8b,0x25,0x31,0x19,0x85,0x26,0x32,0x19,0x8a,0x25,0x31,0x85,0x28,0x34,0x85,0x19,0x86,0x25,0x31,0x19,0x85,0x26,0x32,0x85,0x23,0x2f,0x17,0x85,0x14,0x85,0x25,0x31,0x19,0x85,0x2c,0x38,0x19,0x8b,0x25,0x31,0x19,0x85,0x28,0x34,0x8a,0x25,0x31,0x19,0x85,0x26,0x32,0x19,0x85,0x19,0x85,0x25,0x31,0x19,0x86,0x28,0x34,0x85,0x19,0x85,0x25,0x31,0x17,0x85,0x26,0x32,0x85,0x23,0x2f,0x14,0x8a,0x25,0x31,0x19,0x86,0x2c,0x38,0x19,0x8a,0x25,0x31,0x19,0x85,0x28,0x34,0x19,0x8a,0x25,0x31,0x19,0x85,0x26,0x32,0x19,0x86,0x19,0x85,0x25,0x31,0x19,0x85,0x28,0x34,0x85,0x19,0x85,0x25,0x31,0x19,0x85,0x26,0x32,0x85,0x23,0x2f,0x17,0x86,0x14,0x85,0x25,0x31,0x19,0x85,0x2c,0x38,0x19,0x8a,0x25,0x31,0x19,0x85,0x28,0x34,0x8b,0x25,0x31,0x19,0x85,0x23,0x2f,0x19,0x85,0x19,0x85,0x23,0x2f,0x19,0x85,0x25,0x31,0x85,0x19,0x85,0x25,0x31,0x17,0x8b,0x14,0x8a,0x31,0x3d,0x19,0x85,0x38,0x44,0x19,0x8a,0x31,0x3d,0x19,0x86,0x34,0x40,0x19,0x8a,0x31,0x3d,0x19,0x85,0x32,0x3e,0x19,0x85,0x19,0x85,0x31,0x3d,0x19,0x85,0x34,0x40,0x86,0x19,0x85,0x31,0x3d,0x19,0x85,0x32,0x3e,0x85,0x17,0x2f,0x3b,0x85,0x14,0x85,0x31,0x3d,0x19,0x85,0x38,0x44,0x19,0x8b,0x31,0x3d,0x19,0x85,0x34,0x40,0x8a,0x31,0x3d,0x19,0x85,0x32,0x3e,0x19,0x85,0x19,0x86,0x31,0x3d,0x19,0x85,0x34,0x40,0x85,0x19,0x85,0x31,0x3d,0x17,0x85,0x32,0x3e,0x85,0x2f,0x3b,0x14,0x8b,0x31,0x3d,0x19,0x85,0x38,0x44,0x8a,0x31,0x3d,0x19,0x85,0x34,0x40,0x19,0x8a,0x31,0x3d,0x19,0x86,0x32,0x3e,0x19,0x8a,0x31,0x3d,0x85,0x34,0x40,0x85,0x19,0x85,0x31,0x3d,0x19,0x85,0x32,0x3e,0x86,0x2f,0x3b,0x17,0x85,0x14,0x85,0x31,0x3d,0x19,0x85,0x38,0x44,0x19,0x8a,0x31,0x3d,0x19,0x85,0x34,0x40,0x19,0x8b,0x31,0x3d,0x19,0x85,0x2f,0x3b,0x8a,0x2f,0x3b,0x85,0x31,0x3d,0x85,0x19,0x86,0x31,0x3d,0x17,0x8a,0x14,0xff  
};

//
// MUSIC DATA 10 -- "Final Boss" by Yuko Takehara - Final Boss Fight
//
const byte music10[] = {
0x31,0x19,0x8a,0x19,0x93,0x19,0x93,0x14,0x8a,0x17,0x8a,0x14,0x89,0x33,0x19,0x8a,0x19,0x93,0x34,0x19,0x93,0x14,0x8a,0x36,0x17,0x89,0x14,0x8a,0x34,0x17,0x88,0x17,0x90,0x33,0x17,0x90,0x12,0x88,0x15,0x88,0x12,0x88,0x17,0x88,0x17,0x88,0x33,0x88,0x17,0x88,0x34,0x88,0x12,0x88,0x36,0x15,0x88,0x12,0x88,0x38,0x15,0x88,0x15,0x90,0x36,0x15,0x84,0x34,0x84,0x38,0x88,0x12,0x88,0x15,0x88,0x12,0x88,0x36,0x17,0x88,0x36,0x17,0x90,0x34,0x17,0x90,0x34,0x14,0x88,0x33,0x17,0x88,0x14,0x88,0x31,0x19,0x88,0x34,0x19,0x88,0x3d,0x88,0x31,0x19,0x88,0x3b,0x88,0x3d,0x14,0x88,0x34,0x17,0x88,0x3d,0x14,0x88,0x19,0x88,0x3b,0x19,0x88,0x3d,0x88,0x31,0x19,0x88,0x34,0x88,0x3d,0x14,0x88,0x3b,0x17,0x88,0x3d,0x14,0x88,0x31,0x17,0x88,0x34,0x17,0x88,0x3d,0x88,0x31,0x17,0x88,0x3b,0x88,0x3d,0x12,0x88,0x34,0x15,0x88,0x3d,0x12,0x88,0x17,0x88,0x3b,0x17,0x88,0x3d,0x88,0x31,0x17,0x88,0x34,0x88,0x3d,0x12,0x88,0x3b,0x15,0x88,0x3d,0x12,0x88,0x31,0x15,0x88,0x36,0x15,0x88,0x3d,0x88,0x31,0x15,0x88,0x39,0x88,0x3d,0x12,0x88,0x36,0x15,0x88,0x3d,0x12,0x88,0x15,0x88,0x39,0x15,0x88,0x3d,0x88,0x31,0x15,0x88,0x36,0x88,0x3d,0x12,0x88,0x39,0x15,0x88,0x3d,0x12,0x88,0x31,0x12,0x88,0x36,0x12,0x88,0x3d,0x88,0x31,0x12,0x88,0x39,0x88,0x3d,0x12,0x88,0x36,0x19,0x88,0x3d,0x12,0x88,0x39,0x15,0x88,0x39,0x15,0x90,0x38,0x14,0xa8,0x19,0xff
};

//
// MUSIC DATA 11 -- "Yesterday" by The Beatles - Ending
//
const byte music11[] = {
0x2e,0x30,0x20,0x8f,0x2c,0x87,0x2c,0xbf,0x82,0x30,0x20,0x9d,0x30,0x1f,0x9c,0x30,0x8f,0x32,0x8e,0x34,0x2b,0x18,0x8f,0x35,0x87,0x37,0x96,0x38,0x8e,0x37,0x2c,0x1d,0x8e,0x35,0x88,0x35,0xbf,0x81,0x1b,0x9d,0x2c,0x19,0x9d,0x35,0x87,0x35,0x8f,0x33,0x87,0x2b,0x1b,0x87,0x31,0x96,0x30,0x8e,0x2e,0x8e,0x31,0x25,0x14,0x9d,0x30,0x24,0x8f,0x30,0x24,0xab,0x13,0x8e,0x2e,0x27,0x8f,0x2c,0x27,0x11,0x9c,0x30,0x2c,0x8f,0x2e,0x27,0x8e,0x26,0x16,0x9d,0x2c,0x8e,0x29,0x26,0x8f,0x2c,0x25,0x19,0x9c,0x30,0x2c,0x24,0x8f,0x30,0x2c,0x24,0xff  
};

//
// MUSIC DATA 12 -- "The Final Countdown" by Europe - Game Over (Jingle)
//
const byte music12[] = {
0x31,0x31,0x31,0x8c,0x31,0x31,0x31,0x91,0x33,0x33,0x33,0x93,0x34,0x34,0x34,0x8d,0x33,0x33,0x33,0x98,0x31,0x31,0x31,0xff
};