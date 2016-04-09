/*
 * suggestion_window.cpp - a core code of Geany completion plugins
 *
 * Copyright (C) 2014-2015 Noto, Yuta <nonotetau(at)gmail(dot)com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <geanycc/suggestion_window.hpp>

#include <string.h>
#include <gdk/gdkkeysyms.h>

namespace geanycc
{
namespace detail
{
static void get_note_height(GtkWidget* widget, int* height)
{
	GtkAllocation allocation;
	gtk_widget_get_allocation(widget, &allocation);
	if (*height < allocation.height) {
		*height = allocation.height;
	}
}
static gboolean close_request(GtkWidget* widget, GdkEvent* event, SuggestionWindow* self)
{
	self->close();
	return FALSE;
}
}

enum { MODEL_TYPEDTEXT_INDEX = 0, MODEL_LABEL_INDEX = 1, MODEL_TYPE_INDEX = 2 };

void SuggestionWindow::move_cursor(bool down)
{
	GtkTreeIter iter;
	GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
	if (gtk_tree_selection_get_selected(selection, NULL, &iter)) {
		if (down) { /* move down */
			if (gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter)) {
				GtkTreePath* path = gtk_tree_model_get_path(GTK_TREE_MODEL(model), &iter);
				gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(tree_view), path, NULL, FALSE, 0.0, 0.0);
				gtk_tree_view_set_cursor_on_cell(GTK_TREE_VIEW(tree_view), path, NULL, NULL, FALSE);
				gtk_tree_path_free(path);
			}
		} else { /* move up */
			GtkTreePath* path = gtk_tree_model_get_path(GTK_TREE_MODEL(model), &iter);
			if (gtk_tree_path_prev(path)) {
				gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(tree_view), path, NULL, FALSE, 0.0, 0.0);
				gtk_tree_view_set_cursor_on_cell(GTK_TREE_VIEW(tree_view), path, NULL, NULL, FALSE);
				gtk_tree_path_free(path);
			}
		}
	}
}

void SuggestionWindow::select_suggestion()
{
	GtkTreeIter iter;
	GtkTreeModel* model;
	GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
	if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
		gchar* typedtext;
		gtk_tree_model_get(model, &iter, MODEL_TYPEDTEXT_INDEX, &typedtext, -1);

		int dist = strlen(typedtext) - filtered_str.length();

		if (dist != 0 || filtered_str.compare(typedtext) != 0) {
			GeanyDocument* doc = document_get_current();
			if (doc != NULL) {
				ScintillaObject* sci = doc->editor->sci;
				int cur_pos = sci_get_current_position(sci);
				int added_byte = filtered_str.length();
				sci_set_selection_start(sci, cur_pos - added_byte);
				sci_set_selection_end(sci, cur_pos);
				sci_replace_sel(sci, typedtext); /* do nothing */
			}
		}

		g_free(typedtext);
		this->close();
	}
}

void SuggestionWindow::signal_tree_selection(GtkTreeView* tree_view, GtkTreePath* path,
                                             GtkTreeViewColumn* column, SuggestionWindow* self)
{
	self->select_suggestion();
}

gboolean SuggestionWindow::signal_key_press_and_release(GtkWidget* widget, GdkEventKey* event,
                                                        SuggestionWindow* self)
{
	if (!self->is_showing()) {
		return FALSE;
	}

	switch (event->keyval) {
		/* take over for selecting a suggestion */
		case GDK_KEY_Down:
		case GDK_KEY_KP_Down:
			self->move_cursor(true);
			return TRUE;
		case GDK_KEY_Up:
		case GDK_KEY_KP_Up:
			self->move_cursor(false);
			return TRUE;
		case GDK_KEY_Page_Down:
		case GDK_KEY_KP_Page_Down:
			for (int i = 0; i < self->page_up_down_skip_amount; i++) {
				self->move_cursor(true);
			}
			return TRUE;
		case GDK_KEY_Page_Up:
		case GDK_KEY_KP_Page_Up:
			for (int i = 0; i < self->page_up_down_skip_amount; i++) {
				self->move_cursor(false);
			}
			return TRUE;
		/* select current suggestion */
		case GDK_KEY_Return:
		case GDK_KEY_KP_Enter:
			self->select_suggestion();
			return TRUE;
		// case GDK_KEY_BackSpace:
		// self->filter_backspace();
		// return FALSE; /* editor will delete a char. */
		// case GDK_KEY_Delete: case GDK_KEY_KP_Delete:
		case GDK_KEY_Escape:
		case GDK_KEY_Right:
		case GDK_KEY_KP_Right:
		case GDK_KEY_Left:
		case GDK_KEY_KP_Left:
			self->close();
			return FALSE;
		default:
			return FALSE;
	}
}

gboolean SuggestionWindow::signal_mouse_scroll(GtkWidget* widget, GdkEventScroll* event,
                                                        SuggestionWindow* self)
{
	if (!self->is_showing()) {
		return FALSE;
	}

	switch (event->direction) {
		case GDK_SCROLL_DOWN:
			self->move_cursor(true);
			return TRUE;
		case GDK_SCROLL_UP:
			self->move_cursor(false);
			return TRUE;
		default:
			return FALSE;
	}
}

void SuggestionWindow::do_filtering()
{
	if (!this->is_showing()) {
		return;
	}

	GtkTreeIter iter;

	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter)) {
		do {
			gchar* typedtext;
			gtk_tree_model_get(GTK_TREE_MODEL(model), &iter, MODEL_TYPEDTEXT_INDEX, &typedtext, -1);

			gboolean endFlag = FALSE;
			if (strstr(typedtext, filtered_str.c_str()) == typedtext) {
				GtkTreePath* path = gtk_tree_model_get_path(GTK_TREE_MODEL(model), &iter);
				gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(tree_view), path, NULL, TRUE, 0.0, 0.0);
				gtk_tree_view_set_cursor_on_cell(GTK_TREE_VIEW(tree_view), path, NULL, NULL, FALSE);
				gtk_tree_path_free(path);

				endFlag = TRUE;
			}
			g_free(typedtext);
			if (endFlag) {
				return;
			}
		} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter));

		/* not found */
		this->close();
	} else { /* empty suggestion */
		this->close();
	}
}

void SuggestionWindow::filter_backspace()
{
	if (this->is_showing()) {
		if (filtered_str.empty()) {
			this->close();
		} else {
			filtered_str.erase(filtered_str.length() - 1);
			this->do_filtering();
		}
	}
}

void SuggestionWindow::filter_add(int ch)
{
	if (this->is_showing()) {
		char buf[8] = {0};
		g_unichar_to_utf8((gunichar)ch, buf);
		filtered_str += buf;
		do_filtering();
	}
}

void SuggestionWindow::filter_add(const std::string& str)
{
	if (this->is_showing()) {
		filtered_str += str;
		do_filtering();
	}
}

//#define CHECK_ARRANGE(fmt, ...) g_print(fmt, __VA_ARGS__)
#define CHECK_ARRANGE(fmt, ...) \
	{                           \
	}

void SuggestionWindow::arrange_window()
{
	// gtk2+
	GtkRequisition sg_win_size;
	gtk_widget_size_request(tree_view, &sg_win_size);
	CHECK_ARRANGE("sg_win_size(%d, %d)", sg_win_size.width, sg_win_size.height);
	if (sg_win_size.height > max_window_height) {
		sg_win_size.height = max_window_height;
	}
	CHECK_ARRANGE("after, sg_win_size(%d, %d)", sg_win_size.width, sg_win_size.height);
	gtk_widget_set_size_request(window, sg_win_size.width, sg_win_size.height);
	/*
	    in gtk3 use follow?
	    gtk_widget_get_preferred_size (tree_view, &minimum_size, &natural_size);
	*/
	int show_x, show_y;

	GtkWidget* main_window = geany_data->main_widgets->window;
	GtkWidget* notebook_widget = geany_data->main_widgets->notebook;
	ScintillaObject* sci = document_get_current()->editor->sci;

	gint x_origin, y_origin;
	GdkWindow* gdk_window = gtk_widget_get_window(GTK_WIDGET(main_window));
	gdk_window_get_origin(gdk_window, &x_origin, &y_origin);

	GtkAllocation rect_mainwindow;
	gtk_widget_get_allocation(main_window, &rect_mainwindow);
	CHECK_ARRANGE("mainwin %d %d %d %d", rect_mainwindow.x, rect_mainwindow.y,
	              rect_mainwindow.width, rect_mainwindow.height);

	GtkAllocation rect_notebook;
	gtk_widget_get_allocation(notebook_widget, &rect_notebook);
	CHECK_ARRANGE("notebook %d %d %d %d", rect_notebook.x, rect_notebook.y, rect_notebook.width,
	              rect_notebook.height);

	int note_height = 0;
	gtk_container_foreach(GTK_CONTAINER(notebook_widget), (GtkCallback)detail::get_note_height,
	                      &note_height);
	int note_tab_height = rect_notebook.height - note_height;

	int margin_width = 0;
	for (int i = 0; i < 5; i++) {
		margin_width += scintilla_send_message(sci, SCI_GETMARGINWIDTHN, i, 0);
	}
	int x_scroll_offset = scintilla_send_message(sci, SCI_GETXOFFSET, 0, 0);

	// Scintilla says "Currently all lines are the same height."
	int text_height = scintilla_send_message(sci, SCI_TEXTHEIGHT, 0, 0);

	// real line (including line wrapped)
	int top_line = scintilla_send_message(sci, SCI_GETFIRSTVISIBLELINE, 0, 0);

	// calc caret-position from top of a note
	int cur_line = sci_get_current_line(sci);  // file line (no wrap)
	int real_visible_line = 0;
	for (int i = 0; i < cur_line; i++) {
		if (sci_get_line_is_visible(sci, i)) {
			real_visible_line += scintilla_send_message(sci, SCI_WRAPCOUNT, i, 0);
		}
	}
	int diff_line = real_visible_line - top_line;

	// calc caret-position from left of a note
	int pos = sci_get_current_position(sci);
	int line = sci_get_current_line(sci);
	int ls_pos = sci_get_position_from_line(sci, line);
	int text_width = 0;
	if (ls_pos < pos) {
		int space_pix = scintilla_send_message(sci, SCI_TEXTWIDTH, STYLE_DEFAULT, (sptr_t) " ");
		int tab_pix = space_pix * sci_get_tab_width(sci);
		// g_print("space_pix %d tab_pix %d", space_pix, tab_pix);
		gchar* line_str = sci_get_contents_range(sci, ls_pos, pos);
		if (g_utf8_validate(line_str, -1, NULL)) {
			gchar* letter = NULL;
			gchar* next = line_str;
			while (next[0]) {
				letter = next;
				next = g_utf8_next_char(next);
				if (letter[0] == '\t') {
					int mod = text_width % tab_pix;
					text_width += (mod == 0 ? tab_pix : tab_pix - mod);
				} else {
					// check each character (for no monospace)
					int styleID = sci_get_style_at(sci, ls_pos + (letter - line_str));
					gchar tmp = next[0];  // for making a letter from a string
					next[0] = '\0';
					int tw = scintilla_send_message(sci, SCI_TEXTWIDTH, styleID, (sptr_t)letter);
					next[0] = tmp;  // restore from NULL
					text_width += tw;
				}
			}
		}
		g_free(line_str);
	}

	int screen_lines = scintilla_send_message(sci, SCI_LINESONSCREEN, 0, 0);

	if (0 <= diff_line && diff_line < screen_lines) {
		int total_text_height = diff_line * text_height;

		CHECK_ARRANGE("x_origin=%d notebook.x=%d margin_w=%d txt_width=%d (x_scroll_offset=%d)",
		              x_origin, rect_notebook.x, margin_width, text_width, x_scroll_offset);
		CHECK_ARRANGE("y_origin=%d notebook.y=%d note_tab_height=%d total_txt_h=%d txt_h%d",
		              y_origin, rect_notebook.y, note_tab_height, total_text_height, text_height);

		show_x = x_origin + rect_notebook.x + margin_width + text_width - x_scroll_offset;
		show_y = y_origin + rect_notebook.y + note_tab_height + total_text_height + text_height + 1;
	} else {  // out of screen
		g_print("out of screen %d %d", diff_line, screen_lines);
		show_x = 0;
		show_y = 0;
	}

	// fix arrange
	GdkScreen* gdk_screen = gdk_screen_get_default();
	int monitor_id = gdk_screen_get_monitor_at_window(gdk_screen, gdk_window);
	GdkRectangle mon_rect;
	gdk_screen_get_monitor_geometry(gdk_screen, monitor_id, &mon_rect);
	CHECK_ARRANGE("%d %d %d %d", mon_rect.x, mon_rect.y, mon_rect.width, mon_rect.height);

	// check vert
	if (show_y + sg_win_size.height > mon_rect.y + mon_rect.height) {
		show_y = show_y - text_height - sg_win_size.height - 1;
	}
	if (show_y < mon_rect.y) {
		show_y = mon_rect.y;
	}
	// check horz
	if (show_x + sg_win_size.width > mon_rect.x + mon_rect.width) {
		show_x = mon_rect.x + mon_rect.width - sg_win_size.width;
	}
	if (show_x < mon_rect.x) {
		show_x = mon_rect.x;
	}

	gtk_window_move(GTK_WINDOW(window), show_x, show_y);
}

void SuggestionWindow::setup_showing(const CodeCompletionResults& results)
{
	CHECK_ARRANGE("start suggest show %d", max_char_in_row);
	int max_signature_length = 0;
	gtk_tree_view_set_model(GTK_TREE_VIEW(tree_view), NULL);
	gtk_list_store_clear(model);

	GtkTreeIter iter;
	for (size_t i = 0; i < results.size(); i++) {
		const char* typedtext = results[i].typed_text.c_str();
		std::string labelstdstr = results[i].signature;
		if (labelstdstr.length() > max_char_in_row) {
			labelstdstr = labelstdstr.substr(0, max_char_in_row);
		}
		gtk_list_store_append(model, &iter);
		gtk_list_store_set(model, &iter, MODEL_TYPEDTEXT_INDEX, typedtext, MODEL_LABEL_INDEX,
		                   labelstdstr.c_str(), MODEL_TYPE_INDEX, icon_pixbufs[results[i].type],
		                   -1);

		if (max_signature_length < labelstdstr.length()) {
			max_signature_length = labelstdstr.length();
		}
	}

	gtk_tree_view_set_model(GTK_TREE_VIEW(tree_view), GTK_TREE_MODEL(model));
	GtkTreeViewColumn* col = gtk_tree_view_get_column(GTK_TREE_VIEW(tree_view), 1);
	gtk_tree_view_column_set_fixed_width(col, max_signature_length * character_width + 10);

	CHECK_ARRANGE("max_signature_length %d char width %d", max_signature_length, character_width);
}

void SuggestionWindow::show(const CodeCompletionResults& results, const char* initial_filter)
{
	if (!results.empty()) {
		if (this->is_showing()) {  // close and show
			this->close();
		}
		setup_showing(results);
		arrange_window();

		showing_flag = true;
		filtered_str = "";

		gtk_widget_show(tree_view);
		gtk_widget_show(window);
		/* call after gtk_widget_show */
		gtk_tree_view_scroll_to_point(GTK_TREE_VIEW(tree_view), 0, 0);

		if (initial_filter) {
			this->filter_add(initial_filter);
		}
	}
}

#include "data/sw_icon_resources.hpp"

SuggestionWindow::SuggestionWindow() : showing_flag(false)
{
	window = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_container_set_border_width(GTK_CONTAINER(window), 1);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
	gtk_widget_set_size_request(window, 150, 150);

	model = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, GDK_TYPE_PIXBUF);

	tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree_view), FALSE);

	/* create icon pixbuf
	   follow cc::CompleteResultType order */
	const guint8* pixbufs[9] = {
		sw_var_icon_pixbuf,
		sw_method_icon_pixbuf,
		sw_class_icon_pixbuf,
		sw_method_icon_pixbuf,
		sw_var_icon_pixbuf,
		sw_struct_icon_pixbuf,
		sw_namespace_icon_pixbuf,
		sw_macro_icon_pixbuf,
		sw_other_icon_pixbuf,
	};
	for (int i = 0; i < sizeof(pixbufs) / sizeof(guint8*); i++) {
		icon_pixbufs.push_back(gdk_pixbuf_new_from_inline(-1, pixbufs[i], FALSE, NULL));	
	}

	GtkCellRenderer* pixbuf_renderer = gtk_cell_renderer_pixbuf_new();
	GtkTreeViewColumn* i_column = gtk_tree_view_column_new_with_attributes(
	    "icon", pixbuf_renderer, "pixbuf", MODEL_TYPE_INDEX, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), i_column);

	GtkCellRenderer* text_renderer = gtk_cell_renderer_text_new();
	GtkTreeViewColumn* t_column = gtk_tree_view_column_new_with_attributes(
	    "label", text_renderer, "text", MODEL_LABEL_INDEX, NULL);
	g_object_set(G_OBJECT(text_renderer), "family", "Monospace", NULL);
	gtk_tree_view_column_set_sizing(t_column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), t_column);

	// get character width for determinate treeview width
	PangoFontDescription* pango_fontdesc;
	PangoLayout* pango_layout;
	g_object_get(G_OBJECT(text_renderer), "font-desc", &pango_fontdesc, NULL);
	pango_layout = gtk_widget_create_pango_layout(GTK_WIDGET(tree_view), "M");
	pango_layout_set_font_description(pango_layout, pango_fontdesc);
	pango_layout_get_pixel_size(pango_layout, &character_width, NULL);
	pango_font_description_free(pango_fontdesc);
	g_object_unref(pango_layout);

	GtkTreeSelection* select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
	gtk_tree_selection_set_mode(select, GTK_SELECTION_BROWSE);
	gtk_tree_view_set_enable_search(GTK_TREE_VIEW(tree_view), FALSE);

	gtk_container_add(GTK_CONTAINER(window), tree_view);

	// enable mouse scroll events on treeview widget
	gtk_widget_add_events(tree_view, GDK_SCROLL_MASK);

	g_signal_connect(G_OBJECT(tree_view), "row-activated", G_CALLBACK(signal_tree_selection), this);
	sig_handler_id[0] =
	    g_signal_connect(G_OBJECT(geany_data->main_widgets->window), "key-press-event",
	                     G_CALLBACK(signal_key_press_and_release), this);
	sig_handler_id[1] =
	    g_signal_connect(G_OBJECT(geany_data->main_widgets->window), "focus-out-event",
	                     G_CALLBACK(detail::close_request), this);
	sig_handler_id[2] =
	    g_signal_connect(G_OBJECT(tree_view), "scroll-event",
	                     G_CALLBACK(signal_mouse_scroll), this);
	gtk_widget_realize(tree_view);
}

SuggestionWindow::~SuggestionWindow()
{
	gtk_widget_destroy(window);
	g_signal_handler_disconnect(G_OBJECT(geany_data->main_widgets->window), sig_handler_id[0]);
	g_signal_handler_disconnect(G_OBJECT(geany_data->main_widgets->window), sig_handler_id[1]);
}

void SuggestionWindow::close()
{
	if (showing_flag) {
		gtk_widget_hide(tree_view);
		gtk_widget_hide(window);
		showing_flag = false;
	}
}

}  // end namespace geanycc
