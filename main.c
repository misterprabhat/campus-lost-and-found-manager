/* Campus Lost & Found Manager - Dashboard Edition (Responsive & Scaled)
   GTK 4 interface + C structures, arrays, functions and file handling.

   Program flow:
     main()  ->  build_ui()  ->  load_items()  ->  refresh_list()
     Every click changes the items[] array, saves it to a file,
     then calls refresh_list() to redraw the whole dashboard.
*/
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(dir) _mkdir(dir)
#else
#include <sys/stat.h>
#define MKDIR(dir) mkdir(dir, 0755)
#endif

#define MAX_ITEMS 500
#define FILE_NAME "data/items.txt"

/* 1 = fill an empty list with demo reports on start-up, 0 = start empty */
#define LOAD_SAMPLE_DATA 1

/* One lost/found report */
typedef struct {
    int id;
    char name[80];
    char category[40];
    char location[80];
    char status[20];     /* "Lost", "Found" or "Claimed" */
} Item;

/* All reports are kept in this array while the program runs */
static Item items[MAX_ITEMS];
static int item_count = 0;

/* Current filters (changed from the sidebar) */
static char current_filter[20] = "All";
static char current_category[40] = "All";

/* Names used by the sidebar, the chart and the "add report" form */
static const char *nav_statuses[4]    = {"All", "Lost", "Found", "Claimed"};
static const char *nav_categories[6]  = {"All", "Electronics", "Accessories", "Books", "Documents", "Other"};
static const char *form_categories[6] = {"Electronics", "Accessories", "Books", "Documents", "Other", NULL};
static const char *form_statuses[3]   = {"Lost", "Found", NULL};
static const char *category_icons[5]  = {"💻", "🎒", "📚", "📄", "📦"};

/* Demo data, used only when there are no saved reports yet */
static const Item sample_items[] = {
    {1,  "Black Casio fx-991 Calculator", "Electronics", "Engineering Block, Room 204", "Lost"},
    {2,  "Blue Nike Backpack",            "Accessories", "Central Library, Reading Hall", "Found"},
    {3,  "Data Structures by Cormen",     "Books",       "Canteen, corner table",         "Claimed"},
    {4,  "Student ID Card (Aarav S.)",    "Documents",   "Main Gate security desk",       "Found"},
    {5,  "White AirPods Case",            "Electronics", "Sports Complex, gym",           "Lost"},
    {6,  "Silver Fossil Wrist Watch",     "Accessories", "Auditorium, Row C",             "Lost"},
    {7,  "Samsung Galaxy M31 Phone",      "Electronics", "Computer Lab 3",                "Claimed"},
    {8,  "Physics Notebook, Unit 4",      "Books",       "Lecture Hall B",                "Found"},
    {9,  "Steel Water Bottle",            "Accessories", "Hostel Block A lobby",          "Found"},
    {10, "Fee Receipt Folder",            "Documents",   "Admin Office counter",          "Lost"},
    {11, "Boat Wired Headphones",         "Electronics", "Seminar Hall",                  "Found"},
    {12, "Red Folding Umbrella",          "Accessories", "Bus stop near Gate 2",          "Claimed"},
    {13, "Engineering Mathematics Book",  "Books",       "Library, second floor",         "Lost"}
};

/* Totals per status, filled by update_stats() and read by the gauge */
static int lost_total = 0, found_total = 0, claimed_total = 0;

/* Widgets that must be reached from several functions */
static GtkWidget *window, *add_dialog;
static GtkWidget *list_box, *search_entry, *empty_box, *list_title_label, *shown_label;
static GtkWidget *name_entry, *location_entry, *category_drop, *status_drop;
static GtkWidget *stat_number[4], *stat_note[4];
static GtkWidget *status_nav[4], *status_nav_count[4], *category_nav[6];
static GtkWidget *bar_widget[5], *bar_count[5];
static GtkWidget *gauge_area, *gauge_percent, *legend_text[3];

/* ------------------------------------------------------------------ */
/* File handling                                                       */
/* ------------------------------------------------------------------ */

static void ensure_data_dir(void) {
    MKDIR("data");   /* does nothing if the folder already exists */
}

static void save_items(void) {
    ensure_data_dir();
    FILE *file = fopen(FILE_NAME, "w");
    if (!file) {
        g_warning("Cannot write %s. Run the app from its project folder.", FILE_NAME);
        return;
    }
    for (int i = 0; i < item_count; i++) {
        fprintf(file, "%d\t%s\t%s\t%s\t%s\n",
                items[i].id, items[i].name, items[i].category,
                items[i].location, items[i].status);
    }
    fclose(file);
}

static void load_items(void) {
    FILE *file = fopen(FILE_NAME, "r");
    if (!file) return;   /* first run: no file yet */

    char line[400];
    while (item_count < MAX_ITEMS && fgets(line, sizeof(line), file)) {
        Item item = {0};
        if (sscanf(line, "%d\t%79[^\t]\t%39[^\t]\t%79[^\t]\t%19[^\r\n]",
                   &item.id, item.name, item.category,
                   item.location, item.status) == 5) {
            items[item_count++] = item;
        }
    }
    fclose(file);
}

/* Copy the demo reports into the array and save them */
static void load_sample_items(void) {
    int total = (int)(sizeof(sample_items) / sizeof(sample_items[0]));
    for (int i = 0; i < total && item_count < MAX_ITEMS; i++) {
        items[item_count++] = sample_items[i];
    }
    save_items();
}

/* ------------------------------------------------------------------ */
/* Small UI helpers                                                    */
/* ------------------------------------------------------------------ */

static GtkWidget *label_new(const char *text, const char *css_class) {
    GtkWidget *label = gtk_label_new(text);
    gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
    if (css_class) gtk_widget_add_css_class(label, css_class);
    return label;
}

static GtkWidget *make_button(const char *text, const char *css_class) {
    GtkWidget *button = gtk_button_new_with_label(text);
    if (css_class) gtk_widget_add_css_class(button, css_class);
    gtk_widget_set_cursor_from_name(button, "pointer");
    return button;
}

static const char *get_category_icon(const char *category) {
    for (int i = 0; i < 5; i++) {
        if (!strcmp(category, form_categories[i])) return category_icons[i];
    }
    return category_icons[4];
}

static int percent_of(int part, int total) {
    return (total > 0) ? (part * 100) / total : 0;
}

/* Small pop-up window used for error messages */
static void show_message(GtkWidget *parent, const char *title_text, const char *message) {
    GtkWidget *dialog = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Campus Lost & Found Manager");
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(parent));
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 230);
    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_add_css_class(box, "dialog-box");

    GtkWidget *title = label_new(title_text, "dialog-title");
    GtkWidget *body = label_new(message, "dialog-text");
    gtk_label_set_wrap(GTK_LABEL(body), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(body), 48);

    GtkWidget *ok = make_button("Got it", "primary-button");
    gtk_widget_set_halign(ok, GTK_ALIGN_END);
    gtk_widget_set_margin_top(ok, 12);
    g_signal_connect_swapped(ok, "clicked", G_CALLBACK(gtk_window_destroy), dialog);

    gtk_box_append(GTK_BOX(box), title);
    gtk_box_append(GTK_BOX(box), body);
    gtk_box_append(GTK_BOX(box), ok);

    gtk_window_set_child(GTK_WINDOW(dialog), box);
    gtk_window_present(GTK_WINDOW(dialog));
}

/* ------------------------------------------------------------------ */
/* Statistics, chart and gauge                                         */
/* ------------------------------------------------------------------ */

static void set_number(GtkWidget *label, int value) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", value);
    gtk_label_set_text(GTK_LABEL(label), buf);
}

static void set_note(GtkWidget *label, const char *format, int value) {
    char buf[48];
    snprintf(buf, sizeof(buf), format, value);
    gtk_label_set_text(GTK_LABEL(label), buf);
}

static void update_stats(void) {
    int category_counts[5] = {0, 0, 0, 0, 0};
    lost_total = found_total = claimed_total = 0;

    for (int i = 0; i < item_count; i++) {
        if (!strcmp(items[i].status, "Lost"))         lost_total++;
        else if (!strcmp(items[i].status, "Found"))   found_total++;
        else if (!strcmp(items[i].status, "Claimed")) claimed_total++;

        int c = 4;   /* anything unknown counts as "Other" */
        for (int j = 0; j < 5; j++) {
            if (!strcmp(items[i].category, form_categories[j])) c = j;
        }
        category_counts[c]++;
    }

    /* Four stat cards */
    set_number(stat_number[0], item_count);
    set_number(stat_number[1], lost_total);
    set_number(stat_number[2], found_total);
    set_number(stat_number[3], claimed_total);

    char buf[64];
    snprintf(buf, sizeof(buf), "%d open  ·  %d resolved", lost_total + found_total, claimed_total);
    gtk_label_set_text(GTK_LABEL(stat_note[0]), buf);
    set_note(stat_note[1], "%d%% of all reports", percent_of(lost_total, item_count));
    set_note(stat_note[2], "%d%% of all reports", percent_of(found_total, item_count));
    set_note(stat_note[3], "%d%% of all reports", percent_of(claimed_total, item_count));

    /* Counts shown beside the sidebar menu */
    set_number(status_nav_count[0], item_count);
    set_number(status_nav_count[1], lost_total);
    set_number(status_nav_count[2], found_total);
    set_number(status_nav_count[3], claimed_total);

    /* Category bars: width is handled via relative rem units in CSS */
    int max_count = 0;
    for (int i = 0; i < 5; i++) {
        if (category_counts[i] > max_count) max_count = category_counts[i];
    }
    for (int i = 0; i < 5; i++) {
        int height = 28 + (max_count > 0 ? (category_counts[i] * 120) / max_count : 0);
        gtk_widget_set_size_request(bar_widget[i], -1, height);
        set_number(bar_count[i], category_counts[i]);
        if (max_count > 0 && category_counts[i] == max_count) gtk_widget_add_css_class(bar_widget[i], "bar-top");
        else                                                  gtk_widget_remove_css_class(bar_widget[i], "bar-top");
    }

    /* Gauge: the big number is the share of reports already claimed */
    snprintf(buf, sizeof(buf), "%d%%", percent_of(claimed_total, item_count));
    gtk_label_set_text(GTK_LABEL(gauge_percent), buf);
    snprintf(buf, sizeof(buf), "Claimed  %d", claimed_total);
    gtk_label_set_text(GTK_LABEL(legend_text[0]), buf);
    snprintf(buf, sizeof(buf), "Found  %d", found_total);
    gtk_label_set_text(GTK_LABEL(legend_text[1]), buf);
    snprintf(buf, sizeof(buf), "Lost  %d", lost_total);
    gtk_label_set_text(GTK_LABEL(legend_text[2]), buf);
    gtk_widget_queue_draw(gauge_area);
}

/* Responsive half-circle gauge drawn with Cairo. Scales dynamically with window size. */
static void draw_gauge(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer data) {
    (void)area;
    (void)data;
    double stroke_w = (height * 0.16 < 18.0) ? 18.0 : height * 0.16;
    double cx = width / 2.0;
    double cy = height - (stroke_w / 2.0);
    double max_r = (width / 2.0 < cy) ? width / 2.0 : cy;
    double radius = max_r - stroke_w;
    if (radius < 20.0) radius = 20.0;

    double start = G_PI;                      /* left end of the half circle */
    int total = lost_total + found_total + claimed_total;

    cairo_set_line_width(cr, stroke_w);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);

    /* grey-blue track */
    cairo_set_source_rgb(cr, 0.88, 0.91, 0.97);
    cairo_arc(cr, cx, cy, radius, G_PI, 2 * G_PI);
    cairo_stroke(cr);
    if (total == 0) return;

    /* claimed (green), found (cyan), lost (red) */
    int counts[3] = {claimed_total, found_total, lost_total};
    double colors[3][3] = {{0.08, 0.64, 0.50}, {0.07, 0.66, 0.84}, {0.94, 0.33, 0.31}};
    for (int i = 0; i < 3; i++) {
        if (counts[i] == 0) continue;
        double sweep = G_PI * counts[i] / total;
        cairo_set_source_rgb(cr, colors[i][0], colors[i][1], colors[i][2]);
        cairo_arc(cr, cx, cy, radius, start, start + sweep);
        cairo_stroke(cr);
        start += sweep;
    }
}

/* ------------------------------------------------------------------ */
/* Actions on a single record                                          */
/* ------------------------------------------------------------------ */

static void refresh_list(void);

static void mark_claimed(GtkButton *button, gpointer user_data) {
    int id = GPOINTER_TO_INT(user_data);
    (void)button;
    for (int i = 0; i < item_count; i++) {
        if (items[i].id == id) {
            g_strlcpy(items[i].status, "Claimed", sizeof(items[i].status));
            save_items();
            refresh_list();
            return;
        }
    }
}

static void delete_item(GtkButton *button, gpointer user_data) {
    int id = GPOINTER_TO_INT(user_data);
    (void)button;
    for (int i = 0; i < item_count; i++) {
        if (items[i].id == id) {
            for (int j = i; j < item_count - 1; j++) {
                items[j] = items[j + 1];   /* shift the rest up by one */
            }
            item_count--;
            save_items();
            refresh_list();
            return;
        }
    }
}

/* ------------------------------------------------------------------ */
/* One row of the reports list                                         */
/* ------------------------------------------------------------------ */

static GtkWidget *make_item_card(const Item *item) {
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_widget_add_css_class(card, "item-card");

    const char *status_text;
    const char *badge_class;
    if (!strcmp(item->status, "Lost")) {
        gtk_widget_add_css_class(card, "item-card-lost");
        status_text = "LOST";
        badge_class = "lost-badge";
    } else if (!strcmp(item->status, "Found")) {
        gtk_widget_add_css_class(card, "item-card-found");
        status_text = "FOUND";
        badge_class = "found-badge";
    } else {
        gtk_widget_add_css_class(card, "item-card-claimed");
        status_text = "CLAIMED";
        badge_class = "claimed-badge";
    }

    /* Left: category icon tile */
    GtkWidget *icon_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(icon_box, "item-icon-box");
    gtk_widget_set_valign(icon_box, GTK_ALIGN_CENTER);

    GtkWidget *icon = gtk_label_new(get_category_icon(item->category));
    gtk_widget_add_css_class(icon, "item-icon-glyph");
    gtk_widget_set_halign(icon, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(icon, GTK_ALIGN_CENTER);
    gtk_widget_set_vexpand(icon, TRUE);
    gtk_box_append(GTK_BOX(icon_box), icon);
    gtk_box_append(GTK_BOX(card), icon_box);

    /* Middle: id + name on top, category + location below */
    GtkWidget *details = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_hexpand(details, TRUE);
    gtk_widget_set_valign(details, GTK_ALIGN_CENTER);

    GtkWidget *title_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    char id_str[20];
    snprintf(id_str, sizeof(id_str), "#%03d", item->id);
    GtkWidget *id_chip = label_new(id_str, "id-chip");
    gtk_widget_set_valign(id_chip, GTK_ALIGN_CENTER);

    GtkWidget *name_lbl = label_new(item->name, "item-name");
    gtk_label_set_ellipsize(GTK_LABEL(name_lbl), PANGO_ELLIPSIZE_END);
    gtk_widget_set_hexpand(name_lbl, TRUE);

    gtk_box_append(GTK_BOX(title_row), id_chip);
    gtk_box_append(GTK_BOX(title_row), name_lbl);
    gtk_box_append(GTK_BOX(details), title_row);

    GtkWidget *meta_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget *cat_chip = label_new(item->category, "category-chip");

    char loc_str[120];
    snprintf(loc_str, sizeof(loc_str), "📍 %s", item->location);
    GtkWidget *loc_lbl = label_new(loc_str, "item-location");
    gtk_label_set_ellipsize(GTK_LABEL(loc_lbl), PANGO_ELLIPSIZE_END);
    gtk_widget_set_hexpand(loc_lbl, TRUE);

    gtk_box_append(GTK_BOX(meta_row), cat_chip);
    gtk_box_append(GTK_BOX(meta_row), loc_lbl);
    gtk_box_append(GTK_BOX(details), meta_row);
    gtk_box_append(GTK_BOX(card), details);

    /* Right: status badge and action buttons in one row */
    GtkWidget *right_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_valign(right_box, GTK_ALIGN_CENTER);

    GtkWidget *badge = label_new(status_text, "status-badge");
    gtk_widget_add_css_class(badge, badge_class);
    gtk_widget_set_valign(badge, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(right_box), badge);

    if (strcmp(item->status, "Claimed") != 0) {
        GtkWidget *claim_btn = make_button("✓ Mark claimed", "claim-button");
        g_signal_connect(claim_btn, "clicked", G_CALLBACK(mark_claimed), GINT_TO_POINTER(item->id));
        gtk_box_append(GTK_BOX(right_box), claim_btn);
    }

    GtkWidget *del_btn = make_button("✕", "delete-button");
    g_signal_connect(del_btn, "clicked", G_CALLBACK(delete_item), GINT_TO_POINTER(item->id));
    gtk_box_append(GTK_BOX(right_box), del_btn);

    gtk_box_append(GTK_BOX(card), right_box);
    return card;
}

/* ------------------------------------------------------------------ */
/* Filtering, searching and redrawing the list                         */
/* ------------------------------------------------------------------ */

static void refresh_list(void) {
    /* 1. Remove all old rows */
    GtkWidget *child;
    while ((child = gtk_widget_get_first_child(list_box))) {
        gtk_list_box_remove(GTK_LIST_BOX(list_box), child);
    }

    /* 2. Add a row for every record that matches status, category and search text */
    const char *query = gtk_editable_get_text(GTK_EDITABLE(search_entry));
    char *needle = g_utf8_strdown(query, -1);
    int visible = 0;

    for (int i = item_count - 1; i >= 0; i--) {   /* newest first */
        if (strcmp(current_filter, "All") != 0 && strcmp(items[i].status, current_filter) != 0) continue;
        if (strcmp(current_category, "All") != 0 && strcmp(items[i].category, current_category) != 0) continue;

        char *name = g_utf8_strdown(items[i].name, -1);
        char *category = g_utf8_strdown(items[i].category, -1);
        char *location = g_utf8_strdown(items[i].location, -1);

        gboolean matches = strstr(name, needle) != NULL ||
                           strstr(category, needle) != NULL ||
                           strstr(location, needle) != NULL;
        g_free(name);
        g_free(category);
        g_free(location);

        if (matches) {
            gtk_list_box_append(GTK_LIST_BOX(list_box), make_item_card(&items[i]));
            visible++;
        }
    }
    g_free(needle);

    /* 3. Heading, "showing" chip, empty message and all statistics */
    const char *a = strcmp(current_filter, "All") ? current_filter : "";
    const char *b = strcmp(current_category, "All") ? current_category : "";
    char title[100];
    if (!*a && !*b)    snprintf(title, sizeof(title), "All reports");
    else if (*a && *b) snprintf(title, sizeof(title), "%s  ·  %s", a, b);
    else               snprintf(title, sizeof(title), "%s reports", *a ? a : b);
    gtk_label_set_text(GTK_LABEL(list_title_label), title);

    char buf[48];
    snprintf(buf, sizeof(buf), "Showing %d of %d", visible, item_count);
    gtk_label_set_text(GTK_LABEL(shown_label), buf);

    gtk_widget_set_visible(empty_box, visible == 0);
    update_stats();
}

/* Highlight the sidebar entry that matches the current filter */
static void set_nav_active(GtkWidget *button, gboolean active) {
    if (active) gtk_widget_add_css_class(button, "nav-active");
    else        gtk_widget_remove_css_class(button, "nav-active");
}

static void update_nav_styles(void) {
    for (int i = 0; i < 4; i++) set_nav_active(status_nav[i], !strcmp(nav_statuses[i], current_filter));
    for (int i = 0; i < 6; i++) set_nav_active(category_nav[i], !strcmp(nav_categories[i], current_category));
}

static void on_status_nav_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    g_strlcpy(current_filter, (const char *)user_data, sizeof(current_filter));
    update_nav_styles();
    refresh_list();
}

static void on_category_nav_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    g_strlcpy(current_category, (const char *)user_data, sizeof(current_category));
    update_nav_styles();
    refresh_list();
}

static void on_search_changed(GtkSearchEntry *entry, gpointer user_data) {
    (void)entry;
    (void)user_data;
    refresh_list();
}

/* ------------------------------------------------------------------ */
/* Adding a new record (pop-up form)                                   */
/* ------------------------------------------------------------------ */

static gboolean valid_text(const char *text) {
    if (!text || !*text) return FALSE;
    int has_non_space = 0;
    for (const char *p = text; *p; p++) {
        if (*p == '\t' || *p == '\n' || *p == '\r') return FALSE;  /* would break the file format */
        if (*p != ' ') has_non_space = 1;
    }
    return has_non_space;
}

/* Used by the "Add report" button and by pressing Enter inside the text boxes */
static void on_add_clicked(GtkWidget *widget, gpointer user_data) {
    (void)widget;
    (void)user_data;
    const char *name = gtk_editable_get_text(GTK_EDITABLE(name_entry));
    const char *location = gtk_editable_get_text(GTK_EDITABLE(location_entry));

    if (!valid_text(name) || !valid_text(location)) {
        show_message(add_dialog, "Missing information",
                     "Enter both an item name and the place it was lost or found.");
        return;
    }
    if (strlen(name) >= sizeof(items[0].name) || strlen(location) >= sizeof(items[0].location)) {
        show_message(add_dialog, "Text too long", "The name and location must each be under 80 characters.");
        return;
    }
    if (item_count >= MAX_ITEMS) {
        show_message(add_dialog, "Storage full", "The maximum of 500 items has been reached.");
        return;
    }

    /* new id = highest existing id + 1 */
    int next_id = 1;
    for (int i = 0; i < item_count; i++) {
        if (items[i].id >= next_id) next_id = items[i].id + 1;
    }

    Item *item = &items[item_count++];
    item->id = next_id;
    g_strlcpy(item->name, name, sizeof(item->name));
    g_strlcpy(item->location, location, sizeof(item->location));
    g_strlcpy(item->category, form_categories[gtk_drop_down_get_selected(GTK_DROP_DOWN(category_drop))],
              sizeof(item->category));
    g_strlcpy(item->status, form_statuses[gtk_drop_down_get_selected(GTK_DROP_DOWN(status_drop))],
              sizeof(item->status));

    save_items();
    gtk_window_destroy(GTK_WINDOW(add_dialog));   /* close the form */
    add_dialog = NULL;
    refresh_list();
}

static GtkWidget *make_field(const char *title, GtkWidget *control) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_box_append(GTK_BOX(box), label_new(title, "field-label"));
    gtk_widget_set_hexpand(control, TRUE);
    gtk_box_append(GTK_BOX(box), control);
    return box;
}

static void open_add_dialog(GtkWidget *widget, gpointer user_data) {
    (void)widget;
    (void)user_data;
    add_dialog = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(add_dialog), "Add report");
    gtk_window_set_transient_for(GTK_WINDOW(add_dialog), GTK_WINDOW(window));
    gtk_window_set_modal(GTK_WINDOW(add_dialog), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(add_dialog), 520, 10);
    gtk_window_set_resizable(GTK_WINDOW(add_dialog), FALSE);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_add_css_class(box, "dialog-box");

    gtk_box_append(GTK_BOX(box), label_new("Report an item", "dialog-title"));
    gtk_box_append(GTK_BOX(box), label_new("Add something lost or found on campus", "panel-subtitle"));

    name_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(name_entry), "e.g. Black Casio calculator");
    g_signal_connect(name_entry, "activate", G_CALLBACK(on_add_clicked), NULL);
    gtk_box_append(GTK_BOX(box), make_field("Item name", name_entry));

    category_drop = gtk_drop_down_new_from_strings(form_categories);
    gtk_widget_add_css_class(category_drop, "field-control");
    gtk_box_append(GTK_BOX(box), make_field("Category", category_drop));

    location_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(location_entry), "e.g. Library, second floor");
    g_signal_connect(location_entry, "activate", G_CALLBACK(on_add_clicked), NULL);
    gtk_box_append(GTK_BOX(box), make_field("Where was it lost or found?", location_entry));

    status_drop = gtk_drop_down_new_from_strings(form_statuses);
    gtk_widget_add_css_class(status_drop, "field-control");
    gtk_box_append(GTK_BOX(box), make_field("Report type", status_drop));

    GtkWidget *buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_halign(buttons, GTK_ALIGN_END);
    gtk_widget_set_margin_top(buttons, 10);

    GtkWidget *cancel = make_button("Cancel", "ghost-button");
    g_signal_connect_swapped(cancel, "clicked", G_CALLBACK(gtk_window_destroy), add_dialog);
    GtkWidget *add = make_button("＋  Add report", "primary-button");
    g_signal_connect(add, "clicked", G_CALLBACK(on_add_clicked), NULL);
    gtk_box_append(GTK_BOX(buttons), cancel);
    gtk_box_append(GTK_BOX(buttons), add);
    gtk_box_append(GTK_BOX(box), buttons);

    gtk_window_set_child(GTK_WINDOW(add_dialog), box);
    gtk_window_present(GTK_WINDOW(add_dialog));
    gtk_widget_grab_focus(name_entry);
}

/* ------------------------------------------------------------------ */
/* Reusable component builders                                         */
/* ------------------------------------------------------------------ */

/* Sidebar menu button: icon + name (+ optional count badge) */
static GtkWidget *make_nav_item(const char *icon, const char *name, GtkWidget **count_out) {
    GtkWidget *button = gtk_button_new();
    gtk_widget_add_css_class(button, "nav-item");
    gtk_widget_set_cursor_from_name(button, "pointer");

    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_box_append(GTK_BOX(row), label_new(icon, "nav-icon"));

    GtkWidget *name_lbl = label_new(name, "nav-name");
    gtk_widget_set_hexpand(name_lbl, TRUE);
    gtk_box_append(GTK_BOX(row), name_lbl);

    if (count_out) {
        *count_out = label_new("0", "nav-count");
        gtk_box_append(GTK_BOX(row), *count_out);
    }
    gtk_button_set_child(GTK_BUTTON(button), row);
    return button;
}

/* Statistic card: title + arrow on top, big number, small note */
static GtkWidget *make_stat_card(const char *title, GtkWidget **number_label,
                                 GtkWidget **note_label, const char *variant_class) {
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_add_css_class(card, "stat-card");
    gtk_widget_add_css_class(card, variant_class);
    gtk_widget_set_hexpand(card, TRUE);

    GtkWidget *top = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *title_lbl = label_new(title, "stat-title");
    gtk_widget_set_hexpand(title_lbl, TRUE);
    gtk_box_append(GTK_BOX(top), title_lbl);
    gtk_box_append(GTK_BOX(top), label_new("↗", "stat-arrow"));
    gtk_box_append(GTK_BOX(card), top);

    *number_label = label_new("0", "stat-number");
    gtk_box_append(GTK_BOX(card), *number_label);

    *note_label = label_new("", "stat-note");
    gtk_widget_set_halign(*note_label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(card), *note_label);
    return card;
}

static void load_css(void) {
    GtkCssProvider *css = gtk_css_provider_new();
    gtk_css_provider_load_from_path(css, "style.css");   /* must be in the folder you run from */
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css);
}

/* ------------------------------------------------------------------ */
/* Window layout                                                       */
/* ------------------------------------------------------------------ */

/* Left column: logo, status menu, category menu, "report an item" card */
static GtkWidget *build_sidebar(void) {
    GtkWidget *side = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);

    gtk_widget_set_size_request(side, 316, -1);
    gtk_widget_set_hexpand(side, FALSE);

    gtk_widget_add_css_class(side, "side-panel");
    gtk_widget_set_vexpand(side, TRUE);

    GtkWidget *brand = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_margin_start(brand, 8);
    gtk_widget_set_margin_bottom(brand, 18);
    GtkWidget *logo = label_new("◈", "logo-mark");
    gtk_widget_set_valign(logo, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(brand), logo);
    gtk_box_append(GTK_BOX(brand),
               label_new("Campus Lost & Found Manager", "logo-text"));
    gtk_box_append(GTK_BOX(side), brand);

    /* MENU: filter by status */
    gtk_box_append(GTK_BOX(side), label_new("MENU", "nav-section"));
    const char *status_icons[4] = {"▦", "🔍", "📦", "✅"};
    const char *status_titles[4] = {"All reports", "Lost items", "Found items", "Claimed"};
    for (int i = 0; i < 4; i++) {
        status_nav[i] = make_nav_item(status_icons[i], status_titles[i], &status_nav_count[i]);
        g_signal_connect(status_nav[i], "clicked", G_CALLBACK(on_status_nav_clicked), (gpointer)nav_statuses[i]);
        gtk_box_append(GTK_BOX(side), status_nav[i]);
    }

    /* CATEGORY: filter by category */
    GtkWidget *cat_title = label_new("CATEGORY", "nav-section");
    gtk_widget_set_margin_top(cat_title, 16);
    gtk_box_append(GTK_BOX(side), cat_title);
    const char *cat_icons[6] = {"▦", "💻", "🎒", "📚", "📄", "📦"};
    for (int i = 0; i < 6; i++) {
        category_nav[i] = make_nav_item(cat_icons[i], nav_categories[i], NULL);
        g_signal_connect(category_nav[i], "clicked", G_CALLBACK(on_category_nav_clicked), (gpointer)nav_categories[i]);
        gtk_box_append(GTK_BOX(side), category_nav[i]);
    }
    update_nav_styles();

    /* Spacer pushes the card to the bottom */
    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_vexpand(spacer, TRUE);
    gtk_box_append(GTK_BOX(side), spacer);

    GtkWidget *promo = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_add_css_class(promo, "promo-card");
    gtk_box_append(GTK_BOX(promo), label_new("Found something?", "promo-title"));
    GtkWidget *promo_text = label_new("Log it now so the owner can find it fast.", "promo-text");
    gtk_label_set_wrap(GTK_LABEL(promo_text), TRUE);
    gtk_box_append(GTK_BOX(promo), promo_text);
    GtkWidget *promo_btn = make_button("＋  Report item", "promo-button");
    gtk_widget_set_margin_top(promo_btn, 8);
    g_signal_connect(promo_btn, "clicked", G_CALLBACK(open_add_dialog), NULL);
    gtk_box_append(GTK_BOX(promo), promo_btn);
    gtk_box_append(GTK_BOX(side), promo);
    return side;
}

/* Top bar: search box on the left, profile on the right */
static GtkWidget *build_topbar(void) {
    GtkWidget *bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 14);
    gtk_widget_add_css_class(bar, "topbar-card");

    search_entry = gtk_search_entry_new();
    gtk_widget_add_css_class(search_entry, "search-field");
    gtk_widget_set_hexpand(search_entry, TRUE);
    gtk_search_entry_set_placeholder_text(GTK_SEARCH_ENTRY(search_entry),
                                          "Search by name, category or location");
    g_signal_connect(search_entry, "search-changed", G_CALLBACK(on_search_changed), NULL);
    gtk_box_append(GTK_BOX(bar), search_entry);

    GtkWidget *avatar = label_new("HD", "user-avatar");
    gtk_widget_set_valign(avatar, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(bar), avatar);

    GtkWidget *user_text = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_valign(user_text, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(user_text), label_new("Help Desk", "user-name"));
    gtk_box_append(GTK_BOX(user_text), label_new("lostfound@campus.edu", "user-mail"));
    gtk_box_append(GTK_BOX(bar), user_text);
    return bar;
}

/* Page title with the "Add report" button */
static GtkWidget *build_title_row(void) {
    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);

    GtkWidget *titles = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_hexpand(titles, TRUE);
    gtk_widget_set_valign(titles, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(titles),
               label_new("Campus Lost & Found Manager", "page-title"));
    gtk_box_append(GTK_BOX(titles), label_new("Track, search and resolve lost and found reports with ease.", "page-subtitle"));
    gtk_box_append(GTK_BOX(row), titles);

    GtkWidget *add_btn = make_button("＋  Add report", "primary-button");
    gtk_widget_set_valign(add_btn, GTK_ALIGN_CENTER);
    g_signal_connect(add_btn, "clicked", G_CALLBACK(open_add_dialog), NULL);
    gtk_box_append(GTK_BOX(row), add_btn);
    return row;
}

static GtkWidget *build_stats_row(void) {
    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_box_set_homogeneous(GTK_BOX(row), TRUE);
    gtk_box_append(GTK_BOX(row), make_stat_card("Total reports", &stat_number[0], &stat_note[0], "stat-main"));
    gtk_box_append(GTK_BOX(row), make_stat_card("Still lost",    &stat_number[1], &stat_note[1], "stat-lost"));
    gtk_box_append(GTK_BOX(row), make_stat_card("Found, waiting", &stat_number[2], &stat_note[2], "stat-found"));
    gtk_box_append(GTK_BOX(row), make_stat_card("Claimed",       &stat_number[3], &stat_note[3], "stat-claimed"));
    return row;
}

/* Wide card with the scrolling list of reports */
static GtkWidget *build_report_list(void) {
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
    gtk_widget_add_css_class(card, "panel-card");
    gtk_widget_set_hexpand(card, TRUE);
    gtk_widget_set_vexpand(card, TRUE);

    GtkWidget *top = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget *titles = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_hexpand(titles, TRUE);
    list_title_label = label_new("All reports", "panel-title");
    gtk_box_append(GTK_BOX(titles), list_title_label);
    gtk_box_append(GTK_BOX(titles), label_new("Use the menu on the left to filter", "panel-subtitle"));
    gtk_box_append(GTK_BOX(top), titles);

    shown_label = label_new("Showing 0 of 0", "count-chip");
    gtk_widget_set_valign(shown_label, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(top), shown_label);
    gtk_box_append(GTK_BOX(card), top);

    /* The list is the only part of the window that scrolls */
    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scroll, -1, 160);
    gtk_widget_set_vexpand(scroll, TRUE);

    GtkWidget *container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    list_box = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(list_box), GTK_SELECTION_NONE);
    gtk_widget_add_css_class(list_box, "item-list");
    gtk_box_append(GTK_BOX(container), list_box);

    /* Empty-state card, shown when nothing matches */
    empty_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_add_css_class(empty_box, "empty-state-box");
    GtkWidget *empty_icon = label_new("🔍", "empty-icon");
    gtk_widget_set_halign(empty_icon, GTK_ALIGN_CENTER);
    GtkWidget *empty_title = label_new("No reports to show", "empty-title");
    gtk_widget_set_halign(empty_title, GTK_ALIGN_CENTER);
    GtkWidget *empty_sub = label_new("Change the filter or search, or add a new report.", "empty-subtitle");
    gtk_widget_set_halign(empty_sub, GTK_ALIGN_CENTER);
    gtk_label_set_wrap(GTK_LABEL(empty_sub), TRUE);
    gtk_label_set_justify(GTK_LABEL(empty_sub), GTK_JUSTIFY_CENTER);
    gtk_box_append(GTK_BOX(empty_box), empty_icon);
    gtk_box_append(GTK_BOX(empty_box), empty_title);
    gtk_box_append(GTK_BOX(empty_box), empty_sub);
    gtk_box_append(GTK_BOX(container), empty_box);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), container);
    gtk_box_append(GTK_BOX(card), scroll);
    return card;
}

/* Right column, top: bar chart of reports per category */
static GtkWidget *build_chart_card(void) {
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_add_css_class(card, "panel-card");
    gtk_widget_set_vexpand(card, TRUE);
    gtk_box_append(GTK_BOX(card), label_new("Reports by category", "panel-title"));
    gtk_box_append(GTK_BOX(card), label_new("Where items go missing most", "panel-subtitle"));

    GtkWidget *chart = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_box_set_homogeneous(GTK_BOX(chart), TRUE);
    gtk_widget_set_vexpand(chart, TRUE);
    gtk_widget_set_size_request(chart, -1, 170);
    gtk_widget_set_margin_top(chart, 12);

    for (int i = 0; i < 5; i++) {
        GtkWidget *column = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
        gtk_widget_set_valign(column, GTK_ALIGN_END);
        gtk_widget_set_tooltip_text(column, form_categories[i]);

        bar_count[i] = label_new("0", "bar-count");
        gtk_widget_set_halign(bar_count[i], GTK_ALIGN_CENTER);

        bar_widget[i] = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_widget_add_css_class(bar_widget[i], "bar");
        gtk_widget_set_halign(bar_widget[i], GTK_ALIGN_CENTER);

        GtkWidget *icon = label_new(category_icons[i], "bar-label");
        gtk_widget_set_halign(icon, GTK_ALIGN_CENTER);

        gtk_box_append(GTK_BOX(column), bar_count[i]);
        gtk_box_append(GTK_BOX(column), bar_widget[i]);
        gtk_box_append(GTK_BOX(column), icon);
        gtk_box_append(GTK_BOX(chart), column);
    }
    gtk_box_append(GTK_BOX(card), chart);
    return card;
}

/* Right column, bottom: half-circle gauge with legend */
static GtkWidget *build_gauge_card(void) {
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_add_css_class(card, "panel-card");
    gtk_widget_set_vexpand(card, TRUE);
    gtk_box_append(GTK_BOX(card), label_new("Recovery progress", "panel-title"));
    gtk_box_append(GTK_BOX(card), label_new("Share of reports already claimed", "panel-subtitle"));

    /* The percentage label sits on top of the drawing, inside the arc */
    GtkWidget *overlay = gtk_overlay_new();
    gtk_widget_set_hexpand(overlay, TRUE);
    gtk_widget_set_vexpand(overlay, TRUE);
    gtk_widget_set_margin_top(overlay, 8);

    gauge_area = gtk_drawing_area_new();
    gtk_widget_set_hexpand(gauge_area, TRUE);
    gtk_widget_set_vexpand(gauge_area, TRUE);
    gtk_widget_set_size_request(gauge_area, 260, 150);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(gauge_area), draw_gauge, NULL, NULL);
    gtk_overlay_set_child(GTK_OVERLAY(overlay), gauge_area);

    GtkWidget *center = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_halign(center, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(center, GTK_ALIGN_END);
    gtk_widget_set_margin_bottom(center, 6);
    gauge_percent = label_new("0%", "gauge-percent");
    gtk_widget_set_halign(gauge_percent, GTK_ALIGN_CENTER);
    GtkWidget *caption = label_new("Recovered", "gauge-caption");
    gtk_widget_set_halign(caption, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(center), gauge_percent);
    gtk_box_append(GTK_BOX(center), caption);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), center);
    gtk_box_append(GTK_BOX(card), overlay);

    /* Legend: coloured dot + text for claimed, found, lost */
    GtkWidget *legend = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_widget_set_halign(legend, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(legend, 8);
    const char *dot_classes[3] = {"dot-claimed", "dot-found", "dot-lost"};
    for (int i = 0; i < 3; i++) {
        GtkWidget *entry = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        gtk_box_append(GTK_BOX(entry), label_new("●", dot_classes[i]));
        legend_text[i] = label_new("", "legend-text");
        gtk_box_append(GTK_BOX(entry), legend_text[i]);
        gtk_box_append(GTK_BOX(legend), entry);
    }
    gtk_box_append(GTK_BOX(card), legend);
    return card;
}

static void build_ui(GtkApplication *app, gpointer user_data) {
    (void)user_data;
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Campus Lost & Found Manager");
    gtk_window_set_default_size(GTK_WINDOW(window), 1440, 840);
    gtk_widget_set_size_request(window, 1240, 720);
    load_css();

    /* Two columns: sidebar on the left, dashboard on the right.
       Nothing here scrolls except the report list itself. */
    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_widget_add_css_class(root, "app-root");
    gtk_window_set_child(GTK_WINDOW(window), root);
    gtk_box_append(GTK_BOX(root), build_sidebar());

    GtkWidget *main_col = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_hexpand(main_col, TRUE);
    gtk_widget_set_vexpand(main_col, TRUE);
    gtk_box_append(GTK_BOX(main_col), build_topbar());
    gtk_box_append(GTK_BOX(main_col), build_title_row());
    gtk_box_append(GTK_BOX(main_col), build_stats_row());

    /* Bottom area: report list (wide) + chart and gauge (responsive right column) */
    /* Bottom area: report list + chart and gauge */
GtkWidget *bottom = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
gtk_widget_set_vexpand(bottom, TRUE);
gtk_box_append(GTK_BOX(bottom), build_report_list());

/* Right analytics column */
GtkWidget *right_col = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
gtk_widget_set_size_request(right_col, 316, -1);
gtk_widget_set_hexpand(right_col, FALSE);
gtk_widget_add_css_class(right_col, "right-analytics-col");
gtk_widget_set_vexpand(right_col, TRUE);

gtk_box_append(GTK_BOX(right_col), build_chart_card());
gtk_box_append(GTK_BOX(right_col), build_gauge_card());
gtk_box_append(GTK_BOX(bottom), right_col);

    gtk_box_append(GTK_BOX(main_col), bottom);
    gtk_box_append(GTK_BOX(root), main_col);

    load_items();
#if LOAD_SAMPLE_DATA
    if (item_count == 0) load_sample_items();
#endif
    refresh_list();
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("com.campus.lostfound", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(build_ui), NULL);
    int result = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return result;
}