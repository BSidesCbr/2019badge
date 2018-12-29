#include "bsides2019.h"
#include "menus.h"

u8g2_t u8g2;

int main(void)
{
    
    display_init();
    display_logos();

    menu_enter(&menu_context, NULL);

    while(1){
        // read buttons
        // call menu_down()
        // call menu_up()
        // call menu_enter()
    }
}

