#include <string.h>
#include <stdlib.h>

#include <gdk/gdkkeysyms.h>

#include "mssh-window.h"

G_DEFINE_TYPE(MSSHWindow, mssh_window, GTK_TYPE_WINDOW)

static void mssh_window_sendhost(GtkWidget *widget, gpointer data)
{
	int i;

	MSSHWindow *window = MSSH_WINDOW(data);

	for(i = 0; i < window->num_servers; i++)
	{
		if(window->terms[i] != NULL)
		{
			if(gtk_check_menu_item_get_active(
				GTK_CHECK_MENU_ITEM(window->items[i])))
			{
				vte_terminal_feed_child(VTE_TERMINAL(window->terms[i]),
					window->servers[i], strlen(window->servers[i]));
			}
		}
	}
}

static void mssh_window_destroy(GtkWidget *widget, gpointer data)
{
	MSSHWindow *window = MSSH_WINDOW(data);

	free(window->terms);
	free(window->items);
	gtk_main_quit();
}

GtkWidget* mssh_window_new(void)
{
	return g_object_new(MSSH_TYPE_WINDOW, NULL);
}

gboolean key_press(GtkWidget *widget, GdkEventKey *event,
	gpointer user_data)
{
	int i;
	gboolean dummy;

	MSSHWindow *window = MSSH_WINDOW(user_data);

	for(i = 0; i < window->num_servers; i++)
	{
		if(window->terms[i] != NULL)
		{
			if(gtk_check_menu_item_get_active(
				GTK_CHECK_MENU_ITEM(window->items[i])))
			{
				g_signal_emit_by_name(window->terms[i], "key-press-event",
					event, &dummy);
			}
		}
	}

	return TRUE;
}

void vte_child_exited(VteTerminal *vte, gpointer user_data)
{
	int i;
	char data[] = "\n[Child Exited]";
	MSSHWindow *window = MSSH_WINDOW(user_data);
	vte_terminal_feed(vte, data, strlen(data));

	for(i = 0; i < window->num_servers; i++)
	{
		if(window->terms[i] == GTK_WIDGET(vte))
		{
			window->terms[i] = NULL;
			break;
		}
	}
}

static void mssh_window_init(MSSHWindow* window)
{
	GtkAccelGroup *accel_group;

	accel_group = gtk_accel_group_new();
	window->vbox = gtk_vbox_new(FALSE, 0);
	window->entry = gtk_entry_new();
	window->menu_bar = gtk_menu_bar_new();
	window->server_menu = gtk_menu_new();
	window->file_menu = gtk_menu_new();
	window->server_item = gtk_menu_item_new_with_label("Servers");
	window->file_item = gtk_menu_item_new_with_label("File");
	window->file_quit = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT,
		NULL);
	window->file_sendhost = gtk_image_menu_item_new_with_label(
		"Send hostname");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(window->file_item),
		window->file_menu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(window->server_item),
		window->server_menu);

	gtk_menu_shell_append(GTK_MENU_SHELL(window->file_menu),
		window->file_sendhost);
	gtk_menu_shell_append(GTK_MENU_SHELL(window->file_menu),
		window->file_quit);
	g_signal_connect(G_OBJECT(window->file_sendhost), "activate",
		G_CALLBACK(mssh_window_sendhost), window);
	g_signal_connect(G_OBJECT(window->file_quit), "activate",
		G_CALLBACK(mssh_window_destroy), window);
	gtk_widget_add_accelerator(window->file_quit, "activate", accel_group,
		GDK_W, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

	gtk_menu_bar_append(GTK_MENU_BAR(window->menu_bar),
		window->file_item);
	gtk_menu_bar_append(GTK_MENU_BAR(window->menu_bar),
		window->server_item);

	g_signal_connect(G_OBJECT(window->entry), "key-press-event",
		G_CALLBACK(key_press), window);

	gtk_box_pack_start(GTK_BOX(window->vbox), window->menu_bar,
		FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(window->vbox), window->entry,
		FALSE, TRUE, 2);

	gtk_container_add(GTK_CONTAINER(window), window->vbox);

	gtk_widget_set_size_request(GTK_WIDGET(window), 1024, 768);
	gtk_window_set_title(GTK_WINDOW(window), "MSSH");
}

void mssh_window_new_session(MSSHWindow* window, char **env,
	int num_servers, char **servers)
{
	char *args[3] = { NULL, NULL, NULL };
	int i, j, k;
	int rows = num_servers/2 + num_servers%2;

	window->env = env;
	window->num_servers = num_servers;
	window->servers = servers;

	window->items = malloc(sizeof(GtkWidget) * num_servers);
	window->terms = malloc(sizeof(GtkWidget) * num_servers);
	memset(window->items, 0, sizeof(GtkWidget) * num_servers);
	memset(window->terms, 0, sizeof(GtkWidget) * num_servers);

	args[0] = strdup("ssh");

	window->table = gtk_table_new(rows, 2, TRUE);
	gtk_box_pack_start(GTK_BOX(window->vbox), window->table,
		TRUE, TRUE, 0);

	for(i = 0; i < rows; i++)
	{
		for(j = 0; j < 2; j++)
		{
			k = j + i*2;
			if(k < num_servers)
			{
				args[1] = window->servers[k];
				window->terms[k] = vte_terminal_new();
				g_signal_connect(G_OBJECT(window->terms[k]),
					"child-exited", G_CALLBACK(vte_child_exited), window);
				vte_terminal_fork_command(VTE_TERMINAL(window->terms[k]),
					"ssh", args, window->env, NULL, FALSE, FALSE,
					FALSE);
				if((k == num_servers - 1) && (num_servers % 2 == 1))
				{
					gtk_table_attach(GTK_TABLE(window->table),
						window->terms[k], j, j+2, i, i+1,
						GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 2,
						2);
				}
				else
				{
					gtk_table_attach(GTK_TABLE(window->table),
						window->terms[k], j, j+1, i, i+1,
						GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 2,
						2);
				}

				window->items[k] = gtk_check_menu_item_new_with_label(
					window->servers[k]);
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
					window->items[k]), TRUE);
				gtk_menu_shell_append(GTK_MENU_SHELL(window->server_menu),
					window->items[k]);
			}
		}
	}

	free(args[0]);
}

static void mssh_window_class_init(MSSHWindowClass *klass)
{
}
