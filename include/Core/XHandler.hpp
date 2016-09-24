#pragma once

#include "Monitor.hpp"
#include "Types.hpp"

#include <map>
#include <string>

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <xcb/xcb.h>
}

#define DEFAULT_Y_OFFSET 4

namespace limebar
{
	class XHandler
	{
		friend class Monitor;
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
		XHandler();
		~XHandler();

		XHandler(XHandler& that) = delete;
		XHandler(const XHandler& that) = delete;
		XHandler &operator=( XHandler &rhs ) = delete;
		XHandler &operator=( const XHandler &rhs ) = delete;

		std::map< std::string, Monitor >::iterator &get_current_monitor();
		std::map< std::string, Monitor > &get_monitors();
		int get_fd();
		bool connection_has_error();
		void flush();
		xcb_generic_event_t *get_event();

		void set_bottom(bool bottom);
		void set_dock(bool dock);
		void set_underline(unsigned int underline);
		void set_wm_name(const std::string &name);

		void init();
		void cleanup();
		void set_geometry(const std::string &source);
		void reset_to_default();
		void update_gc();
		void redraw_all();
		void clear_all();
		void config(XrmDatabase &db);

		void draw_string(std::string text);
		void draw_vertical_line();
		void draw_monitor();

		void handle_monitor(std::string &input, size_t &pos_i, size_t &pos_j);
		void handle_image(std::string &input, size_t &pos_i, size_t &pos_j);
		void handle_offset(std::string &input, size_t &pos_i, size_t &pos_j);
	
	private:
		xcb_visualtype_t *get_visualtype();
		void init_ewmh();
		void init_randr();
		void create_monitors(const std::map< std::string, xcb_rectangle_t > &rects_map);

	private:
		int                  					 m_width;
		int                  					 m_height;
		int                  					 m_x;
		int                  					 m_y;
		bool                 					 m_bottom;
		bool                 					 m_dock;
		std::string          					 m_wm_name;
		unsigned int         					 m_underline;
		std::map<std::string, Monitor>::iterator m_default_monitor;
		std::map<std::string, Monitor>::iterator m_current_monitor;

		xcb_gcontext_t                           m_gc[GC_MAX];
		xcb_connection_t                        *m_connection;
		xcb_screen_t                     		*m_screen;
		xcb_visualtype_t                 		*m_visualtype;
		uint8_t                                  m_depth;
		xcb_colormap_t                           m_colormap;

		std::map< std::string, Monitor >         m_monitors;
	};
}