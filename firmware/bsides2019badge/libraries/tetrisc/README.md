Snake (C)
=========
Snake in C.
Based off YouTube video https://www.youtube.com/watch?v=xGmXxpIj6vs

Language
--------
C

Arduino
-------
Checkout/copy this library to your Arduino libraries folder.

How to use it
-------------
Include the library:

```c
#include <snakec.h>
```

Create memory for game data (size depends on grid size):

```c
uint8_t snake[SNKC_CALC_DATA_SIZE(20, 20)] = {0};
```

Set the grid size for the game, 20 x 20 seems to be the norm:

```c
// Snake game
if (!snkc_init(snake, sizeof(snake)))
{
    printf("snake init failed\n");
}
if (!snkc_set_grid(snake, 20, 20))
{
    printf("snake set grid failed\n");
}
if (!snkc_reset(snake))
{
    printf("snake reset failed\n");
}
```

Tell the snake game how to do things:

```c
snake.set_draw_clear([]() {
    // Clear grid of everything
});
snake.set_draw_snake([](int16_t x, int16_t y) {
    // Draw piece (block) of the snake at (x, y)
});
snake.set_draw_apple([](int16_t x, int16_t y) {
    // Draw the apple at (x, y)
});
snake.set_random([](int16_t min, int16_t max) -> int16_t {
    // return a random number between min and max (inclusive)
});
```

During the game, call the following functions when you get user input:

```cpp
snake.key_left();
snake.key_up();
snake.key_right();
snake.key_down();
```

Call the tick function 15 frames per second (or every 1000/15 milliseconds):

```cpp
snake.tick();
```

To reset the game at any point, call the reset function:

```cpp
snake.reset();
```
