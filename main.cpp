#include <iostream>
#include <vector>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#include <dialog.h>     

#define HEAD_ICON 'O'
#define TAIL_ICON '#'
#define ITEM_ICON '?'
#define BORDER_TOP_ICON '-'
#define BORDER_BOTTOM_ICON '-'
#define BORDER_LEFT_ICON '|'
#define BORDER_RIGHT_ICON '|'
#define BOARD_HORIZONTAL_LENGTH 40
#define BOARD_VERTICAL_LENGTH 20

enum Direction {
    UP,
    DOWN,
    RIGHT,
    LEFT
};

typedef struct coordinates_t {
    int x;
    int y;
} coordinates;

class GameBoard {
public:
    int vertical_length;
    int horizontal_length;
    coordinates item; 

    GameBoard(int ver, int hor);
    void draw_board();
    void generate_item();
};

class Snake {
public:
    int x, y; // coordinates of snakes head
    Direction direction;
    std::vector<coordinates> tail;

    Snake(int x, int y, Direction dir);
    void update();
    void set_direction(int input, GameBoard *board);
    void draw();
    void add_to_tail();
    bool check_collision(GameBoard *board);
    bool check_backwards_collision(Direction dir);
    void destroy();
};

void Snake::destroy() {
    tail.clear();
    x = y = 5;
    direction = RIGHT;
}

bool Snake::check_collision(GameBoard *board) {
    // loop the tail of snake and check if snakes head hits it
    for (auto item : tail) {
        if (item.x == x && item.y == y) {
            return true;
        }
    }

    //check collision with the borders
    if (x == board->horizontal_length || y == board->vertical_length ||
        y == 0 || x == 0) {
        return true;
    }

    // is snakes head hits the item, add length to snake and generate a new item
    if (x == board->item.x && y == board->item.y) {
        add_to_tail();
        board->generate_item();
    }

    return false;
}

Snake::Snake(int x_par, int y_par, Direction dir) {
    x = x_par;
    y = y_par;
    direction = dir;
}

GameBoard::GameBoard(int ver, int hor) {
    vertical_length = ver;
    horizontal_length = hor;

    item.x = rand() % horizontal_length + 2;
    item.y = rand() % vertical_length + 2;
}

void GameBoard::draw_board() {
    // draw borders
    for (int index=0; index<=horizontal_length; index++) {
        mvaddch(0, index, BORDER_TOP_ICON);
        mvaddch(vertical_length, index, BORDER_BOTTOM_ICON);
    }
    for (int index=0; index<=vertical_length; index++) {
        mvaddch(index, 0, BORDER_LEFT_ICON);
        mvaddch(index, horizontal_length, BORDER_RIGHT_ICON);

    }
    // draw the collectable item
    mvaddch(item.y, item.x, ITEM_ICON);
}

void GameBoard::generate_item() {
    srand(time(NULL));

    item.x = rand() % (horizontal_length-2) + 2;
    item.y = rand() % (vertical_length-2) + 2;
}

void Snake::add_to_tail() {
    // based on direction, add length to the snake
    switch (direction) {
        case UP:
            tail.push_back({x, y+1});
            break;
        case DOWN:
            tail.push_back({x, y-1});
            break;
        case RIGHT:
            tail.push_back({x+1, y});
            break;
        case LEFT:
            tail.push_back({x-1, y});
            break;
    }
}

void Snake::set_direction(int input, GameBoard *board) {
    // set the direction based on input from the keypad
    // before setting the direction, confirm that the snake wouldn't go inside of itself
    switch (input) {
    case KEY_LEFT: 
        if (check_backwards_collision(LEFT) != true)  { direction=LEFT; }
        break;
     case KEY_DOWN:
        if (check_backwards_collision(DOWN) != true)  { direction=DOWN; }
        break;   
    case KEY_UP:
        if (check_backwards_collision(UP) != true)  { direction=UP; }
        break;
    case KEY_RIGHT:
        if (check_backwards_collision(RIGHT) != true)  { direction=RIGHT; }
        break;
    default:
        break;
    }
}

void Snake::draw() {
    mvaddch(y, x, HEAD_ICON);

    for (auto i : tail) {
        mvaddch(i.y, i.x, TAIL_ICON);
    }

    mvprintw(3, 10+BOARD_HORIZONTAL_LENGTH, "Length: %d", tail.size());
}

bool Snake::check_backwards_collision(Direction dir) {
    // check if given direction would go backwards, inside the snake
    if (tail.size() == 0) { return false; }
    switch (dir) {
        case DOWN:
            if (y+1==tail[0].y && x==tail[0].x) { return true;}
            break;
        case UP:
            if (y-1==tail[0].y && x==tail[0].x) { return true;}
            break;
        case RIGHT:
            if (y==tail[0].y && x+1==tail[0].x) { return true;}
            break;
        case LEFT:
            if (y==tail[0].y && x-1==tail[0].x) { return true;}
            break;
    }
    return false;
}

void Snake::update() {
    // start looping the snakes tail from the tail, shifting each value
    for (int index = tail.size(); index > 0; index--) {
          tail[index] = tail[index-1];
    }
    
    // if snake has a tail, move the heads coordinates to first item on the tail vector
    if (tail.size() > 0) {
        tail.at(0).x = x;
        tail.at(0).y = y;
    }
    
    switch (direction) {
        case DOWN:
            y+=1;
            break;
        case UP:
            y-=1;
            break;
        case RIGHT:
            x+=1;
            break;
        case LEFT:
            x-=1;
            break;
    }
}

void mainloop(Snake snake, GameBoard board)
{
    while (true) {

        int input = getch();
        if (input != 'q') {
            snake.set_direction(input, &board);
        } else {
            return;
        }

        usleep(100000/2);
        input = getch();
        if (input != 'q') {
            snake.set_direction(input, &board);
        } else {
            return;
        }


        usleep(100000/2);

        // read input and set direction 
        input = getch();
        if (input != 'q') {
            snake.set_direction(input, &board);
        } else {
            return;
        }

        // update snake's coordinates and end the game if
        // the snake hits a wall or itself
        snake.update();
        if (snake.check_collision(&board)) {
            nodelay(stdscr, false);
            mvprintw(5, 5, "Press 'q' to quit or 'r' to restart") ;

            while (true)  {
                if ((input = getch()) == 'q')
                    return;
                else if (input == 'r') {
                    nodelay(stdscr, true);
                    snake.destroy();
                    break;
                }

            }

        }
        input = getch();

        // redraw everything
        clear();
        board.draw_board();
        snake.draw();
        refresh();
    }
}

void print_startscreen() {
    // draw borders for the info box
    for (int i = 0; i < 68; i++) {
        mvaddch(0, i, BORDER_TOP_ICON);
        mvaddch(10, i, BORDER_BOTTOM_ICON);
    }
    for (int i=0; i < 10; i++) {
        mvaddch(i, 0, BORDER_LEFT_ICON);
        mvaddch(i, 68, BORDER_RIGHT_ICON);
    }

    mvprintw(5, 5, "Welcome to snake game. Use the keypad to control the snake.");
//    mvprintw(6, 5, "Press anything to start the game or 'q' to quit.");

    getch();
}

int main()
{
    Snake snake(5, 5, RIGHT);
    GameBoard board(BOARD_VERTICAL_LENGTH, BOARD_HORIZONTAL_LENGTH);

    srand(time(NULL));
    initscr();

    print_startscreen();

    nodelay(stdscr, true);
    curs_set(0);
    noecho();
    keypad(stdscr, true);

    mainloop(snake, board);
    endwin();
}