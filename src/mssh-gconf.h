#ifndef __MSSH_GCONF__
#define __MSSH_GCONF__

#include <gconf/gconf-client.h>

#define MSSH_GCONF_PATH				"/apps/mssh"
#define MSSH_GCONF_KEY_FONT			MSSH_GCONF_PATH"/font"
#define MSSH_GCONF_KEY_FG_COLOUR	MSSH_GCONF_PATH"/fg_colour"
#define MSSH_GCONF_KEY_BG_COLOUR	MSSH_GCONF_PATH"/bg_colour"
#define MSSH_GCONF_KEY_COLUMNS		MSSH_GCONF_PATH"/columns"

void mssh_gconf_notify_font(GConfClient *client, guint cnxn_id,
	GConfEntry *entry, gpointer data);
void mssh_gconf_notify_fg_colour(GConfClient *client, guint cnxn_id,
	GConfEntry *entry, gpointer data);
void mssh_gconf_notify_bg_colour(GConfClient *client, guint cnxn_id,
	GConfEntry *entry, gpointer data);
void mssh_gconf_notify_columns(GConfClient *client, guint cnxn_id,
	GConfEntry *entry, gpointer data);

#endif
