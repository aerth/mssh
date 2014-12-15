#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <gconf/gconf-client.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#define GETTEXT_PACKAGE "mssh"
#include <glib/gi18n-lib.h>

#include "mssh-terminal.h"
#include "mssh-pref.h"
#include "mssh-gconf.h"
#include "mssh-window.h"

#include "config.h"

#include <regex.h>  

static void mssh_window_sendhost(GtkWidget *widget, gpointer data);
static void mssh_window_destroy(GtkWidget *widget, gpointer data);
static void mssh_window_pref(GtkWidget *widget, gpointer data);
static gboolean mssh_window_key_press(GtkWidget *widget,
    GdkEventKey *event, gpointer data);
static gboolean mssh_window_entry_focused(GtkWidget *widget,
    GtkDirectionType dir, gpointer data);
static gboolean mssh_window_session_close(gpointer data);
static void mssh_window_session_focused(MSSHTerminal *terminal,
    gpointer data);
static gboolean mssh_window_mouse_paste_cb(MSSHTerminal *terminal,
    gpointer data);
static void mssh_window_insert(GtkWidget *widget, gchar *new_text,
    gint new_text_length, gint *position, gpointer data);
static void mssh_window_add_session(MSSHWindow *window, char *hostname);
static void mssh_window_init(MSSHWindow* window);
static void mssh_window_class_init(MSSHWindowClass *klass);
static void mssh_window_add(GtkWidget *widget, gpointer data);
gboolean mssh_window_dialog_emit_response(GtkWidget *widget, GObject *acceleratable,
    guint keyval, GdkModifierType modifier, gpointer data);
static void mssh_window_maximize(GtkWidget *widget, gpointer data);
static void mssh_window_restore_layout(GtkWidget *widget, gpointer data);
void mssh_window_relayout_for_one(MSSHWindow *window, GtkWidget *t);

G_DEFINE_TYPE(MSSHWindow, mssh_window, GTK_TYPE_WINDOW)

struct WinTermPair
{
    MSSHWindow *window;
    MSSHTerminal *terminal;
};

GtkWidget* mssh_window_new(void)
{
    return g_object_new(MSSH_TYPE_WINDOW, NULL);
}

static void mssh_window_sendhost(GtkWidget *widget, gpointer data)
{
    int i;

    MSSHWindow *window = MSSH_WINDOW(data);

    for(i = 0; i < window->terminals->len; i++)
    {
        mssh_terminal_send_host(g_array_index(window->terminals,
            MSSHTerminal*, i));
    }
}

static void mssh_window_sendcommand(GtkWidget *widget, gpointer data)
{
    int i;
    char *command;

    MSSHWindow *window = MSSH_WINDOW(data);
    GtkMenuItem *item = (GtkMenuItem *)widget;

    command = g_datalist_get_data(MSSH_WINDOW(data)->commands, gtk_menu_item_get_label (item));

    for(i = 0; i < window->terminals->len; i++)
    {
        mssh_terminal_send_string(g_array_index(window->terminals,
            MSSHTerminal*, i), command);
    }

}

static void mssh_window_destroy(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
}

static void mssh_window_pref(GtkWidget *widget, gpointer data)
{
    MSSHWindow *window = MSSH_WINDOW(data);
    GtkWidget *pref = mssh_pref_new();

    gtk_window_set_transient_for(GTK_WINDOW(pref), GTK_WINDOW(window));
    gtk_window_set_modal(GTK_WINDOW(pref), TRUE);
    gtk_window_set_position(GTK_WINDOW(pref),
        GTK_WIN_POS_CENTER_ON_PARENT);

    gtk_widget_show_all(pref);
}

static void mssh_window_insert(GtkWidget *widget, gchar *new_text,
    gint new_text_length, gint *position, gpointer data)
{
    int i;

    MSSHWindow *window = MSSH_WINDOW(data);

    for(i = 0; i < window->terminals->len; i++)
    {
        mssh_terminal_send_string(g_array_index(window->terminals,
            MSSHTerminal*, i), new_text);
    }

    g_signal_stop_emission_by_name(G_OBJECT(widget), "insert-text");
}

static gboolean mssh_window_key_press(GtkWidget *widget,
    GdkEventKey *event, gpointer data)
{
    int i;

    MSSHWindow *window = MSSH_WINDOW(data);

    for(i = 0; i < window->terminals->len; i++)
    {
        mssh_terminal_send_data(g_array_index(window->terminals,
            MSSHTerminal*, i), event);
    }

    return TRUE;
}

static gboolean mssh_window_entry_focused(GtkWidget *widget,
    GtkDirectionType dir, gpointer data)
{
    GConfClient *client;
    GConfEntry *entry;
    MSSHWindow *window = MSSH_WINDOW(data);

    gtk_window_set_title(GTK_WINDOW(window), PACKAGE_NAME" - All");
    window->last_focus = NULL;

    /* clear the coloring for the focused window */
    client = gconf_client_get_default();

    entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_FG_COLOUR, NULL,
        TRUE, NULL);
    mssh_gconf_notify_fg_colour(client, 0, entry, window);
    entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_BG_COLOUR, NULL,
        TRUE, NULL);
    mssh_gconf_notify_bg_colour(client, 0, entry, window);
    return FALSE;
}

gboolean mssh_window_focus(GtkWidget *widget, GObject *acceleratable,
    guint keyval, GdkModifierType modifier, gpointer data)
{
    MSSHWindow *window = MSSH_WINDOW(data);
    GtkWidget *focus;
    GConfClient *client;
    GConfEntry *entry;

    int i, idx = -1, len = window->terminals->len;
    int wcols = window->columns_override ? window->columns_override :
        window->columns;
    int cols = (len < wcols) ? len : wcols;

    focus = gtk_window_get_focus(GTK_WINDOW(window));

    for(i = 0; i < len; i++)
    {
        if(focus == GTK_WIDGET(g_array_index(window->terminals,
            MSSHTerminal*, i)))
        {
            idx = i;
            break;
        }
    }

    client = gconf_client_get_default();

    /* recolor the windows */
    if (window->recolor_focused) {
        entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_FG_COLOUR, NULL,
            TRUE, NULL);
        mssh_gconf_notify_fg_colour(client, 0, entry, window);
        entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_BG_COLOUR, NULL,
            TRUE, NULL);
        mssh_gconf_notify_bg_colour(client, 0, entry, window);
    }
    if(focus == window->global_entry && keyval == GDK_KEY_Down &&
        window->dir_focus)
        idx = 0;
    else if(idx == -1 && window->dir_focus)
        return TRUE;
    else
    {
        switch(keyval)
        {
        case GDK_KEY_Up:
            if(window->dir_focus)
                idx = idx - cols;
            break;
        case GDK_KEY_Down:
            if(window->dir_focus)
            {
                if((idx + cols >= len) && (idx < len -
                    (len % cols) ? (len % cols) : cols))
                    idx = len - 1;
                else
                    idx = idx + cols;
            }
            break;
        case GDK_KEY_Left:
            if(idx % cols != 0 || !window->dir_focus)
                idx = idx - 1;
            break;
        case GDK_KEY_Right:
            if(idx % cols != cols - 1 || !window->dir_focus)
                idx = idx + 1;
            break;
        }
    }

    if(idx >= len && !window->dir_focus)
        focus = window->global_entry;

    if(idx < -1 && !window->dir_focus)
        idx = len - 1;

    if(idx < 0)
        focus = window->global_entry;
    else if(idx < len)
    {
        focus = GTK_WIDGET(g_array_index(window->terminals,
            MSSHTerminal*, idx));
    }

    gtk_window_set_focus(GTK_WINDOW(window), focus);

    return TRUE;
}

static gboolean mssh_window_session_close(gpointer data)
{
    int i, idx = -1;

    struct WinTermPair *data_pair = (struct WinTermPair*)data;

    for(i = 0; i < data_pair->window->terminals->len; i++)
    {
        if(data_pair->terminal == g_array_index(
            data_pair->window->terminals, MSSHTerminal*, i))
        {
            idx = i;
            break;
        }
    }

    data_pair->window->last_closed = idx;

    if(idx == -1)
    {
        fprintf(stderr,
            _("mssh: Fatal Error: Can't find terminal to remove!\n"));
    }
    else
    {
        /* set the focus on the entry only if the terminal closed has it */
        if ( gtk_window_get_focus(GTK_WINDOW(data_pair->window)) == GTK_WIDGET(data_pair->terminal) ) {
            gtk_window_set_focus(GTK_WINDOW(data_pair->window), GTK_WIDGET(data_pair->window->global_entry));
        }

        gtk_widget_destroy(data_pair->terminal->menu_item);

        gtk_container_remove(GTK_CONTAINER(data_pair->window->grid),
            GTK_WIDGET(data_pair->terminal));

        g_array_remove_index(data_pair->window->terminals, idx);

        mssh_window_relayout(data_pair->window);

    }

    if(data_pair->window->terminals->len == 0 &&
        data_pair->window->exit_on_all_closed)
    {
        mssh_window_destroy(NULL, (void*)data_pair->window);
    }

    free(data_pair);

    return FALSE;
}

void mssh_window_session_closed(MSSHTerminal *terminal, gpointer data)
{
    struct WinTermPair *data_pair = malloc(sizeof(struct WinTermPair));
    data_pair->terminal = terminal;
    data_pair->window = MSSH_WINDOW(data);

    if(data_pair->window->close_ended_sessions)
    {
        g_timeout_add_seconds(data_pair->window->timeout,
            mssh_window_session_close, data_pair);
    }
}

static gboolean mssh_window_mouse_paste_cb(MSSHTerminal *terminal,
    gpointer data)
{
    gtk_widget_grab_focus(GTK_WIDGET(terminal));

    return FALSE;
}

static void mssh_window_session_focused(MSSHTerminal *terminal,
    gpointer data)
{
    char *title;
    size_t len;

    GConfClient *client;
    GConfEntry *entry;
    MSSHWindow *window = MSSH_WINDOW(data);

    len = strlen(PACKAGE_NAME" - ") + strlen(terminal->hostname) + 1;
    title = malloc(len);

    snprintf(title, len, PACKAGE_NAME" - %s", terminal->hostname);

    gtk_window_set_title(GTK_WINDOW(window), title);

    free(title);
    client = gconf_client_get_default();

    /* recolor all windows */
    entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_FG_COLOUR, NULL,
        TRUE, NULL);
    mssh_gconf_notify_fg_colour(client, 0, entry, window);
    entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_BG_COLOUR, NULL,
        TRUE, NULL);
    mssh_gconf_notify_bg_colour(client, 0, entry, window);

    /* recolor the focused window - if needed */
    if (window->recolor_focused && window->is_maximized == 0) {
        entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_FG_COLOUR_FOCUS, NULL,
            TRUE, NULL);
        mssh_gconf_notify_fg_colour_focus(client, 0, entry, window);
        entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_BG_COLOUR_FOCUS, NULL,
            TRUE, NULL);
        mssh_gconf_notify_bg_colour_focus(client, 0, entry, window);
    }
}

void mssh_window_relayout(MSSHWindow *window)
{
    GConfClient *client;
    GConfEntry *entry;
    GtkWidget *focus;
    int i, len = window->terminals->len;
    int wcols = window->columns_override ? window->columns_override :
        window->columns;
    int cols = (len < wcols) ? len : wcols;
    int width = 1;

    focus = gtk_window_get_focus(GTK_WINDOW(window));

    if(!focus)
    {
        if(window->last_closed < 0)
            window->last_closed = 0;

        if(len == 0)
            focus = window->global_entry;
        else if(window->last_closed < len)
        {
            focus = GTK_WIDGET(g_array_index(window->terminals,
                MSSHTerminal*, window->last_closed));
        }
        else
        {
            focus = GTK_WIDGET(g_array_index(window->terminals,
                MSSHTerminal*, 0));
        }
    }

    for(i = 0; i < len; i++)
    {
        MSSHTerminal *terminal = g_array_index(window->terminals,
            MSSHTerminal*, i);

        g_object_ref(terminal);
        if(gtk_widget_get_parent(GTK_WIDGET(terminal)) == GTK_WIDGET(window->grid))
        {
            gtk_container_remove(GTK_CONTAINER(window->grid),
                GTK_WIDGET(terminal));
        }

        /* Set margins to terminal widget */
        gtk_widget_set_margin_start(GTK_WIDGET(terminal), 1);
        gtk_widget_set_margin_end(GTK_WIDGET(terminal), 1);
        gtk_widget_set_margin_top(GTK_WIDGET(terminal), 1);
        gtk_widget_set_margin_bottom(GTK_WIDGET(terminal), 1);

        if (i == len - 1) {
            width = cols - (i % cols);
        }
        gtk_grid_attach(GTK_GRID(window->grid), /* grid */
                        GTK_WIDGET(terminal),   /* child */
                        (i % cols),             /* left */
                        i / cols,               /* top */
                        width,                  /* width */
                        1);                     /* height */

        g_object_unref(terminal);

        if(!terminal->started)
        {
            mssh_terminal_start_session(terminal, window->env);
            terminal->started = 1;
        }
    }

    client = gconf_client_get_default();

    gtk_widget_show_all(GTK_WIDGET(window));

    entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_FONT, NULL,
        TRUE, NULL);
    mssh_gconf_notify_font(client, 0, entry, window);
    entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_FG_COLOUR, NULL,
        TRUE, NULL);
    mssh_gconf_notify_fg_colour(client, 0, entry, window);
    entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_BG_COLOUR, NULL,
        TRUE, NULL);
    mssh_gconf_notify_bg_colour(client, 0, entry, window);

    gtk_window_set_focus(GTK_WINDOW(window), GTK_WIDGET(focus));
}

static void mssh_window_add_session(MSSHWindow *window, char *hostname)
{
    MSSHTerminal *terminal = MSSH_TERMINAL(mssh_terminal_new());

    terminal->backscroll_buffer_size = window->backscroll_buffer_size;
    g_array_append_val(window->terminals, terminal);

    g_signal_connect(G_OBJECT(terminal), "session-closed",
        G_CALLBACK(mssh_window_session_closed), window);
    g_signal_connect(G_OBJECT(terminal), "session-focused",
        G_CALLBACK(mssh_window_session_focused), window);
    g_signal_connect(GTK_WIDGET(terminal), "button-release-event",
        G_CALLBACK(mssh_window_mouse_paste_cb), window);

    mssh_terminal_init_session(terminal, hostname);

    gtk_menu_shell_append(GTK_MENU_SHELL(window->server_menu),
        terminal->menu_item);
}

static void mssh_window_init(MSSHWindow* window)
{
    GConfClient *client;

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *entry = gtk_entry_new();

    GtkWidget *menu_bar = gtk_menu_bar_new();
    GtkWidget *file_menu = gtk_menu_new();
    GtkWidget *edit_menu = gtk_menu_new();

    GtkWidget *file_item = gtk_menu_item_new_with_label(_("File"));
    GtkWidget *edit_item = gtk_menu_item_new_with_label(_("Edit"));
    GtkWidget *server_item = gtk_menu_item_new_with_label(_("Servers"));
    GtkWidget *command_item = gtk_menu_item_new_with_label(_("Commands"));

    GtkWidget *file_quit = gtk_menu_item_new_with_mnemonic(_("_Quit"));
    GtkWidget *file_sendhost = gtk_menu_item_new_with_label(_("Send hostname"));
    GtkWidget *file_add = gtk_menu_item_new_with_label(_("Add session"));

    GtkWidget *edit_pref = gtk_menu_item_new_with_mnemonic(_("_Edit"));

    GtkAccelGroup *accel = gtk_accel_group_new();

    window->accel = NULL;

    window->server_menu = gtk_menu_new();

    window->command_menu = gtk_menu_new();

    window->global_entry = entry;

    window->last_closed = -1;

    window->terminals = g_array_new(FALSE, TRUE, sizeof(MSSHTerminal*));

    window->backscroll_buffer_size = 5000;

    window->is_maximized = 0;

   window->recolor_focused = FALSE;

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit_item), edit_menu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(server_item),
        window->server_menu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(command_item),
        window->command_menu);

    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_add);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_sendhost);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_quit);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), edit_pref);
    g_signal_connect(G_OBJECT(file_sendhost), "activate",
        G_CALLBACK(mssh_window_sendhost), window);
    g_signal_connect(G_OBJECT(file_add), "activate",
        G_CALLBACK(mssh_window_add), window);
    g_signal_connect(G_OBJECT(file_quit), "activate",
        G_CALLBACK(mssh_window_destroy), window);
    g_signal_connect(G_OBJECT(edit_pref), "activate",
        G_CALLBACK(mssh_window_pref), window);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), edit_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), server_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), command_item);

    g_signal_connect(G_OBJECT(entry), "key-press-event",
        G_CALLBACK(mssh_window_key_press), window);
    g_signal_connect(G_OBJECT(entry), "insert-text",
        G_CALLBACK(mssh_window_insert), window);
    g_signal_connect(G_OBJECT(entry), "focus-in-event",
        G_CALLBACK(mssh_window_entry_focused), window);

    gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, TRUE, 2);

    window->grid = gtk_grid_new();
    gtk_grid_set_row_homogeneous(GTK_GRID(window->grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(window->grid), TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), window->grid, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(window), vbox);

    gtk_widget_set_size_request(GTK_WIDGET(window), 0, 0);
    gtk_window_set_default_size(GTK_WINDOW(window), 1024, 768);
    gtk_window_set_title(GTK_WINDOW(window), PACKAGE_NAME);

    client = gconf_client_get_default();

    gconf_client_add_dir(client, MSSH_GCONF_PATH,
        GCONF_CLIENT_PRELOAD_RECURSIVE, NULL);

    gconf_client_notify_add(client, MSSH_GCONF_KEY_FONT,
        mssh_gconf_notify_font, window, NULL, NULL);
    gconf_client_notify_add(client, MSSH_GCONF_KEY_FG_COLOUR,
        mssh_gconf_notify_fg_colour, window, NULL, NULL);
    gconf_client_notify_add(client, MSSH_GCONF_KEY_BG_COLOUR,
        mssh_gconf_notify_bg_colour, window, NULL, NULL);
    gconf_client_notify_add(client, MSSH_GCONF_KEY_FG_COLOUR_FOCUS,
        mssh_gconf_notify_fg_colour_focus, window, NULL, NULL);
    gconf_client_notify_add(client, MSSH_GCONF_KEY_BG_COLOUR_FOCUS,
        mssh_gconf_notify_bg_colour_focus, window, NULL, NULL);
    gconf_client_notify_add(client, MSSH_GCONF_KEY_COLUMNS,
        mssh_gconf_notify_columns, window, NULL, NULL);
    gconf_client_notify_add(client, MSSH_GCONF_KEY_TIMEOUT,
        mssh_gconf_notify_timeout, window, NULL, NULL);
    gconf_client_notify_add(client, MSSH_GCONF_KEY_CLOSE_ENDED,
        mssh_gconf_notify_close_ended, window, NULL, NULL);
    gconf_client_notify_add(client, MSSH_GCONF_KEY_RECOLOR_FOCUSED,
        mssh_gconf_notify_recolor_focused, window, NULL, NULL);
    gconf_client_notify_add(client, MSSH_GCONF_KEY_QUIT_ALL_ENDED,
        mssh_gconf_notify_quit_all_ended, window, NULL, NULL);
    gconf_client_notify_add(client, MSSH_GCONF_KEY_DIR_FOCUS,
        mssh_gconf_notify_dir_focus, window, NULL, NULL);
    gconf_client_notify_add(client, MSSH_GCONF_KEY_MODIFIER,
        mssh_gconf_notify_modifier, window, NULL, NULL);
    gconf_client_notify_add(client, MSSH_GCONF_KEY_BACKSCROLL_BUFFER_SIZE,
        mssh_gconf_backscroll_buffer_size, window, NULL, NULL);

    gconf_client_notify(client, MSSH_GCONF_KEY_COLUMNS);
    gconf_client_notify(client, MSSH_GCONF_KEY_TIMEOUT);
    gconf_client_notify(client, MSSH_GCONF_KEY_CLOSE_ENDED);
    gconf_client_notify(client, MSSH_GCONF_KEY_RECOLOR_FOCUSED);
    gconf_client_notify(client, MSSH_GCONF_KEY_QUIT_ALL_ENDED);
    gconf_client_notify(client, MSSH_GCONF_KEY_DIR_FOCUS);
    gconf_client_notify(client, MSSH_GCONF_KEY_MODIFIER);
    gconf_client_notify(client, MSSH_GCONF_KEY_BACKSCROLL_BUFFER_SIZE);

    gtk_accel_group_connect(accel, GDK_KEY_Up, window->modifier,
        GTK_ACCEL_VISIBLE, g_cclosure_new(
        G_CALLBACK(mssh_window_focus), window, NULL));
    gtk_accel_group_connect(accel, GDK_KEY_Down, window->modifier,
        GTK_ACCEL_VISIBLE, g_cclosure_new(
        G_CALLBACK(mssh_window_focus), window, NULL));
    gtk_accel_group_connect(accel, GDK_KEY_Left, window->modifier,
        GTK_ACCEL_VISIBLE, g_cclosure_new(
        G_CALLBACK(mssh_window_focus), window, NULL));
    gtk_accel_group_connect(accel, GDK_KEY_Right, window->modifier,
        GTK_ACCEL_VISIBLE, g_cclosure_new(
        G_CALLBACK(mssh_window_focus), window, NULL));

    /* bind Ctrl + Shift + x to toggling maximize terminal */
    gtk_accel_group_connect(accel, GDK_KEY_x, GDK_CONTROL_MASK | GDK_SHIFT_MASK,
        GTK_ACCEL_VISIBLE, g_cclosure_new(
        G_CALLBACK(mssh_window_toggle_maximize), window, NULL));

    /* bind Ctrl + Shift + N to show the dialog for adding new sessions */
    gtk_accel_group_connect(accel, GDK_KEY_n, GDK_CONTROL_MASK | GDK_SHIFT_MASK,
        GTK_ACCEL_VISIBLE, g_cclosure_new(
        G_CALLBACK(mssh_window_add), window, NULL));

    window->accel = accel;

    gtk_window_add_accel_group(GTK_WINDOW(window), accel);
}

void mssh_window_start_session(MSSHWindow* window, char **env,
    GArray *hosts, long cols)
{
    int i, j, k;
    int nhosts = hosts->len;
    int rows = (nhosts / 2) + (nhosts % 2);

    window->env = env;
    window->columns_override = cols;

    for(i = 0; i < rows; i++)
    {
        for(j = 0; j < 2; j++)
        {
            k = j + i*2;
            if(k < nhosts)
            {
                mssh_window_add_session(window, g_array_index(hosts,
                    char*, k));
            }
        }
    }

    mssh_window_relayout(window);
}

void mssh_window_add_command(GQuark key_id, gpointer data, gpointer user_data)
{
    GtkWidget *menu_item;
    GtkWidget* window = (GtkWidget *)user_data;

    menu_item = gtk_menu_item_new_with_label(g_quark_to_string (key_id));

    gtk_menu_shell_append(GTK_MENU_SHELL(MSSH_WINDOW(window)->command_menu), menu_item);
    g_signal_connect(G_OBJECT(menu_item), "activate",
        G_CALLBACK(mssh_window_sendcommand), window);
}

static void mssh_window_class_init(MSSHWindowClass *klass)
{
}

void mssh_window_relayout_for_one(MSSHWindow *window, GtkWidget *t)
{

    GConfClient *client;
    GConfEntry *entry;
    int len = window->terminals->len;
    int wcols = window->columns_override ? window->columns_override :
        window->columns;
    int cols = (len < wcols) ? len : wcols;
    int rows = (len + 1) / cols;

    /* get the terminal widget */
    GtkWidget *terminal = GTK_WIDGET(t);

    g_object_ref(terminal);

    /* remove the widget from the container temporarily */
    gtk_container_remove(GTK_CONTAINER(window->grid), GTK_WIDGET(terminal));

    /* add it back again, now resized */
    gtk_grid_attach(GTK_GRID(window->grid), GTK_WIDGET(terminal), 0, 0, cols, rows);

    /* make the terminal focused */
    gtk_window_set_focus(GTK_WINDOW(window), GTK_WIDGET(terminal));

    /* remove the coloring */
    if (window->recolor_focused) {

        client = gconf_client_get_default();

        entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_FG_COLOUR, NULL,
            TRUE, NULL);
        mssh_gconf_notify_fg_colour(client, 0, entry, window);
        entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_BG_COLOUR, NULL,
            TRUE, NULL);
        mssh_gconf_notify_bg_colour(client, 0, entry, window);
    }

    g_object_unref(terminal);

}

gboolean mssh_window_toggle_maximize(GtkWidget *widget, GObject *acceleratable,
    guint keyval, GdkModifierType modifier, gpointer data)
{

    MSSHWindow *window = MSSH_WINDOW(data);

    if (window->is_maximized) {
        /* toggle restore */
        mssh_window_restore_layout(widget, data);
    } else {
        /* toggle maximize */
        mssh_window_maximize(widget, data);
    }
    return TRUE;
}

static void mssh_window_maximize(GtkWidget *widget, gpointer data)
{

    /* find the id of the currently focused window (if any) */
    MSSHWindow *window = MSSH_WINDOW(data);

    int i;
    int idx = -1;
    int len = window->terminals->len;
    GConfClient *client;
    GConfEntry *entry;

    /* get the currently focused window */
    GtkWidget *focus = gtk_window_get_focus(GTK_WINDOW(window));
    /* save the currently focused window so we can restore it later */
    window->last_focus = focus;

    /* find the focused window in the terminal list */
    for(i = 0; i < len; i++)
    {
        if(focus == GTK_WIDGET(g_array_index(window->terminals,
            MSSHTerminal*, i)))
        {
            idx = i;
            break;
        }
    }

    /* recolor the window with the normal color */
    if (window->recolor_focused) {
        client = gconf_client_get_default();

        entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_FG_COLOUR, NULL,
            TRUE, NULL);
        mssh_gconf_notify_fg_colour(client, 0, entry, window);
        entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_BG_COLOUR, NULL,
            TRUE, NULL);
        mssh_gconf_notify_bg_colour(client, 0, entry, window);
    }
    if (idx == -1) {
        /* there's no window focused, do nothing */
    } else {
        /* call relayout, it will reposition the widget to occupy the whole table */
        mssh_window_relayout_for_one(window, GTK_WIDGET(g_array_index(window->terminals,
                    MSSHTerminal*, idx)));
        window->is_maximized = 1;
    }
}

static void mssh_window_restore_layout(GtkWidget *widget, gpointer data)
{

    GConfClient *client;
    GConfEntry *entry;
    /* get the window */
    MSSHWindow *window = MSSH_WINDOW(data);

    /* just call relayout */
    mssh_window_relayout(window);
    window->is_maximized = 0;

    /* restore the focus */
    if (window->last_focus != NULL) {
        gtk_window_set_focus(GTK_WINDOW(window), window->last_focus);
    }

    /* recolor the focused window - if needed */
    client = gconf_client_get_default();
    if (window->recolor_focused && window->is_maximized == 0) {
        entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_FG_COLOUR_FOCUS, NULL,
            TRUE, NULL);
        mssh_gconf_notify_fg_colour_focus(client, 0, entry, window);
        entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_BG_COLOUR_FOCUS, NULL,
            TRUE, NULL);
        mssh_gconf_notify_bg_colour_focus(client, 0, entry, window);
	}
}

/* show a popup window for adding new sessions  */
static void mssh_window_add(GtkWidget *widget, gpointer data)
{

    MSSHWindow *window = MSSH_WINDOW(data);
    GtkWidget *dialog, *label, *content_area, *button_add;
	GtkWidget *new_session_entry;
	gint result;
    
    /* create new dialog */
    dialog = gtk_dialog_new();
    /* get the content area that will be packed */
    content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

    /* label for text */
    label = gtk_label_new (_("Add new session with hostname: "));

    /* Add the label and entry, and show everything we've added to the dialog */
    new_session_entry = gtk_entry_new();
    gtk_entry_set_max_length (GTK_ENTRY(new_session_entry), 255);

    /* pack the widgets */
    gtk_container_add (GTK_CONTAINER (content_area), label);
    gtk_container_add (GTK_CONTAINER (content_area), new_session_entry);
    /* add two buttons */
    button_add = gtk_dialog_add_button(GTK_DIALOG(dialog), _("Add"), GTK_RESPONSE_ACCEPT);
    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Cancel"), GTK_RESPONSE_CANCEL);
    /* make the add button the default */
    gtk_widget_grab_default(button_add);
    /* set dialog properties (modal, etc) */
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_transient_for (GTK_WINDOW(dialog), GTK_WINDOW(window));
    /* set it's title */
    gtk_window_set_title(GTK_WINDOW(dialog), _("Add new session"));

    /* catch the activate signal (hitting enter) */
    g_signal_connect(G_OBJECT(new_session_entry), "activate",
        G_CALLBACK(mssh_window_dialog_emit_response), window);

    /* show the dialog and it's widgets */
    gtk_widget_show_all (dialog);

    /* wait for input */
    result = gtk_dialog_run (GTK_DIALOG (dialog));
    switch (result)
      {
        case GTK_RESPONSE_ACCEPT:
           mssh_window_add_session(window, (gchar*) gtk_entry_get_text(GTK_ENTRY(new_session_entry)));
           /* relayout */
           mssh_window_relayout(window);
           break;
        default:
           /* do nothing */
           break;
      }
    gtk_widget_destroy (dialog);
}

/* catch the 'activate' signal of the entry (return has been pushed)  */
/* emit the response for accept, simulating a mouse click on the add button  */
gboolean mssh_window_dialog_emit_response(GtkWidget *widget, GObject *acceleratable,
    guint keyval, GdkModifierType modifier, gpointer data)
{

    /* get the dialog by getting the parent of the parent for the emitting (entry) widget */
    GtkWidget *vbox = gtk_widget_get_parent(widget);
    GtkWidget *dialog = gtk_widget_get_parent(vbox);
    /* emit the response signal simulating the clicking of 'ok'  */
    gtk_dialog_response (GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);

    return TRUE;
}
