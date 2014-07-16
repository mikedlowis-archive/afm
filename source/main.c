#include <ncurses.h>
#include <stdbool.h>

bool Running = true;

void handle_input(char ch) {
    if(ch == 'q')
        Running = false;
}

int main(int argc, char** argv) {
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    printw("Hello, World! Please Press 'q' to quit.");
    while(Running) {
        refresh();
        handle_input(getch());
    }
    endwin();
    return 0;
}
