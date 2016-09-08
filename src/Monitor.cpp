#include "Monitor.hpp"
#include "Resource.hpp"

#include <algorithm>
#include <iostream>

#include <gdk-pixbuf/gdk-pixbuf.h>

Monitor::Monitor()
{
	m_rect.width = m_rect.height = m_rect.x = m_rect.y = 0;
}

Monitor::Monitor(const xcb_rectangle_t &rect)
{
	m_rect.x = rect.x;
	m_rect.y = (Resource::instance().m_bottom ? rect.height - Resource::instance().m_height - Resource::instance().m_y : Resource::instance().m_y) + rect.y;
	m_rect.width = rect.width;
	m_rect.height = Resource::instance().m_height;

	m_window = xcb_generate_id(Resource::instance().m_connection);
	uint32_t w_mask  = XCB_CW_BACK_PIXEL | XCB_CW_BORDER_PIXEL | XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK | XCB_CW_COLORMAP;
	uint32_t w_val[] = { Resource::instance().m_current_background.v, Resource::instance().m_current_background.v, Resource::instance().m_dock,
            	XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS, Resource::instance().m_colormap };
    xcb_create_window(Resource::instance().m_connection, Resource::instance().m_depth, m_window, Resource::instance().m_screen->root,
    	m_rect.x, m_rect.y, m_rect.width, Resource::instance().m_height, 0,
            XCB_WINDOW_CLASS_INPUT_OUTPUT, Resource::instance().m_visualtype->visual_id,
            w_mask, w_val);

    m_pixmap = xcb_generate_id(Resource::instance().m_connection);
    xcb_create_pixmap(Resource::instance().m_connection, Resource::instance().m_depth, m_pixmap, m_window, m_rect.width, Resource::instance().m_height);

    m_cairo_surface = cairo_xcb_surface_create(Resource::instance().m_connection, m_pixmap, Resource::instance().m_visualtype, m_rect.width, m_rect.height);
    m_cairo = cairo_create(m_cairo_surface);

	m_pango_context = pango_cairo_create_context(m_cairo);
    m_pango_layout  = pango_layout_new(m_pango_context);
}

Monitor::~Monitor()
{
	cairo_destroy(m_cairo);
	cairo_surface_destroy(m_cairo_surface);
	g_object_unref(m_pango_layout);
	g_object_unref(m_pango_context);
	if (Resource::instance().m_connection != nullptr)
	{
		xcb_destroy_window(Resource::instance().m_connection, m_window);
	    xcb_free_pixmap(Resource::instance().m_connection, m_pixmap);
	}
}

void Monitor::draw_image(const std::string &image)
{
	GError *gerr =NULL;
	GdkPixbuf *gbuf = gdk_pixbuf_new_from_file(image.c_str(), &gerr);
	if(gbuf == NULL) {
		std::cerr << "Cannot allocate pixbuf for image " << image << ": " << gerr->message << std::endl;
		return;
	}

	uint16_t width = gdk_pixbuf_get_width(gbuf);
    uint16_t height = gdk_pixbuf_get_height(gbuf);
    int pix_stride = gdk_pixbuf_get_rowstride(gbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(gbuf);
    int channels = gdk_pixbuf_get_n_channels(gbuf);
    cairo_surface_t *surface;
    int cairo_stride;
    unsigned char *cairo_pixels;


    cairo_format_t format = CAIRO_FORMAT_ARGB32;
    if (channels == 3)
        format = CAIRO_FORMAT_RGB24;

    surface = cairo_image_surface_create(format, width, height);
    cairo_surface_flush(surface);
    cairo_stride = cairo_image_surface_get_stride(surface);
    cairo_pixels = cairo_image_surface_get_data(surface);

    for (int y = 0; y < height; y++) {
        guchar *row = pixels;
        uint32_t *cairo = (uint32_t *) cairo_pixels;
        for (int x = 0; x < width; x++) {
            if (channels == 3) {
                uint8_t r = *row++;
                uint8_t g = *row++;
                uint8_t b = *row++;
                *cairo++ = (r << 16) | (g << 8) | b;
            }
            else {
                uint8_t r = *row++;
                uint8_t g = *row++;
                uint8_t b = *row++;
                uint8_t a = *row++;
                double alpha = a / 255.0;
                r = r * alpha;
                g = g * alpha;
                b = b * alpha;
                *cairo++ = (a << 24) | (r << 16) | (g << 8) | b;
            }
        }
        pixels += pix_stride;
        cairo_pixels += cairo_stride;
    }

    cairo_surface_mark_dirty(surface);

	g_object_unref(gbuf);

	float scale_factor = static_cast<float>(Resource::instance().get_max_font_height()) / static_cast<float>(height);
	int16_t x = shift(width*scale_factor), y = (m_rect.height - Resource::instance().get_max_font_height()) / 2;
	cairo_scale(m_cairo, scale_factor, scale_factor);

	cairo_set_source_surface(m_cairo, surface, x/scale_factor, y/scale_factor);
	cairo_paint(m_cairo);
	cairo_surface_destroy(surface);
	cairo_scale(m_cairo, 1.0/scale_factor, 1.0/scale_factor);

    draw_lines(width*scale_factor, x);
    area_shift(width*scale_factor);
	Resource::instance().m_pos_x += width*scale_factor;
}

void Monitor::draw_vertical_line()
{
	int16_t x = shift(Resource::instance().m_underline);

	xcb_rectangle_t rect;
	rect.x = x;
	rect.y = 0;
	rect.width = Resource::instance().m_underline;
	rect.height = m_rect.height;
	fill_rectangle(Resource::instance().m_gc[GC_ATTR], rect);
}

void Monitor::draw_lines(uint16_t width, int x)
{
	xcb_rectangle_t rect;
	rect.x = x;
	rect.width = width;
	rect.height = Resource::instance().m_underline;

	if (Resource::instance().m_attr & ATTR_OVERLINE)
	{
		rect.y = 0;
		fill_rectangle(Resource::instance().m_gc[GC_ATTR], rect);
	}
    if (Resource::instance().m_attr & ATTR_UNDERLINE)
    {
		rect.y = m_rect.height - Resource::instance().m_underline;
		fill_rectangle(Resource::instance().m_gc[GC_ATTR], rect);
    }
}

void Monitor::draw_shift(uint16_t width)
{
	int16_t x = shift(width);
    draw_lines(width, x);
}

int16_t Monitor::shift (uint16_t str_width)
{
	int16_t x;
    switch (Resource::instance().m_align) {
        case ALIGN_C:
            xcb_copy_area(Resource::instance().m_connection, m_pixmap, m_pixmap, Resource::instance().m_gc[GC_DRAW],
                    m_rect.width / 2 - Resource::instance().m_pos_x / 2, 0,
                    m_rect.width / 2 - (Resource::instance().m_pos_x + str_width) / 2, 0,
                    Resource::instance().m_pos_x, m_rect.height);
            x += m_rect.width / 2 + (Resource::instance().m_pos_x - str_width) / 2;
            break;
        case ALIGN_R:
            xcb_copy_area(Resource::instance().m_connection, m_pixmap, m_pixmap, Resource::instance().m_gc[GC_DRAW],
                    m_rect.width - Resource::instance().m_pos_x, 0,
                    m_rect.width - Resource::instance().m_pos_x - str_width, 0,
                    Resource::instance().m_pos_x, m_rect.height);
            x = m_rect.width - str_width;
            break;
        default:
        	x = Resource::instance().m_pos_x;
    }

    // Draw the background first
    xcb_rectangle_t clear_rect = {x, 0, str_width, m_rect.height};
    fill_rectangle(Resource::instance().m_gc[GC_CLEAR], clear_rect);
    return x;
}

void Monitor::draw_string(const std::string &str)
{
	rgba_t &color = Resource::instance().m_current_foreground;
	cairo_set_source_rgba(m_cairo, get_R(color), get_G(color), get_B(color), get_A(color));

	pango_layout_set_text(m_pango_layout, str.c_str(), -1);
	pango_layout_set_font_description(m_pango_layout, Resource::instance().m_font_desc);

	int wd, ht;
	pango_layout_get_pixel_size(m_pango_layout, &wd, &ht);

	int16_t x = shift(wd), y = (m_rect.height - Resource::instance().get_font_height()) / 2 - Resource::instance().get_descent(m_pango_context);
	cairo_move_to(m_cairo, x, y);
	pango_cairo_update_layout(m_cairo, m_pango_layout);
	pango_cairo_show_layout(m_cairo, m_pango_layout);

    draw_lines(wd, x);
    area_shift(wd);
	Resource::instance().m_pos_x += wd;
}

void Monitor::clear()
{
	xcb_rectangle_t rect = {.x=0, .y=0, .width=m_rect.width, .height=m_rect.height};
	xcb_poly_fill_rectangle(Resource::instance().m_connection, m_pixmap, Resource::instance().m_gc[GC_CLEAR], 1, &rect);

	m_areas.clear();
}

void Monitor::fill_rectangle(const xcb_gcontext_t &gc, const xcb_rectangle_t &rect)
{
	xcb_poly_fill_rectangle(Resource::instance().m_connection, m_pixmap, gc, 1, &rect);
}

void Monitor::redraw(const xcb_gcontext_t &gc)
{
	xcb_copy_area(Resource::instance().m_connection, m_pixmap, m_window, gc, 0, 0, 0, 0, m_rect.width, m_rect.height);
}

const xcb_rectangle_t &Monitor::get_dimensions() const
{
	return m_rect;
}

const xcb_window_t &Monitor::get_window() const
{
	return m_window;
}

const xcb_pixmap_t &Monitor::get_pixmap() const
{
	return m_pixmap;
}

void Monitor::on_click(uint16_t x, uint8_t button)
{
	std::forward_list<Area>::iterator it = std::find_if (m_areas.begin(), m_areas.end(), [&](const Area &area){return area.is_valid(x, button);});
	if (it != m_areas.end())
	{
		std::cout << *it << std::endl;
	}
}

void Monitor::area_shift(uint16_t delta)
{
	switch (Resource::instance().m_align)
	{
		case ALIGN_L: return;
		case ALIGN_C: delta /= 2;
	}

	for ( Area &area : m_areas )
	{
		area.shift(delta);
	}
}

bool Monitor::add_area(uint8_t button, uint16_t begin, uint16_t end, Alignment align, const std::string &command, bool print_pos)
{
	m_areas.emplace_front(button, begin, end, align, command, print_pos);
}