#pragma once

extern "C" {
#include <xcb/xcb.h>
}

#include "Monitor.hpp"
#include "Singleton.hpp"
#include "Types.hpp"

#include <map>
#include <string>

#include <pango/pangocairo.h>

#define DEFAULT_Y_OFFSET 4

double get_R(const rgba_t &color);
double get_G(const rgba_t &color);
double get_B(const rgba_t &color);
double get_A(const rgba_t &color);

namespace limebar {

class Resource : public singleton<Resource> {
	friend class singleton<Resource>;
private:
	enum {
	    NET_WM_WINDOW_TYPE,
	    NET_WM_WINDOW_TYPE_DOCK,
	    NET_WM_DESKTOP,
	    NET_WM_STRUT_PARTIAL,
	    NET_WM_STRUT,
	    NET_WM_STATE,
	    NET_WM_STATE_STICKY,
	    NET_WM_STATE_ABOVE,
	};

public:
	Resource(Resource& that) = delete;
	Resource(const Resource& that) = delete;
	Resource &operator=( Resource &rhs ) = delete;
	Resource &operator=( const Resource &rhs ) = delete;

	void init();
	void cleanup();

	unsigned int get_font_height();
	unsigned int get_max_font_height();
	int  get_descent(PangoContext *pango_context);

	void set_foreground(const std::string &source, bool is_default = false);
	void set_background(const std::string &source, bool is_default = false);
	void set_underline (const std::string &source, bool is_default = false);
	void switch_colors();
	void set_font(const std::string &source, bool is_default = false);
	void set_geometry(const std::string &source);

	void reset_to_default();
	void update_gc();

	void redraw_all();
	void clear_all();

protected:
	Resource();

private:
	void set_color(rgba_t &destination, const std::string &source);
	xcb_visualtype_t *get_visualtype();
	void init_ewmh();
	void init_randr();
	void create_monitors(const std::map< std::string, xcb_rectangle_t > &rects_map);

public:
	rgba_t               					 m_default_foreground;
	rgba_t               					 m_default_background;
	rgba_t               					 m_default_underline;
	PangoFontDescription 					*m_default_font_desc;
	int                  					 m_width;
	int                  					 m_height;
	int                  					 m_x;
	int                  					 m_y;
	std::string          					 m_command;
	bool                 					 m_bottom;
	bool                 					 m_dock;
	bool                 					 m_permanent;
	std::string          					 m_wm_name;
	unsigned int         					 m_underline;
	std::map<std::string, Monitor>::iterator m_default_monitor;

	rgba_t               					 m_current_foreground;
	rgba_t               					 m_current_background;
	rgba_t               					 m_current_underline;
	PangoFontDescription 					*m_font_desc;
	std::map<std::string, Monitor>::iterator m_current_monitor;
	int                   					 m_pos_x;
	Alignment             					 m_align;
	xcb_gcontext_t                           m_gc[GC_MAX];
	uint8_t                                  m_attr;

	xcb_connection_t                        *m_connection;
	xcb_screen_t                     		*m_screen;
	xcb_visualtype_t                 		*m_visualtype;
	uint8_t                                  m_depth;
	xcb_colormap_t                           m_colormap;

	std::map< std::string, Monitor >               m_monitors;
	std::map< std::string, PangoFontDescription* > m_fonts;
	std::map< PangoFontDescription*, int >         m_descents;

};

}