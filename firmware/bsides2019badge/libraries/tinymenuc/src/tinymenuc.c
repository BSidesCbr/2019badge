#include "tinymenuc.h"

TMNU_BOOL TMNU_API tmnu_init(void *data, size_t size)
{
    if (size < sizeof(TMNU_DATA))
    {
        return TMNU_FALSE;
    }
    memset(data, 0, size);
    ((TMNU_DATA*)data)->buffer_size = (size - (sizeof(TMNU_DATA)-1));
    return TMNU_TRUE;
}


static void tmnu_recalculate(TMNU_DATA *viewer) {
    size_t index = 0;
    size_t unused = 0;
    size_t line_height = 0;

    // use 'j' as it is usually the 'longer' one in vertical height
    char string_for_height[2] = {'j', '\0'};

    // reset
    viewer->item = 0;
    viewer->items_per_view = 0;

    // must have 'all the things' before I can calcualte
    if (0 == viewer->item_count) {
        return;
    }
    if (0 == viewer->view_width) {
        return;
    }
    if (0 == viewer->view_height) {
        return;
    }
    if (!(viewer->calc_string_view)) {
        return;
    }

    // determine the height of a single text line
    viewer->calc_string_view(viewer->calc_string_view_ctx, string_for_height, &unused, &line_height);
    viewer->items_per_view = viewer->view_height / line_height;

    // if the view is smaller then text, not much we can do
    if (0 == viewer->items_per_view) {
        return;
    }

    // done
    return;
}

TMNU_BOOL tmnu_set_view(void *data, size_t width, size_t height)
{
    TMNU_DATA *menu = (TMNU_DATA *)data;
    if (NULL == menu) {
        return TMNU_FALSE;
    }
    menu->view_width = width;
    menu->view_height = height;
    tmnu_recalculate(menu);
    return TMNU_TRUE;
}

TMNU_BOOL tmnu_set_menu_item_string(void *data, size_t item_count, TmnuMenuItemStringFn func, void *ctx)
{
    TMNU_DATA *menu = (TMNU_DATA *)data;
    if (NULL == menu) {
        return TMNU_FALSE;
    }
    menu->item_count = item_count;
    menu->menu_item_string = func;
    menu->menu_item_string_ctx = ctx;
    tmnu_recalculate(menu);
    return TMNU_TRUE;
}

TMNU_BOOL tmnu_set_calc_string_view(void *data, TmnuCalcStringViewFn func, void *ctx)
{
    TMNU_DATA *menu = (TMNU_DATA *)data;
    if (NULL == menu) {
        return TMNU_FALSE;
    }
    menu->calc_string_view = func;
    menu->calc_string_view_ctx = ctx;
    tmnu_recalculate(menu);
    return TMNU_TRUE;
}

TMNU_BOOL tmnu_set_draw_string(void *data, TmnuDrawStringFn func, void *ctx) {
    TMNU_DATA *menu = (TMNU_DATA *)data;
    if (NULL == menu) {
        return TMNU_FALSE;
    }
    menu->draw_string = func;
    menu->draw_string_ctx = ctx;
    tmnu_recalculate(menu);
    return TMNU_TRUE;
}

TMNU_BOOL tmnu_set_on_select(void *data, TmnuMenuItemOnSelectFn func, void *ctx) {
    TMNU_DATA *menu = (TMNU_DATA *)data;
    if (NULL == menu) {
        return TMNU_FALSE;
    }
    menu->on_menu_select = func;
    menu->on_menu_select_ctx = ctx;
    tmnu_recalculate(menu);
    return TMNU_TRUE;
}

TMNU_BOOL tmnu_get_item(void *data, size_t *item) {
    TMNU_DATA *menu = (TMNU_DATA *)data;
    if (NULL == menu) {
        return TMNU_FALSE;
    }
    if (item) {
        *item = menu->item;
    }
    return TMNU_TRUE;
}

TMNU_BOOL tmnu_set_item(void *data, size_t item)
{
    TMNU_DATA *menu = (TMNU_DATA *)data;
    if (NULL == menu) {
        return TMNU_FALSE;
    }
    menu->item = item;
    return TMNU_TRUE;
}

TMNU_BOOL tmnu_get_item_count(void *data, size_t *item_count) {
    TMNU_DATA *menu = (TMNU_DATA *)data;
    if (NULL == menu) {
        return TMNU_FALSE;
    }
    if (item_count) {
        *item_count = menu->item_count;
    }
    return TMNU_TRUE;
}

TMNU_BOOL tmnu_get_items_per_view(void *data, size_t *item_count) {
    TMNU_DATA *menu = (TMNU_DATA *)data;
    if (NULL == menu) {
        return TMNU_FALSE;
    }
    if (item_count) {
        *item_count = menu->items_per_view;
    }
    return TMNU_TRUE;
}

TMNU_BOOL tmnu_key_up(void *data) {
    TMNU_DATA *menu = (TMNU_DATA *)data;
    if (NULL == menu) {
        return TMNU_FALSE;
    }
    if (menu->item > 0) {
        menu->item -= 1;
    }
    return TMNU_TRUE;
}

TMNU_BOOL tmnu_key_down(void *data) {
    TMNU_DATA *menu = (TMNU_DATA *)data;
    if (NULL == menu) {
        return TMNU_FALSE;
    }
    if ((menu->item + 1) < menu->item_count) {
        menu->item += 1;
    }
    return TMNU_TRUE;
}

TMNU_BOOL tmnu_key_enter(void *data) {
    TMNU_DATA *menu = (TMNU_DATA *)data;
    if (NULL == menu) {
        return TMNU_FALSE;
    }
    if (menu->on_menu_select) {
        menu->on_menu_select(menu->on_menu_select_ctx, menu->item);
    }
    return TMNU_TRUE;
}

static void tmnu_read_menu_item_string(TMNU_DATA *menu, size_t item) {
    size_t size = 0;
    size_t unused = 0;
    size_t line_width = 0;

    // zero the buffer
    memset(menu->buffer, 0, menu->buffer_size);

    // read the string for this menu item
    menu->menu_item_string(menu->menu_item_string_ctx, item, menu->buffer, menu->buffer_size - 1);

    // make sure its still nul-terminated
    menu->buffer[menu->buffer_size - 1] = '\0';

    // shrink the string until it fits the width of the view
    menu->calc_string_view(menu->calc_string_view_ctx, menu->buffer, &line_width, &unused);
    while (line_width > menu->view_width) {
        size = strlen(menu->buffer);
        if (0 == size) {
            break;
        }
        menu->buffer[size - 1] = '\0';
        menu->calc_string_view(menu->calc_string_view_ctx, menu->buffer, &line_width, &unused);
    }
}

TMNU_BOOL tmnu_draw_view(void *data) {
    size_t item_index = 0;
    size_t item_selected = 0;
    size_t view_index = 0;
    size_t unused = 0;
    size_t line_height = 0;

    // use 'j' as it is usually the 'longer' one in vertical height
    char string_for_height[2] = {'j', '\0'};

    TMNU_DATA *menu = (TMNU_DATA *)data;
    if (NULL == menu) {
        return TMNU_FALSE;
    }

    // check we have the apis we need
    if (!(menu->menu_item_string)) {
        return TMNU_FALSE;
    }
    if (!(menu->calc_string_view)) {
        return TMNU_FALSE;
    }
    if (!(menu->draw_string)) {
        return TMNU_FALSE;
    }

    // need the line height
    menu->calc_string_view(menu->calc_string_view_ctx, string_for_height, &unused, &line_height);

    // find the view with items_per_view items
    view_index = ((menu->item) / (menu->items_per_view));
    item_selected = ((menu->item) % (menu->items_per_view));

    // draw the items
    for (item_index = 0; item_index < menu->items_per_view; item_index++) {
        // handle the case where the last view is not full
        if (((view_index * (menu->items_per_view)) + item_index) >= menu->item_count) {
            break;
        }

        // get the string for the item
        tmnu_read_menu_item_string(menu, (view_index * (menu->items_per_view)) + item_index);

        // draw the item
        if (item_selected == item_index) {
            menu->draw_string(menu->draw_string_ctx, 0, item_index * line_height, menu->buffer, TMNU_TRUE);
        } else {
            menu->draw_string(menu->draw_string_ctx, 0, item_index * line_height, menu->buffer, TMNU_FALSE);
        }
    }

    // done
    return TMNU_TRUE;
}

TMNU_BOOL TMNU_API tmnu_fini(void *data)
{
    size_t size = sizeof(TMNU_DATA);
    if (NULL == data) {
        return TMNU_FALSE;
    }
    memset(data, 0, size);
    return TMNU_TRUE;
}
