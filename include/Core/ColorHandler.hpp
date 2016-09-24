#pragma once

#include "Types.hpp"

#include <string>

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xresource.h>
}

namespace limebar
{
	class ColorHandler
	{
	public:
		ColorHandler();

		ColorHandler(ColorHandler& that) = delete;
		ColorHandler(const ColorHandler& that) = delete;
		ColorHandler &operator=( ColorHandler &rhs ) = delete;
		ColorHandler &operator=( const ColorHandler &rhs ) = delete;

		rgba_t &get_foreground();
		rgba_t &get_background();
		rgba_t &get_underline ();
		void set_foreground(const std::string &source, bool is_default = false);
		void set_background(const std::string &source, bool is_default = false);
		void set_underline (const std::string &source, bool is_default = false);
		void switch_colors();
		void reset_to_default();
		void config(XrmDatabase &db);

		void handle_foreground(std::string &input, size_t &pos_i, size_t &pos_j);
		void handle_background(std::string &input, size_t &pos_i, size_t &pos_j);
		void handle_underline (std::string &input, size_t &pos_i, size_t &pos_j);

	private:
		void set_color(rgba_t &destination, const std::string &source);
		void handle_color(rgba_t &destination, rgba_t &def, std::string &input, size_t &pos_i, size_t &pos_j);

	private:
		rgba_t 	m_default_foreground;
		rgba_t 	m_default_background;
		rgba_t 	m_default_underline;
		rgba_t 	m_current_foreground;
		rgba_t 	m_current_background;
		rgba_t 	m_current_underline;
	};
}