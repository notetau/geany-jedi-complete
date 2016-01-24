/*
 * suggestion_window.hpp - a core code of Geany completion plugins
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

#pragma once

#include "cc_plugin.hpp"
// for CodeCompletionResults
#include "completion_base.hpp"

namespace geanycc
{

class SuggestionWindow
{
   public:
	SuggestionWindow();
	~SuggestionWindow();

	void filter_add(int ch);
	void filter_add(const std::string& str);
	void filter_backspace();

	void show(const CodeCompletionResults& results, const char* initial_filter = NULL);
	// void show_with_filter(const cc::CodeCompletionResults& results, const std::string& filter);
	void close();

	bool is_showing() const { return showing_flag; }
	void arrange_window();

	int set_max_char_in_row(int num) { max_char_in_row = num; }
	int set_max_window_height(int px) { max_window_height = px; }

   private:
	GtkWidget* window;
	GtkWidget* tree_view;
	GtkListStore* model;

	std::vector<GdkPixbuf*> icon_pixbufs;

	bool showing_flag;

	std::string filtered_str;
	int pos_start;

	int max_char_in_row = 100;  /// maximum suggestion text (letters)
	int max_window_height = 300; /// suggestion window height (px)

	int character_width;  // for calc treeview width

	int sig_handler_id[3];  // for need disconnect event

	static gboolean signal_key_press_and_release(GtkWidget* widget, GdkEventKey* event,
	                                             SuggestionWindow* self);
	static gboolean signal_mouse_scroll(GtkWidget* widget, GdkEventScroll* event,
	                                             SuggestionWindow* self);
	static void signal_tree_selection(GtkTreeView* tree_view, GtkTreePath* path,
	                                  GtkTreeViewColumn* column, SuggestionWindow* self);

	void move_cursor(bool down);
	void select_suggestion();
	void do_filtering();

	void setup_showing(const CodeCompletionResults& results);
};
} // namespace geanycc
