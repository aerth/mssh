#include <string.h>
#include <stdlib.h>

#include "mssh-terminal.h"

G_DEFINE_TYPE(MSSHTerminal, mssh_terminal, VTE_TYPE_TERMINAL)

static void mssh_terminal_init(MSSHTerminal* terminal);
static void mssh_terminal_class_init(MSSHTerminalClass *klass);
static void mssh_terminal_child_exited(VteTerminal *vte, int pid, gpointer data);
static gboolean mssh_terminal_focused(GtkWidget *widget,
    GtkDirectionType dir, gpointer data);

GtkWidget* mssh_terminal_new()
{
    return g_object_new(MSSH_TYPE_TERMINAL, NULL);
}

void mssh_terminal_destroy(MSSHTerminal *terminal)
{
    free(terminal->hostname);
}

gboolean mssh_terminal_isactive(MSSHTerminal *terminal)
{
    return gtk_check_menu_item_get_active(
        GTK_CHECK_MENU_ITEM(terminal->menu_item));
}

void mssh_terminal_set_backscroll_size(MSSHTerminal *terminal, gint *backscroll_buffer_size)
{
    /* use scrollback buffer from gconf*/
    vte_terminal_set_scrollback_lines(VTE_TERMINAL(terminal), *backscroll_buffer_size);
}
void mssh_terminal_init_session(MSSHTerminal *terminal, char *hostname)
{
    terminal->hostname = hostname;
    /* use scrollback buffer from gconf*/
    mssh_terminal_set_backscroll_size(terminal, &terminal->backscroll_buffer_size);

    terminal->menu_item = gtk_check_menu_item_new_with_label(
        terminal->hostname);

    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
        terminal->menu_item), TRUE);
}

void mssh_terminal_start_session(MSSHTerminal *terminal, char **env)
{
    char *args[5];
    char *fullhost;
    char *host = NULL;
    char *port = NULL;

    fullhost = strdup(terminal->hostname);
    host = strtok(fullhost, ":");
    port = strtok(NULL, "");

    args[0] = strdup("ssh");
    args[1] = host;

    if (!port)
        args[2] = NULL;
    else {
         args[2] = strdup("-p");
         args[3] = port;
         args[4] = NULL;
    }

    vte_terminal_spawn_sync(VTE_TERMINAL(terminal),
                            VTE_PTY_NO_LASTLOG|VTE_PTY_NO_UTMP|VTE_PTY_NO_WTMP,
                            NULL,  /* working dir */
                            args,
                            env,
                            G_SPAWN_SEARCH_PATH,
                            NULL,  /* child_setup */
                            NULL,  /* child_setup_data */
                            NULL,  /* *child_pid */
                            NULL,  /* *cancellable */
                            NULL); /* Error handling */

    free(args[0]);
}

void mssh_terminal_send_host(MSSHTerminal *terminal)
{
    if(mssh_terminal_isactive(terminal))
    {
        vte_terminal_feed_child(VTE_TERMINAL(terminal),
            terminal->hostname, strlen(terminal->hostname));
    }
}

void mssh_terminal_send_string(MSSHTerminal *terminal, gchar *string)
{
    if(mssh_terminal_isactive(terminal))
    {
        vte_terminal_feed_child(VTE_TERMINAL(terminal), string,
            strlen(string));
    }
}

void mssh_terminal_send_data(MSSHTerminal *terminal, GdkEventKey *event)
{
    gboolean dummy;

    if(mssh_terminal_isactive(terminal))
    {
        g_signal_emit_by_name(terminal, "key-press-event", event, &dummy);
    }
}

static void mssh_terminal_init(MSSHTerminal* terminal)
{
    terminal->started = 0;
    terminal->ended = 0;

    g_signal_connect(G_OBJECT(terminal), "child-exited",
        G_CALLBACK(mssh_terminal_child_exited), terminal);
    g_signal_connect(G_OBJECT(terminal), "focus-in-event",
        G_CALLBACK(mssh_terminal_focused), terminal);
}

static void mssh_terminal_class_init(MSSHTerminalClass *klass)
{
    klass->session_closed_signal = g_signal_new("session-closed",
        G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST,
        G_STRUCT_OFFSET(MSSHTerminalClass, session_closed), NULL, NULL,
        g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, NULL);

    klass->session_focused_signal = g_signal_new("session-focused",
        G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST,
        G_STRUCT_OFFSET(MSSHTerminalClass, session_focused), NULL, NULL,
        g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, NULL);
}

static void mssh_terminal_child_exited(VteTerminal *vte, int pid, gpointer data)
{
    char msg[] = "\r\n[Child Exited]\r\n";

    MSSHTerminal *terminal = MSSH_TERMINAL(data);

    terminal->ended = 1;

    vte_terminal_feed(vte, msg, strlen(msg));

    g_signal_emit_by_name(terminal, "session-closed");
}

static gboolean mssh_terminal_focused(GtkWidget *widget,
    GtkDirectionType dir, gpointer data)
{
    MSSHTerminal *terminal = MSSH_TERMINAL(data);

    g_signal_emit_by_name(terminal, "session-focused");

    return FALSE;
}
