#ifndef __MSSH_GCONF__
#define __MSSH_GCONF__

#include <gconf/gconf-client.h>

#define MSSH_GCONF_PATH			"/apps/mssh"
#define MSSH_GCONF_KEY_FONT		MSSH_GCONF_PATH"/font"

void mssh_gconf_notify_font(GConfClient *client, guint cnxn_id,
	GConfEntry *entry, gpointer data);

#endif
