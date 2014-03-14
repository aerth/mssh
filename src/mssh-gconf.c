#include <vte/vte.h>
#include <gdk/gdkkeysyms.h>

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
    GdkRGBA colour;
    int i;

    MSSHWindow *window = MSSH_WINDOW(data);

    value = gconf_entry_get_value(entry);
    colour_s = gconf_value_get_string(value);
    gdk_rgba_parse(&colour, colour_s);

    for(i = 0; i < window->terminals->len; i++)
    {
        vte_terminal_set_color_foreground_rgba(VTE_TERMINAL(g_array_index(
            window->terminals, MSSHTerminal*, i)), &colour);
    }
}

void mssh_gconf_notify_bg_colour(GConfClient *client, guint cnxn_id,
    GConfEntry *entry, gpointer data)
{
    GConfValue *value;
    const gchar *colour_s;
    GdkRGBA colour;
    int i;

    MSSHWindow *window = MSSH_WINDOW(data);

    value = gconf_entry_get_value(entry);
    colour_s = gconf_value_get_string(value);
    gdk_rgba_parse(&colour, colour_s);

    for(i = 0; i < window->terminals->len; i++)
    {
        vte_terminal_set_color_background_rgba(VTE_TERMINAL(g_array_index(
            window->terminals, MSSHTerminal*, i)), &colour);
    }
}

void mssh_gconf_notify_fg_colour_focus(GConfClient *client, guint cnxn_id,
    GConfEntry *entry, gpointer data)
{
    GConfValue *value;
    const gchar *colour_s;
    GdkRGBA colour;
    int i;
    int idx = -1;
    GtkWidget *focus;

    MSSHWindow *window = MSSH_WINDOW(data);

    value = gconf_entry_get_value(entry);
    colour_s = gconf_value_get_string(value);
    gdk_rgba_parse(&colour, colour_s);

    /* get the currently focused window */
	focus = gtk_window_get_focus(GTK_WINDOW(window));

    /* find the focused window in the terminal list */
    for(i = 0; i < window->terminals->len; i++)
    {
        if(focus == GTK_WIDGET(g_array_index(window->terminals,
            MSSHTerminal*, i)))
        {
            idx = i;
            break;
        }
    }

    if (idx != -1) {
        /* found the currently focused terminal, update the color */
        vte_terminal_set_color_foreground_rgba(VTE_TERMINAL(g_array_index(
            window->terminals, MSSHTerminal*, idx)), &colour);
    }
}

void mssh_gconf_notify_bg_colour_focus(GConfClient *client, guint cnxn_id,
    GConfEntry *entry, gpointer data)
{
    GConfValue *value;
    const gchar *colour_s;
    GdkRGBA colour;
    int i;
    int idx = -1;
	GtkWidget *focus;

    MSSHWindow *window = MSSH_WINDOW(data);

    value = gconf_entry_get_value(entry);
    colour_s = gconf_value_get_string(value);
    gdk_rgba_parse(&colour, colour_s);

    /* get the currently focused window */
    focus = gtk_window_get_focus(GTK_WINDOW(window));

    /* find the focused window in the terminal list */
    for(i = 0; i < window->terminals->len; i++)
    {
        if(focus == GTK_WIDGET(g_array_index(window->terminals,
            MSSHTerminal*, i)))
        {
            idx = i;
            break;
        }
    }

    if (idx != -1) {
        /* found the currently focused terminal, update the color */
        vte_terminal_set_color_background_rgba(VTE_TERMINAL(g_array_index(
            window->terminals, MSSHTerminal*, idx)), &colour);
    }

}
void mssh_gconf_notify_columns(GConfClient *client, guint cnxn_id,
    GConfEntry *entry, gpointer data)
{
    GConfValue *value;
    int columns;

    MSSHWindow *window = MSSH_WINDOW(data);

    value = gconf_entry_get_value(entry);
    columns = gconf_value_get_int(value);

    if(columns <= 0)
    {
        columns = 1;
        gconf_client_set_int(client, MSSH_GCONF_KEY_COLUMNS, columns,
            NULL);
    }

    window->columns = columns;
    mssh_window_relayout(window);
}

void mssh_gconf_notify_timeout(GConfClient *client, guint cnxn_id,
    GConfEntry *entry, gpointer data)
{
    GConfValue *value;
    int timeout;

    MSSHWindow *window = MSSH_WINDOW(data);

    value = gconf_entry_get_value(entry);
    timeout = gconf_value_get_int(value);

    if(timeout < 0)
    {
        timeout = 0;
        gconf_client_set_int(client, MSSH_GCONF_KEY_TIMEOUT, timeout,
            NULL);
    }

    window->timeout = timeout;
    mssh_window_relayout(window);
}

void mssh_gconf_notify_close_ended(GConfClient *client, guint cnxn_id,
    GConfEntry *entry, gpointer data)
{
    GConfValue *value;
    gboolean close_ended;
    int i;

    MSSHWindow *window = MSSH_WINDOW(data);

    value = gconf_entry_get_value(entry);
    close_ended = gconf_value_get_bool(value);

    window->close_ended_sessions = close_ended;

    if(close_ended)
    {
        for(i = 0; i < window->terminals->len; i++)
        {
            MSSHTerminal *terminal = g_array_index(window->terminals,
                MSSHTerminal*, i);

            if(terminal->ended)
            {
                mssh_window_session_closed(terminal, window);
            }
        }
    }
}

void mssh_gconf_notify_recolor_focused(GConfClient *client, guint cnxn_id,
    GConfEntry *entry, gpointer data)
{
    GConfValue *value;
    gboolean recolor_focused;

    MSSHWindow *window = MSSH_WINDOW(data);

    value = gconf_entry_get_value(entry);
    recolor_focused = gconf_value_get_bool(value);

    window->recolor_focused = recolor_focused;

}

void mssh_gconf_notify_quit_all_ended(GConfClient *client, guint cnxn_id,
    GConfEntry *entry, gpointer data)
{
    GConfValue *value;

    MSSHWindow *window = MSSH_WINDOW(data);

    value = gconf_entry_get_value(entry);

    window->exit_on_all_closed = gconf_value_get_bool(value);
}

void mssh_gconf_notify_dir_focus(GConfClient *client, guint cnxn_id,
    GConfEntry *entry, gpointer data)
{
    GConfValue *value;

    MSSHWindow *window = MSSH_WINDOW(data);

    value = gconf_entry_get_value(entry);

    window->dir_focus = gconf_value_get_bool(value);
}

void mssh_gconf_notify_modifier(GConfClient *client, guint cnxn_id,
    GConfEntry *entry, gpointer data)
{
    GConfValue *value;

    MSSHWindow *window = MSSH_WINDOW(data);

    value = gconf_entry_get_value(entry);

    if(window->accel)
    {
        gtk_accel_group_disconnect_key(window->accel, GDK_KEY_Up,
            window->modifier);
        gtk_accel_group_disconnect_key(window->accel, GDK_KEY_Down,
            window->modifier);
        gtk_accel_group_disconnect_key(window->accel, GDK_KEY_Left,
            window->modifier);
        gtk_accel_group_disconnect_key(window->accel, GDK_KEY_Right,
            window->modifier);
    }

    window->modifier = gconf_value_get_int(value);

    if(window->accel)
    {
        gtk_accel_group_connect(window->accel, GDK_KEY_Up, window->modifier,
            GTK_ACCEL_VISIBLE, g_cclosure_new(
            G_CALLBACK(mssh_window_focus), window, NULL));
        gtk_accel_group_connect(window->accel, GDK_KEY_Down, window->modifier,
            GTK_ACCEL_VISIBLE, g_cclosure_new(
            G_CALLBACK(mssh_window_focus), window, NULL));
        gtk_accel_group_connect(window->accel, GDK_KEY_Left, window->modifier,
            GTK_ACCEL_VISIBLE, g_cclosure_new(
            G_CALLBACK(mssh_window_focus), window, NULL));
        gtk_accel_group_connect(window->accel, GDK_KEY_Right, window->modifier,
            GTK_ACCEL_VISIBLE, g_cclosure_new(
            G_CALLBACK(mssh_window_focus), window, NULL));
    }
}


void mssh_gconf_backscroll_buffer_size(GConfClient *client, guint cnxn_id,
    GConfEntry *entry, gpointer data)
{
    GConfValue *value;
    gint backscroll_buffer_size;

    MSSHWindow *window = MSSH_WINDOW(data);

    int i;
    int len = window->terminals->len;

    value = gconf_entry_get_value(entry);
    backscroll_buffer_size = gconf_value_get_int(value);


    if (backscroll_buffer_size < -1)
    {
        backscroll_buffer_size = 5000;
        gconf_client_set_int(client, MSSH_GCONF_KEY_BACKSCROLL_BUFFER_SIZE, backscroll_buffer_size,
            NULL);
    }

    window->backscroll_buffer_size = backscroll_buffer_size;
    /* reconfigure all terminals with the new size*/
    for(i = 0; i < len; i++)
    {
        mssh_terminal_set_backscroll_size(g_array_index(window->terminals,
            MSSHTerminal*, i), &backscroll_buffer_size);
    }
}
