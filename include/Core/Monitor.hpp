#pragma once

#include <forward_list>
#include <string>

extern "C" {
#include <cairo-xcb.h>
#include <pango/pangocairo.h>
#include <xcb/xcb.h>
}

#include "Area.hpp"

namespace limebar {

class Resource;

class Monitor
{
public:
	explicit Monitor(const xcb_rectangle_t &rect);
	~Monitor();

	Monitor(Monitor& that) = delete;
	Monitor(const Monitor& that) = delete;
	Monitor &operator=( Monitor &rhs ) = delete;
	Monitor &operator=( const Monitor &rhs ) = delete;

	void draw_image(const std::string &image);
	void draw_vertical_line();
	void draw_lines(uint16_t width, int x);
	void draw_shift(uint16_t width);
	void draw_string(const std::string &str);
	void clear();
	void fill_rectangle(const xcb_gcontext_t &gc, const xcb_rectangle_t &rect);
	void redraw(const xcb_gcontext_t &gc);
	int16_t shift(uint16_t str_width);

	const xcb_rectangle_t &get_dimensions() const;
	const xcb_window_t &get_window() const;
	const xcb_pixmap_t &get_pixmap() const;

	void on_click(uint16_t x, uint8_t button);

	void add_area(std::forward_list<Area> &areas);
	void area_shift(uint16_t delta);

private:
	xcb_rectangle_t   				  m_rect;
    xcb_window_t      				  m_window;
    xcb_pixmap_t      				  m_pixmap;

    cairo_surface_t  				 *m_cairo_surface;
    cairo_t          				 *m_cairo;
    PangoLayout                      *m_pango_layout;
    PangoContext                     *m_pango_context;

    std::forward_list<Area>           m_areas;
};

}