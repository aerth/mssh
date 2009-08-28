#include <vte/vte.h>

#include "mssh-gconf.h"
#include "mssh-window.h"
#include "mssh-terminal.h"

void mssh_gconf_notify_font(GConfClient *client, guint cnxn_id,
	GConfEntry *entry, gpointer data)
{
	GConfValue *value;
	const gchar *font;
	int i;
	MSSHWindow *window = MSSH_WINDOW(data);

	value = gconf_entry_get_value(entry);
	font = gconf_value_get_string(value);

	for(i = 0; i < window->terminals->len; i++)
	{
		vte_terminal_set_font_from_string(VTE_TERMINAL(g_array_index(
			window->terminals, MSSHTerminal*, i)), font);
	}
}
