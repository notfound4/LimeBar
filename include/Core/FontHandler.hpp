#pragma once

#include <map>

extern "C" {
#include <pango/pangocairo.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
}

namespace limebar
{
	class FontHandler
	{
	public:
		FontHandler();
		~FontHandler();

		FontHandler(FontHandler& that) = delete;
		FontHandler(const FontHandler& that) = delete;
		FontHandler &operator=( FontHandler &rhs ) = delete;
		FontHandler &operator=( const FontHandler &rhs ) = delete;

		unsigned int get_font_height();
		unsigned int get_max_font_height();
		int  get_descent(PangoContext *pango_context);
		void set_font(const std::string &font_name, bool is_default = false);
		PangoFontDescription *get_current_font();
		PangoFontDescription *get_font(const std::string &font_name);
		void reset_to_default();
		void config(XrmDatabase &db);

		void handle_font(std::string &input, size_t &pos_i, size_t &pos_j);

	private:
		std::map< std::string, PangoFontDescription* > m_fonts;
		std::map< PangoFontDescription*, int >         m_descents;
		PangoFontDescription 						  *m_default_font_desc;
		PangoFontDescription 						  *m_current_font_desc;
	};
}