#include <gconf/gconf-client.h>

#define GETTEXT_PACKAGE "mssh"
#include <glib/gi18n-lib.h>

#include "mssh-gconf.h"

#include "mssh-pref.h"

static void mssh_pref_init(MSSHPref* pref);
static void mssh_pref_class_init(MSSHPrefClass *klass);

G_DEFINE_TYPE(MSSHPref, mssh_pref, GTK_TYPE_WINDOW)

GtkWidget* mssh_pref_new(void)
{
    return g_object_new(MSSH_TYPE_PREF, NULL);
}

static void mssh_pref_close(GtkWidget *widget, gpointer data)
{
    MSSHPref *pref = MSSH_PREF(data);

    gtk_widget_destroy(GTK_WIDGET(pref));
}

static void mssh_pref_font_select(GtkWidget *widget, gpointer data)
{
    GConfClient *client;
    const gchar *font;

    client = gconf_client_get_default();

    font = gtk_font_button_get_font_name(GTK_FONT_BUTTON(widget));

    gconf_client_set_string(client, MSSH_GCONF_KEY_FONT, font, NULL);
}

static void mssh_pref_fg_colour_select(GtkWidget *widget, gpointer data)
{
    GConfClient *client;
    GdkRGBA colour;
    const gchar *colour_s;

    client = gconf_client_get_default();

    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(widget), &colour);
    colour_s = gdk_rgba_to_string(&colour);

    gconf_client_set_string(client, MSSH_GCONF_KEY_FG_COLOUR, colour_s,
        NULL);
}

static void mssh_pref_bg_colour_select(GtkWidget *widget, gpointer data)
{
    GConfClient *client;
    GdkRGBA colour;
    const gchar *colour_s;

    client = gconf_client_get_default();

    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(widget), &colour);
    colour_s = gdk_rgba_to_string(&colour);

    gconf_client_set_string(client, MSSH_GCONF_KEY_BG_COLOUR, colour_s,
        NULL);
}

static void mssh_pref_fg_colour_select_focus(GtkWidget *widget, gpointer data)
{
    GConfClient *client;
    GdkRGBA colour;
    const gchar *colour_s;

    client = gconf_client_get_default();

    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(widget), &colour);
    colour_s = gdk_rgba_to_string(&colour);


    gconf_client_set_string(client, MSSH_GCONF_KEY_FG_COLOUR_FOCUS, colour_s,
        NULL);
}

static void mssh_pref_bg_colour_select_focus(GtkWidget *widget, gpointer data)
{
    GConfClient *client;
    GdkRGBA colour;
    const gchar *colour_s;

    client = gconf_client_get_default();

    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(widget), &colour);
    colour_s = gdk_rgba_to_string(&colour);

    gconf_client_set_string(client, MSSH_GCONF_KEY_BG_COLOUR_FOCUS, colour_s,
        NULL);
}
static void mssh_pref_columns_select(GtkWidget *widget, gpointer data)
{
    GConfClient *client;
    int columns;

    client = gconf_client_get_default();

    columns = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));

    gconf_client_set_int(client, MSSH_GCONF_KEY_COLUMNS, columns, NULL);
}

static void mssh_pref_backscroll_buffer_size_select(GtkWidget *widget, gpointer data)
{
    GConfClient *client;
    gint backscroll_buffer_size;

    client = gconf_client_get_default();

    backscroll_buffer_size = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));

    gconf_client_set_int(client, MSSH_GCONF_KEY_BACKSCROLL_BUFFER_SIZE, backscroll_buffer_size, NULL);
}
static void mssh_pref_timeout_select(GtkWidget *widget, gpointer data)
{
    GConfClient *client;
    int timeout;

    client = gconf_client_get_default();

    timeout = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));

    gconf_client_set_int(client, MSSH_GCONF_KEY_TIMEOUT, timeout, NULL);
}

static void mssh_pref_close_check(GtkWidget *widget, gpointer data)
{
    GConfClient *client;
    gboolean close;

    client = gconf_client_get_default();

    close = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    gconf_client_set_bool(client, MSSH_GCONF_KEY_CLOSE_ENDED, close, NULL);
}

static void mssh_pref_recolor_focused_check(GtkWidget *widget, gpointer data)
{
    GConfClient *client;
    gboolean recolor_focused;

    client = gconf_client_get_default();

    recolor_focused = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    gconf_client_set_bool(client, MSSH_GCONF_KEY_RECOLOR_FOCUSED, recolor_focused, NULL);
}
static void mssh_pref_exit_check(GtkWidget *widget, gpointer data)
{
    GConfClient *client;
    gboolean close;

    client = gconf_client_get_default();

    close = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    gconf_client_set_bool(client, MSSH_GCONF_KEY_QUIT_ALL_ENDED, close,
        NULL);
}

static void mssh_pref_dir_focus_check(GtkWidget *widget, gpointer data)
{
    GConfClient *client;
    gboolean focus;

    client = gconf_client_get_default();

    focus = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    gconf_client_set_bool(client, MSSH_GCONF_KEY_DIR_FOCUS, focus, NULL);
}

static void mssh_pref_modifier_check(GtkWidget *widget, gpointer data)
{
    GConfClient *client;
    gboolean ctrl, alt, shift, super;
    gint val;

    MSSHPref * pref = MSSH_PREF(data);

    client = gconf_client_get_default();

    ctrl = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pref->ctrl));
    shift = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pref->shift));
    alt = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pref->alt));
    super = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pref->super));

    val = (shift << 0) | (ctrl << 2) | (alt << 3) | (super << 26);

    gconf_client_set_int(client, MSSH_GCONF_KEY_MODIFIER, val, NULL);
}

static void mssh_pref_init(MSSHPref* pref)
{
    GConfClient *client;
    GConfEntry *entry;
    GConfValue *value;
    GdkRGBA colour;
    const gchar *colour_s;

    GtkWidget *frame = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *notebook = gtk_notebook_new();
    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);

    GtkWidget *font_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *font_label = gtk_label_new(_("Font:"));
    GtkWidget *font_select = gtk_font_button_new();

    GtkWidget *colour_table = gtk_grid_new();
    GtkWidget *bg_colour_label = gtk_label_new(_("Background:"));
    GtkWidget *bg_colour_select = gtk_color_button_new();
    GtkWidget *fg_colour_label = gtk_label_new(_("Foreground:"));
    GtkWidget *fg_colour_select = gtk_color_button_new();

    GtkWidget *recolor_focused_check = gtk_check_button_new_with_label(
        _("Use different color for focused window"));

    GtkWidget *colour_table_focus = gtk_grid_new();
    GtkWidget *bg_colour_label_focus = gtk_label_new(_("Background for focused window:"));
    GtkWidget *bg_colour_select_focus = gtk_color_button_new();
    GtkWidget *fg_colour_label_focus = gtk_label_new(_("Foreground for focused window:"));
    GtkWidget *fg_colour_select_focus = gtk_color_button_new();
    GtkWidget *exit_check = gtk_check_button_new_with_label(
        _("Quit after all sessions have ended"));
    GtkWidget *close_check = gtk_check_button_new_with_label(
        _("Close ended sessions"));

    GtkWidget *timeout_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *timeout_label1 = gtk_label_new(
        _("Closed ended sessions after"));
    GtkAdjustment *timeout_adj = gtk_adjustment_new(3, 0, 100, 1, 10, 0);
    GtkWidget *timeout_select = gtk_spin_button_new(
        GTK_ADJUSTMENT(timeout_adj), 1, 0);
    GtkWidget *timeout_label2 = gtk_label_new(_("seconds"));

    GtkWidget *columns_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *columns_label = gtk_label_new(_("Columns:"));
    GtkAdjustment *columns_adj = gtk_adjustment_new(2, 1, 10, 1, 10, 0);
    GtkWidget *columns_select = gtk_spin_button_new(
        GTK_ADJUSTMENT(columns_adj), 1, 0);

    GtkWidget *backscroll_buffer_size_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *backscroll_buffer_size_label = gtk_label_new(_("Scrollback Lines:"));
    GtkAdjustment *backscroll_buffer_size_adj = gtk_adjustment_new(5000, -1, 65535, 1, 100, 0);
    GtkWidget *backscroll_buffer_size_select = gtk_spin_button_new(
        GTK_ADJUSTMENT(backscroll_buffer_size_adj), 1, 0);  
    GtkWidget *mod_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *mod_label = gtk_label_new(_("Modifier:"));
    GtkWidget *mod_ctrl_check = gtk_check_button_new_with_label("Ctrl");
    GtkWidget *mod_alt_check = gtk_check_button_new_with_label("Alt");
    GtkWidget *mod_shift_check = gtk_check_button_new_with_label("Shift");
    GtkWidget *mod_super_check = gtk_check_button_new_with_label("Super");

    GtkWidget *close_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *close_button = gtk_button_new_with_label("Close");

    GtkWidget *dir_focus_check = gtk_check_button_new_with_label(
        _("Use directional focus"));

    pref->ctrl = mod_ctrl_check;
    pref->shift = mod_shift_check;
    pref->alt = mod_alt_check;
    pref->super = mod_super_check;

    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), content, NULL);

    gtk_box_pack_start(GTK_BOX(font_hbox), font_label, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(font_hbox), font_select, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content), font_hbox, FALSE, TRUE, 0);

    gtk_grid_attach(GTK_GRID(colour_table), bg_colour_label,
        0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(colour_table), bg_colour_select,
        1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(colour_table), fg_colour_label,
        0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(colour_table), fg_colour_select,
        1, 1, 1, 1);
    gtk_box_pack_start(GTK_BOX(content), colour_table, FALSE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(content), recolor_focused_check, FALSE, TRUE, 0);

    gtk_grid_attach(GTK_GRID(colour_table_focus), bg_colour_label_focus,
        0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(colour_table_focus), bg_colour_select_focus,
        1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(colour_table_focus), fg_colour_label_focus,
        0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(colour_table_focus), fg_colour_select_focus,
        1, 1, 1, 1);
    gtk_box_pack_start(GTK_BOX(content), colour_table_focus, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content), exit_check, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content), close_check, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content), dir_focus_check, FALSE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(timeout_hbox), timeout_label1, FALSE,
        TRUE, 0);
    gtk_box_pack_start(GTK_BOX(timeout_hbox), timeout_select, FALSE,
        TRUE, 0);
    gtk_box_pack_start(GTK_BOX(timeout_hbox), timeout_label2, FALSE,
        TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content), timeout_hbox, FALSE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(columns_hbox), columns_label, FALSE,
        TRUE, 0);
    gtk_box_pack_start(GTK_BOX(columns_hbox), columns_select, FALSE,
        TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content), columns_hbox, FALSE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(backscroll_buffer_size_hbox), backscroll_buffer_size_label, FALSE,
        TRUE, 0);
    gtk_box_pack_start(GTK_BOX(backscroll_buffer_size_hbox), backscroll_buffer_size_select, FALSE,
        TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content), backscroll_buffer_size_hbox, FALSE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(mod_hbox), mod_label, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(mod_hbox), mod_ctrl_check, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(mod_hbox), mod_alt_check, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(mod_hbox), mod_shift_check, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(mod_hbox), mod_super_check, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content), mod_hbox, FALSE, TRUE, 0);

    gtk_box_pack_end(GTK_BOX(close_hbox), close_button, FALSE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(frame), notebook, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(frame), close_hbox, FALSE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(pref), frame);

    g_signal_connect(G_OBJECT(close_button), "clicked",
        G_CALLBACK(mssh_pref_close), pref);

    gtk_container_set_border_width(GTK_CONTAINER(content), 6);
    gtk_container_set_border_width(GTK_CONTAINER(pref), 15);
    gtk_window_set_title(GTK_WINDOW(pref), _("Preferences"));
    gtk_window_set_resizable(GTK_WINDOW(pref), FALSE);

    g_signal_connect(G_OBJECT(font_select), "font-set",
        G_CALLBACK(mssh_pref_font_select), NULL);
    g_signal_connect(G_OBJECT(fg_colour_select), "color-set",
        G_CALLBACK(mssh_pref_fg_colour_select), NULL);
    g_signal_connect(G_OBJECT(bg_colour_select), "color-set",
        G_CALLBACK(mssh_pref_bg_colour_select), NULL);
    g_signal_connect(G_OBJECT(fg_colour_select_focus), "color-set",
        G_CALLBACK(mssh_pref_fg_colour_select_focus), NULL);
    g_signal_connect(G_OBJECT(bg_colour_select_focus), "color-set",
        G_CALLBACK(mssh_pref_bg_colour_select_focus), NULL);
    g_signal_connect(G_OBJECT(columns_select), "value-changed",
        G_CALLBACK(mssh_pref_columns_select), NULL);
    g_signal_connect(G_OBJECT(backscroll_buffer_size_select), "value-changed",
        G_CALLBACK(mssh_pref_backscroll_buffer_size_select), NULL);    
    g_signal_connect(G_OBJECT(timeout_select), "value-changed",
        G_CALLBACK(mssh_pref_timeout_select), NULL);
    g_signal_connect(G_OBJECT(close_check), "toggled",
        G_CALLBACK(mssh_pref_close_check), NULL);
    g_signal_connect(G_OBJECT(recolor_focused_check), "toggled",
        G_CALLBACK(mssh_pref_recolor_focused_check), NULL);
    g_signal_connect(G_OBJECT(exit_check), "toggled",
        G_CALLBACK(mssh_pref_exit_check), NULL);
     g_signal_connect(G_OBJECT(dir_focus_check), "toggled",
        G_CALLBACK(mssh_pref_dir_focus_check), NULL);
    g_signal_connect(G_OBJECT(mod_ctrl_check), "toggled",
        G_CALLBACK(mssh_pref_modifier_check), pref);
    g_signal_connect(G_OBJECT(mod_alt_check), "toggled",
        G_CALLBACK(mssh_pref_modifier_check), pref);
    g_signal_connect(G_OBJECT(mod_shift_check), "toggled",
        G_CALLBACK(mssh_pref_modifier_check), pref);
    g_signal_connect(G_OBJECT(mod_super_check), "toggled",
        G_CALLBACK(mssh_pref_modifier_check), pref);

    client = gconf_client_get_default();

    entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_FONT, NULL,
        TRUE, NULL);
    value = gconf_entry_get_value(entry);
    gtk_font_button_set_font_name(GTK_FONT_BUTTON(font_select),
        gconf_value_get_string(value));

    entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_FG_COLOUR, NULL,
        TRUE, NULL);
    value = gconf_entry_get_value(entry);
    colour_s = gconf_value_get_string(value);
    gdk_rgba_parse(&colour, colour_s);
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(fg_colour_select), 
        &colour);

    entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_BG_COLOUR, NULL,
        TRUE, NULL);
    value = gconf_entry_get_value(entry);
    colour_s = gconf_value_get_string(value);
    gdk_rgba_parse(&colour, colour_s);
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(bg_colour_select),
        &colour);

    entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_FG_COLOUR_FOCUS, NULL,
        TRUE, NULL);
    value = gconf_entry_get_value(entry);
    colour_s = gconf_value_get_string(value);
    gdk_rgba_parse(&colour, colour_s);
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(fg_colour_select_focus),
        &colour);

    entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_BG_COLOUR_FOCUS, NULL,
        TRUE, NULL);
    value = gconf_entry_get_value(entry);
    colour_s = gconf_value_get_string(value);
    gdk_rgba_parse(&colour, colour_s);
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(bg_colour_select_focus),
        &colour);

    entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_COLUMNS, NULL,
        TRUE, NULL);
    value = gconf_entry_get_value(entry);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(columns_select),
        gconf_value_get_int(value));

    entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_BACKSCROLL_BUFFER_SIZE, NULL,
        TRUE, NULL);
    value = gconf_entry_get_value(entry);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(backscroll_buffer_size_select),
        gconf_value_get_int(value));

    entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_TIMEOUT, NULL,
        TRUE, NULL);
    value = gconf_entry_get_value(entry);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(timeout_select),
        gconf_value_get_int(value));

    entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_CLOSE_ENDED,
        NULL, TRUE, NULL);
    value = gconf_entry_get_value(entry);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(close_check),
        gconf_value_get_bool(value));

    entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_RECOLOR_FOCUSED,
        NULL, TRUE, NULL);
    value = gconf_entry_get_value(entry);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(recolor_focused_check),
        gconf_value_get_bool(value));

    entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_QUIT_ALL_ENDED,
            NULL, TRUE, NULL);
    value = gconf_entry_get_value(entry);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(exit_check),
        gconf_value_get_bool(value));

    entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_DIR_FOCUS,
            NULL, TRUE, NULL);
    value = gconf_entry_get_value(entry);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dir_focus_check),
        gconf_value_get_bool(value));

    entry = gconf_client_get_entry(client, MSSH_GCONF_KEY_MODIFIER,
            NULL, TRUE, NULL);
    value = gconf_entry_get_value(entry);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mod_ctrl_check),
        (gconf_value_get_int(value) & GDK_CONTROL_MASK) ? 1 : 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mod_shift_check),
        (gconf_value_get_int(value) & GDK_SHIFT_MASK) ? 1 : 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mod_alt_check),
        (gconf_value_get_int(value) & GDK_MOD1_MASK) ? 1 : 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mod_super_check),
        (gconf_value_get_int(value) & GDK_SUPER_MASK) ? 1 : 0);
}

static void mssh_pref_class_init(MSSHPrefClass *klass)
{
}
