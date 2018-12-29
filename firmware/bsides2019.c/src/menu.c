#include "bsides2019.h"

void menu_enter(menu_context_t *context, menu_t *menu)
{
    u8g2_ClearBuffer(&u8g2);
    u8g2_ClearDisplay(&u8g2);
    u8g2_DrawStr(&u8g2, 1, 1, "dakka");
    u8g2_SendBuffer(&u8g2);

    menu->previous = context->menu;
    context->menu = menu;
    menu_display(context);
}

void menu_select(menu_context_t *context)
{
    menu_entry_t *entry = &context->menu->entry[context->menu->current_entry];
    u8g2_ClearBuffer(&u8g2);
    u8g2_ClearDisplay(&u8g2);
    u8g2_DrawStr(&u8g2, 0, 0, "pakka");

    if (entry->flags & MENU_FLAG_SUBMENU) {
            menu_enter(context, (menu_t *)entry->value);
    } else {
            entry->select(entry->value, entry->name);
    }
    
    menu_display(context);
}

void menu_display(menu_context_t *context)
{
    uint8_t i;
    menu_t *menu = context->menu;
    menu_entry_t *disp_entry;
    uint8_t dindex = 0;

    u8g2_ClearBuffer(&u8g2);
    u8g2_DrawStr(&u8g2, 0, 0, "wakka");

    //loop through visible entries
    for (i = 0; i < context->height; i++) {
        do {
            disp_entry = &menu->entry[menu->top_entry + dindex];
            if (dindex++ >= menu->num_entries - menu->top_entry)
                return;
        } while (disp_entry->flags & MENU_FLAG_HIDDEN);

        // if this item is the selected one
        if (menu->current_entry == dindex-1) {
            u8g2_SetDrawColor(&u8g2, 0);
        } else {
            u8g2_SetDrawColor(&u8g2, 1);
        }

        u8g2_DrawStr(&u8g2, context->x_loc, context->y_loc+i, disp_entry->name);
    }
}

void menu_next_entry(menu_context_t *context)
{
    menu_t *menu = context->menu;
    uint8_t new_entry = menu->current_entry;

    while(1) {
        if (++new_entry >= menu->num_entries)
            return;
        if (!(menu->entry[new_entry].flags & MENU_FLAG_HIDDEN))
            break;
    }

    menu->current_entry = new_entry;

    if (menu->current_entry >= menu->top_entry + context->height)
        menu->top_entry = menu->current_entry - context->height;

    menu_display(context);
}


void menu_prev_entry(menu_context_t *context)
{
    menu_t *menu = context->menu;
    uint8_t new_entry = menu->current_entry;


    while(1) {
        if (new_entry-- == 0)
            return;

        if (!(menu->entry[new_entry].flags & MENU_FLAG_HIDDEN))
            break;
    }

    menu->current_entry = new_entry;

    if (menu->current_entry < menu->top_entry) 
        menu->top_entry = menu->current_entry;

    menu_display(context);
}


void menu_exit(menu_context_t *context)
{
    if (context->menu->previous)
        context->menu = context->menu->previous;

    menu_display(context);
}


