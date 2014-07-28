#include <stdint.h>
#include <ncurses.h>

#include "aardvark.h"

typedef struct {
    uint8_t a, b, c;
} Triplet_T;

static Triplet_T aadata[77] = {
    {  0, 34, 14 }, {  1, 30, 22 }, {  2, 17,  2 }, {  2, 28, 26 },
    {  3,  3,  2 }, {  3, 16,  4 }, {  3, 26, 30 }, {  4,  3,  3 }, {  4, 15,  5 }, {  4, 24, 33 },
    {  5,  4,  3 }, {  5, 15,  5 }, {  5, 22, 37 }, {  6,  4,  5 }, {  6, 15, 45 },
    {  7,  5,  5 }, {  7, 14, 47 }, {  8,  6,  5 }, {  8, 14, 48 }, {  9,  7,  6 }, {  9, 14, 48 },
    { 10,  9, 54 }, { 11,  9, 54 }, { 12,  8, 56 }, { 13,  7,  5 }, { 13, 14, 50 },
    { 14,  6, 58 }, { 15,  6, 59 }, { 16,  5, 60 }, { 17,  4, 12 }, { 17, 19, 39 }, { 17, 59,  6 },
    { 18,  3, 10 }, { 18, 20, 17 }, { 18, 39,  9 }, { 18, 49,  7 }, { 18, 60,  5 },
    { 19,  1, 10 }, { 19, 20, 17 }, { 19, 40,  8 }, { 19, 50,  6 }, { 19, 61,  5 },
    { 20,  0,  8 }, { 20, 21, 15 }, { 20, 42,  6 }, { 20, 50,  6 }, { 20, 62,  4 },
    { 21,  0,  7 }, { 21, 22,  5 }, { 21, 29,  7 }, { 21, 42,  5 }, { 21, 50,  5 }, { 21, 63,  4 },
    { 22,  2,  3 }, { 22, 22,  5 }, { 22, 30,  6 }, { 22, 41,  5 }, { 22, 50,  6 }, { 22, 64,  3 },
    { 23, 23,  5 }, { 23, 30,  7 }, { 23, 40,  5 }, { 23, 50,  6 }, { 23, 65,  4 },
    { 24, 23,  5 }, { 24, 31,  6 }, { 24, 38,  7 }, { 24, 50,  6 }, { 24, 66,  4 },
    { 25, 21,  6 }, { 25, 30, 14 }, { 25, 49,  7 }, { 25, 68,  2 },
    { 26, 18,  8 }, { 26, 27, 15 }, { 26, 49,  7 }, { 27, 26,  6 }
};

void aardvark_draw(void) {
    int row, col;
    int i;
    int trips=sizeof(aadata)/sizeof(Triplet_T);
    char* aardvark="############################################################";
    getmaxyx(stdscr, row, col);
    for(i=0; i<trips; i++)
        mvaddnstr(((row-28)/2)+aadata[i].a, (col-70)/2+aadata[i].b, aardvark, aadata[i].c);
    refresh();
}

