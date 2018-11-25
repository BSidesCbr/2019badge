Virtual Interrupts (vint)
=========================
Platform independent way to make timed interrupts from a tick count API.

Language
--------
C++11 and up (should compile for anything, doesn't use any OS APIs).

Arduino
-------
Checkout/copy this library to your Arduino libraries folder.

How to use it
-------------
Include the library:

```cpp
#include <vint.h>
```

Construct the class:

```cpp
vint::Interrupt interrupts;
```

Tell the library how to get the tick count:

```cpp
interrupts.set_get_tick_count([]() -> uint32_t {
    // return the tick count
    // Arduino: return (uint32_t) millis();  // only acurate to 10ms
    // Windows: (uint32_t)GetTickCount();
});
```

During the game, call the following functions when you get user input:

```cpp
interrupts.set_interrupt(100, [](){
    // Do something every 100 ticks (e.g. every 100 milliseconds if thats your tick count source)
});
interrupts.set_interrupt(210, [](){
    // Do something every 210 ticks (e.g. every 210 milliseconds if thats your tick count source)
});
```

You can also remove interrupts using a handle:
```cpp
auto handle = interrupts.set_interrupt(100, [](){
    // do something
});

// stop doing something
interrupts.remove(handle);
```

Check and trigger interrupts by call the followin in a loop:

```cpp
interrupts.run_loop();
```
