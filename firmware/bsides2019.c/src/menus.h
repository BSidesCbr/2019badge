#ifndef MENUS_H_INCLUDED
#define MENUS_H_INCLUDED

const menu_t main_menu = {
	.top_entry = 0,
	.current_entry = 0,
	.entry = {
		{.flags = MENU_FLAG_SUBMENU,
		 .select = NULL,
		 .name = "schedule",
		 .value = 0,
		},
		{.flags = MENU_FLAG_SUBMENU,
		 .select = NULL,
		 .name = "games",
		 .value = 0,
		},
		{.flags = 0,
		 .select = NULL,
		 .name = "reboot",
		 .value = 0,
		}
	},
	.num_entries = 3,
	.previous = NULL,
};

menu_t game_menu = {
	.top_entry = 0,
	.current_entry = 0,
	.entry =  {
		{.flags = 0,
		 .select = NULL,
		 .name = "snek",
		 .value = 0,
		},
		{.flags = 0,
		 .select = NULL,
		 .name = "tetra",
		 .value = 0,
		},
		{.flags = 0,
		 .select = NULL,
		 .name = "..back",
		 .value = 0,
		},
	},
	.num_entries = 3,
	.previous = NULL,
};

menu_t schedule_menu = {
	.top_entry = 0,
	.current_entry = 0,
	.entry = {
		{.flags = 0,
		 .select = NULL, 
		 .name = "day 1",
		 .value = 0,
		},
		{.flags = 0,
		 .select = NULL, 
		 .name = "day 2",
		 .value = 0,
		},
		{.flags = 0,
		 .select = NULL, 
		 .name = "..back",
		 .value = 0,
		},
	},
	.num_entries = 3,
	.previous = NULL,
};

menu_context_t menu_context = {
	.x_loc = 0,
	.y_loc = 2,
	.height = 4,
	.width = 10,
};

#endif
