#pragma once

#include "Area.hpp"
#include "ColorHandler.hpp"
#include "FontHandler.hpp"
#include "Module.hpp"
#include "XHandler.hpp"
#include "Singleton.hpp"

#include <functional>
#include <forward_list>
#include <map>
#include <vector>

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <poll.h>
}

namespace limebar
{
	class Bar : public singleton<Bar>
	{
		friend class singleton<Bar>;
	public:
		Bar(Bar& that) = delete;
		Bar(const Bar& that) = delete;
		Bar &operator=( Bar &rhs ) = delete;
		Bar &operator=( const Bar &rhs ) = delete;

		FontHandler  &get_font_handler ();
		ColorHandler &get_color_handler();
		XHandler     &get_X_handler    ();
		std::map< std::string, std::string > &get_labels();

		void set_permanent(bool permanent);
		void set_duplicate(bool duplicate);

		int &get_pos_x();
		Alignment &get_align();
		uint8_t &get_attr();

		bool is_init();

		void config();
		void add_config(std::function<void(XrmDatabase &db)> fn);
		void init();
		void add_init(std::function<void()> fn);
		void cleanup();
		void add_cleanup(std::function<void()> fn);
		void pre_render();
		void add_pre_render(std::function<void()> fn);
		void add_fd(const struct pollfd &fd, std::function<bool(struct pollfd &)> fn);
		void add_parse_render(char c, std::function<void(std::string &, size_t &, size_t &)> fn);
		void add_parse_non_render(char c, std::function<void(std::string &, size_t &)> fn);
		void add_module(const std::string &name);

		void loop();

		void parse_input(std::string input);

	private:
		void parse_pre_render(std::string input);
		void parse_render(std::string input);

		bool handle_input(struct pollfd &fd);
		bool handle_xcb(struct pollfd &fd);
		void handle_attr(const std::string &input, size_t &pos_i, size_t &pos_j);
		void handle_label_use(std::string &input, size_t &pos_i, size_t &pos_j);
		void handle_label_set(std::string &input, size_t &pos);
		void handle_area(std::string &input, size_t &pos_i, size_t &pos_j);

	protected:
		Bar();
		~Bar();

	private:
		FontHandler 					  			m_font_handler;
		ColorHandler 					  			m_color_handler;
		XHandler        				  			m_X_handler;

		bool            				  			m_duplicate;
		bool            				  			m_permanent;
		int             				  			m_pos_x;
		Alignment       				  			m_align;
		uint8_t         				  			m_attr;

		std::string                                 m_old_render_str;

		std::map< std::string, std::string > 		m_labels;
		std::forward_list< limebar::Area >          m_areas;
		std::forward_list< limebar::module_entry >  m_modules;

		std::forward_list< std::function<void(XrmDatabase &db)> > 	m_configs;
		std::forward_list< std::function<void()> > 					m_inits;
		std::forward_list< std::function<void()> > 					m_cleanups;
		std::forward_list< std::function<void()> > 					m_pre_render;

		std::vector<struct pollfd>									m_pollfds;
		std::map< int, std::function<bool(struct pollfd &)> >		m_pollcbs;

		std::map< char, std::function<void(std::string &, size_t &, size_t &)> > 	m_parse_render;
		std::map< char, std::function<void(std::string &, size_t &)> >         		m_parse_non_render;

		bool m_is_init;
	};
}

#define LIMEBAR limebar::Bar::instance()