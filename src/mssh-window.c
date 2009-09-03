#include <string.h>
#include <stdlib.h>

#include <gconf/gconf-client.h>
#include <gdk/gdkkeysyms.h>

#include "mssh-terminal.h"
#include "mssh-pref.h"
#include "mssh-gconf.h"
#include "mssh-window.h"

#include "config.h"

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
static void mssh_window_insert(GtkWidget *widget, gchar *new_text,
	gint new_text_length, gint *position, gpointer data);
static void mssh_window_add_session(MSSHWindow *window, char *hostname);
static void mssh_window_init(MSSHWindow* window);
static void mssh_window_class_init(MSSHWindowClass *klass);

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
	MSSHWindow *window = MSSH_WINDOW(data);

	gtk_window_set_title(GTK_WINDOW(window), PACKAGE_NAME" - All");

	return FALSE;
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

	if(idx == -1)
	{
		fprintf(stderr,
			"mssh: Fatal Error: Can't find terminal to remove!\n");
	}
	else
	{
		gtk_widget_destroy(data_pair->terminal->menu_item);

		gtk_container_remove(GTK_CONTAINER(data_pair->window->table),
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

static void mssh_window_session_focused(MSSHTerminal *terminal,
	gpointer data)
{
	char *title;
	size_t len;

	MSSHWindow *window = MSSH_WINDOW(data);

	len = strlen(PACKAGE_NAME" - ") + strlen(terminal->hostname) + 1;
	title = malloc(len);

	snprintf(title, len, PACKAGE_NAME" - %s", terminal->hostname);

	gtk_window_set_title(GTK_WINDOW(window), title);

	free(title);
}

void mssh_window_relayout(MSSHWindow *window)
{
	GConfClient *client;
	GConfEntry *entry;
	int i, len = window->terminals->len;
	int cols = (len < window->columns) ? len : window->columns;
	int rows = (len + 0.5) / cols;

	for(i = 0; i < len; i++)
	{
		MSSHTerminal *terminal = g_array_index(window->terminals,
			MSSHTerminal*, i);

		g_object_ref(terminal);
		if(GTK_WIDGET(terminal)->parent == GTK_WIDGET(window->table))
		{
			gtk_container_remove(GTK_CONTAINER(window->table),
				GTK_WIDGET(terminal));
		}

		gtk_table_attach(GTK_TABLE(window->table), GTK_WIDGET(terminal),
			(i % cols), (i == len - 1) ? cols : (i % cols) + 1, i / cols,
			(i / cols) + 1,
			GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 1, 1);
		g_object_unref(terminal);

		if(!terminal->started)
		{
			mssh_terminal_start_session(terminal, window->env);
			terminal->started = 1;
		}
	}

	if(len > 0)
	{
		gtk_table_resize(GTK_TABLE(window->table), rows, cols);
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
}

static void mssh_window_add_session(MSSHWindow *window, char *hostname)
{
	MSSHTerminal *terminal = MSSH_TERMINAL(mssh_terminal_new());

	g_array_append_val(window->terminals, terminal);

	g_signal_connect(G_OBJECT(terminal), "session-closed",
		G_CALLBACK(mssh_window_session_closed), window);
	g_signal_connect(G_OBJECT(terminal), "session-focused",
		G_CALLBACK(mssh_window_session_focused), window);

	mssh_terminal_init_session(terminal, hostname);

	gtk_menu_shell_append(GTK_MENU_SHELL(window->server_menu),
		terminal->menu_item);
}

static void mssh_window_init(MSSHWindow* window)
{
	GConfClient *client;

	GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
	GtkWidget *entry = gtk_entry_new();

	GtkWidget *menu_bar = gtk_menu_bar_new();
	GtkWidget *file_menu = gtk_menu_new();
	GtkWidget *edit_menu = gtk_menu_new();

	GtkWidget *file_item = gtk_menu_item_new_with_label("File");
	GtkWidget *edit_item = gtk_menu_item_new_with_label("Edit");
	GtkWidget *server_item = gtk_menu_item_new_with_label("Servers");

	GtkWidget *file_quit = gtk_image_menu_item_new_from_stock(
		GTK_STOCK_QUIT, NULL);
	GtkWidget *file_sendhost = gtk_image_menu_item_new_with_label(
		"Send hostname");
/*	GtkWidget *file_add = gtk_image_menu_item_new_with_label(
		"Add session");*/

	GtkWidget *edit_pref = gtk_image_menu_item_new_from_stock(
		GTK_STOCK_PREFERENCES, NULL);

	window->server_menu = gtk_menu_new();

	window->terminals = g_array_new(FALSE, TRUE, sizeof(MSSHTerminal*));

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit_item), edit_menu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(server_item),
		window->server_menu);

/*	gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_add);*/
	gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_sendhost);
	gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_quit);
	gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), edit_pref);
	g_signal_connect(G_OBJECT(file_sendhost), "activate",
		G_CALLBACK(mssh_window_sendhost), window);
	g_signal_connect(G_OBJECT(file_quit), "activate",
		G_CALLBACK(mssh_window_destroy), window);
	g_signal_connect(G_OBJECT(edit_pref), "activate",
		G_CALLBACK(mssh_window_pref), window);

	gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), file_item);
	gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), edit_item);
	gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), server_item);

	g_signal_connect(G_OBJECT(entry), "key-press-event",
		G_CALLBACK(mssh_window_key_press), window);
	g_signal_connect(G_OBJECT(entry), "insert-text",
		G_CALLBACK(mssh_window_insert), window);
	g_signal_connect(G_OBJECT(entry), "focus-in-event",
		G_CALLBACK(mssh_window_entry_focused), window);

	gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, TRUE, 2);

	window->table = gtk_table_new(1, 1, TRUE);
	gtk_box_pack_start(GTK_BOX(vbox), window->table, TRUE, TRUE, 0);

	gtk_container_add(GTK_CONTAINER(window), vbox);

	gtk_widget_set_size_request(GTK_WIDGET(window), 1024, 768);
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
	gconf_client_notify_add(client, MSSH_GCONF_KEY_COLUMNS,
		mssh_gconf_notify_columns, window, NULL, NULL);
	gconf_client_notify_add(client, MSSH_GCONF_KEY_TIMEOUT,
		mssh_gconf_notify_timeout, window, NULL, NULL);
	gconf_client_notify_add(client, MSSH_GCONF_KEY_CLOSE_ENDED,
		mssh_gconf_notify_close_ended, window, NULL, NULL);
	gconf_client_notify_add(client, MSSH_GCONF_KEY_QUIT_ALL_ENDED,
		mssh_gconf_notify_quit_all_ended, window, NULL, NULL);

	gconf_client_notify(client, MSSH_GCONF_KEY_COLUMNS);
	gconf_client_notify(client, MSSH_GCONF_KEY_TIMEOUT);
	gconf_client_notify(client, MSSH_GCONF_KEY_CLOSE_ENDED);
	gconf_client_notify(client, MSSH_GCONF_KEY_QUIT_ALL_ENDED);
}

void mssh_window_start_session(MSSHWindow* window, char **env,
	GArray *hosts)
{
	int i, j, k;
	int nhosts = hosts->len;
	int rows = (nhosts / 2) + (nhosts % 2);

	window->env = env;

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

static void mssh_window_class_init(MSSHWindowClass *klass)
{
}
