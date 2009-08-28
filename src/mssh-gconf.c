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

void mssh_gconf_notify_fg_colour(GConfClient *client, guint cnxn_id,
	GConfEntry *entry, gpointer data)
{
	GConfValue *value;
	const gchar *colour_s;
	GdkVisual *visual = gdk_visual_get_system();
	GdkColormap *colour_map;
	GdkColor colour;
	int i;

	MSSHWindow *window = MSSH_WINDOW(data);

	value = gconf_entry_get_value(entry);
	colour_s = gconf_value_get_string(value);
	colour_map = gdk_colormap_new(visual, TRUE);
	gdk_colormap_alloc_color(colour_map, &colour, TRUE, TRUE);

	gdk_color_parse(colour_s, &colour);

	for(i = 0; i < window->terminals->len; i++)
	{
		vte_terminal_set_color_foreground(VTE_TERMINAL(g_array_index(
			window->terminals, MSSHTerminal*, i)), &colour);
	}
}

void mssh_gconf_notify_bg_colour(GConfClient *client, guint cnxn_id,
	GConfEntry *entry, gpointer data)
{
	GConfValue *value;
	const gchar *colour_s;
	GdkVisual *visual = gdk_visual_get_system();
	GdkColormap *colour_map;
	GdkColor colour;
	int i;

	MSSHWindow *window = MSSH_WINDOW(data);

	value = gconf_entry_get_value(entry);
	colour_s = gconf_value_get_string(value);
	colour_map = gdk_colormap_new(visual, TRUE);
	gdk_colormap_alloc_color(colour_map, &colour, TRUE, TRUE);

	gdk_color_parse(colour_s, &colour);

	for(i = 0; i < window->terminals->len; i++)
	{
		vte_terminal_set_color_background(VTE_TERMINAL(g_array_index(
			window->terminals, MSSHTerminal*, i)), &colour);
	}
}
