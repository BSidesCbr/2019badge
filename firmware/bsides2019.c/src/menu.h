#ifndef MENU_H_INCLUDED
#define MENU_H_INCLUDED

#define MENU_FLAG_SUBMENU 1   // This entry calls a submenu
#define MENU_FLAG_HIDDEN 2    // don't display this entry
#define MENU_ENTRY_NAMELEN 16 // Max size of a menu entry's name

typedef struct menu_entry_s {
	uint8_t flags;
	void (*select)(void *arg, char *name);
	char name[MENU_ENTRY_NAMELEN];
	void *value;
} menu_entry_t;

typedef struct menu_s {
	uint8_t top_entry;
	uint8_t current_entry;
	uint8_t num_entries;
	struct menu_s *previous;
	menu_entry_t entry[];
} menu_t;

typedef struct {
	uint8_t x_loc;
	uint8_t y_loc;
	uint8_t height;
	uint8_t width;
	menu_t *menu;
} menu_context_t;

void menu_select(menu_context_t *context);
void menu_display(menu_context_t *context);
void menu_prev_entry(menu_context_t *context);
void menu_next_entry(menu_context_t *context);
void menu_exit(menu_context_t *context);
void menu_enter(menu_context_t *context, menu_t *menu);

#endif
