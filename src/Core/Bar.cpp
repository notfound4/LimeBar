#include "Bar.hpp"

#include "Area.hpp"
#include "DatabaseHelper.hpp"
#include "Types.hpp"

#include <iostream>
#include <sstream>

#include <boost/filesystem.hpp>

extern "C" {
#include <dlfcn.h>
}

#define PERMANENT false
#define DUPLICATE false

boost::filesystem::path get_user() {
	char const* home = getenv("HOME");

	if ( home or ( home = getenv("USERPROFILE") ) )
		return boost::filesystem::path(home);
	else
	{
		char const *hdrive = getenv("HOMEDRIVE");
        char const *hpath  = getenv("HOMEPATH");
		return boost::filesystem::path(std::string(hdrive) + hpath);
	}
}

limebar::Bar::Bar()
{
	m_permanent = PERMANENT;
	m_duplicate = DUPLICATE;

	add_init( [&]{m_X_handler.init();} );

	add_cleanup( [&]{m_X_handler.cleanup();} );

	add_pre_render( [&]{m_pos_x = 0; m_attr = 0; m_align = ALIGN_L; m_areas.clear();} );
	add_pre_render( [&]{m_X_handler.reset_to_default();} );
	add_pre_render( [&]{m_font_handler.reset_to_default();} );
	add_pre_render( [&]{m_color_handler.reset_to_default();} );

	add_config( [&](XrmDatabase &db){ limebar::DatabaseHelper::get_boolean( db, "limebar.permanent", "limebar.permanent", m_permanent );
		limebar::DatabaseHelper::get_boolean( db, "limebar.duplicate", "limebar.duplicate", m_duplicate ); } );
	add_config( [&](XrmDatabase &db){ m_color_handler.config(db); } );
	add_config( [&](XrmDatabase &db){ m_font_handler.config(db); } );
	add_config( [&](XrmDatabase &db){ m_X_handler.config(db); } );

	add_fd({ .fd = STDIN_FILENO, .events = POLLIN }, [&](struct pollfd &fd){return handle_input(fd);});
	add_fd({ .fd = m_X_handler.get_fd(), .events = POLLIN }, [&](struct pollfd &fd){return handle_xcb(fd);});

	add_parse_non_render('L', [&](std::string &input, size_t &pos){handle_label_set(input, pos);} );

	add_parse_render('|', [&](std::string &input, size_t &pos_i, size_t &pos_j){m_X_handler.draw_vertical_line(); pos_i++;} );
	add_parse_render('+', [&](std::string &input, size_t &pos_i, size_t &pos_j){handle_attr(input, pos_i, pos_j);} );
	add_parse_render('-', [&](std::string &input, size_t &pos_i, size_t &pos_j){handle_attr(input, pos_i, pos_j);} );
	add_parse_render('!', [&](std::string &input, size_t &pos_i, size_t &pos_j){handle_attr(input, pos_i, pos_j);} );
	add_parse_render('m', [&](std::string &input, size_t &pos_i, size_t &pos_j){m_X_handler.draw_monitor(); pos_i++;} );
	add_parse_render('R', [&](std::string &input, size_t &pos_i, size_t &pos_j){m_color_handler.switch_colors(); m_X_handler.update_gc(); pos_i++;} );
	add_parse_render('l', [&](std::string &input, size_t &pos_i, size_t &pos_j){m_pos_x = 0; m_align = ALIGN_L; pos_i++;} );
	add_parse_render('c', [&](std::string &input, size_t &pos_i, size_t &pos_j){m_pos_x = 0; m_align = ALIGN_C; pos_i++;} );
	add_parse_render('r', [&](std::string &input, size_t &pos_i, size_t &pos_j){m_pos_x = 0; m_align = ALIGN_R; pos_i++;} );
	add_parse_render('T', [&](std::string &input, size_t &pos_i, size_t &pos_j){m_font_handler.handle_font(input, pos_i, pos_j);} );
	add_parse_render('S', [&](std::string &input, size_t &pos_i, size_t &pos_j){m_X_handler.handle_monitor(input, pos_i, pos_j);} );
	add_parse_render('I', [&](std::string &input, size_t &pos_i, size_t &pos_j){m_X_handler.handle_image(input, pos_i, pos_j);} );
	add_parse_render('F', [&](std::string &input, size_t &pos_i, size_t &pos_j){m_color_handler.handle_foreground(input, pos_i, pos_j); m_X_handler.update_gc();} );
	add_parse_render('B', [&](std::string &input, size_t &pos_i, size_t &pos_j){m_color_handler.handle_background(input, pos_i, pos_j); m_X_handler.update_gc();} );
	add_parse_render('U', [&](std::string &input, size_t &pos_i, size_t &pos_j){m_color_handler.handle_underline (input, pos_i, pos_j); m_X_handler.update_gc();} );
	add_parse_render('O', [&](std::string &input, size_t &pos_i, size_t &pos_j){m_X_handler.handle_offset(input, pos_i, pos_j);} );
	add_parse_render('L', [&](std::string &input, size_t &pos_i, size_t &pos_j){handle_label_use(input, pos_i, pos_j);} );
	add_parse_render('A', [&](std::string &input, size_t &pos_i, size_t &pos_j){handle_area(input, pos_i, pos_j);} );
}

limebar::Bar::~Bar()
{
	for (std::forward_list< limebar::module_entry >::iterator it = m_modules.begin(); it != m_modules.end(); ++it)
	{
		it->m_destroyer(it->m_module);
		dlclose(it->m_library);
	}
}

limebar::FontHandler &limebar::Bar::get_font_handler()
{
	return m_font_handler;
}

limebar::ColorHandler &limebar::Bar::get_color_handler()
{
	return m_color_handler;
}

limebar::XHandler &limebar::Bar::get_X_handler()
{
	return m_X_handler;
}

int &limebar::Bar::get_pos_x()
{
	return m_pos_x;
}

Alignment &limebar::Bar::get_align()
{
	return m_align;
}

uint8_t &limebar::Bar::get_attr()
{
	return m_attr;
}

void limebar::Bar::set_permanent(bool permanent)
{
	m_permanent = permanent;
}

void limebar::Bar::set_duplicate(bool duplicate)
{
	m_duplicate = duplicate;
}

void limebar::Bar::config()
{
	std::string path_to_resource;
	try
	{
		path_to_resource = boost::filesystem::canonical(".config/limebar/limebarrc", get_user()).string();
	}
	catch(...)
	{
        std::cerr << "limebarrc not found" << std::endl;
		return;
	}

    XrmInitialize();
    XrmDatabase db = XrmGetFileDatabase(path_to_resource.c_str());
    if (db == NULL)
    {
        std::cerr << "limebarrc not found" << std::endl;
        return;
    }

	for (auto &fn : m_configs)
	{
		fn(db);
	}

	XrmDestroyDatabase(db);
}

void limebar::Bar::add_config(std::function<void(XrmDatabase &db)> fn)
{
	m_configs.push_front(fn);
}

void limebar::Bar::init()
{
	for (auto &fn : m_inits)
	{
		fn();
	}
}

void limebar::Bar::add_init(std::function<void()> fn)
{
	m_inits.push_front(fn);
}

void limebar::Bar::cleanup()
{
	for (auto &fn : m_cleanups)
	{
		fn();
	}
}

void limebar::Bar::add_cleanup(std::function<void()> fn)
{
	m_cleanups.push_front(fn);
}

void limebar::Bar::pre_render()
{
	for (auto &fn : m_pre_render)
	{
		fn();
	}
}

void limebar::Bar::add_pre_render(std::function<void()> fn)
{
	m_pre_render.push_front(fn);
}

void limebar::Bar::add_fd(const struct pollfd &fd, std::function<bool(struct pollfd &)> fn)
{
	m_pollfds.push_back(fd);
	m_pollcbs[fd.fd] = fn;
}

void limebar::Bar::add_parse_render(char c, std::function<void(std::string &, size_t &, size_t &)> fn)
{
	m_parse_render[c] = fn;
}

void limebar::Bar::add_parse_non_render(char c, std::function<void(std::string &, size_t &)> fn)
{
	m_parse_non_render[c] = fn;
}

void limebar::Bar::add_module(const std::string &name)
{
	limebar::module_entry entry;

    // load the triangle library
	entry.m_library = dlopen(name.c_str(), RTLD_NOW);
    if (!entry.m_library) {
        std::cerr << "Cannot load library: " << dlerror() << std::endl;
        return;
    }

    // reset errors
    dlerror();
    
    // load the symbols
    entry.m_creator = (create_module_t*) dlsym(entry.m_library, "create");
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        std::cerr << "Cannot load symbol create: " << dlsym_error << std::endl;
        dlclose(entry.m_library);
        return;
    }
    
    entry.m_destroyer = (destroy_module_t*) dlsym(entry.m_library, "destroy");
    dlsym_error = dlerror();
    if (dlsym_error) {
        std::cerr << "Cannot load symbol destroy: " << dlsym_error << std::endl;
        dlclose(entry.m_library);
        return;
    }

    // create an instance of the class
    entry.m_module = entry.m_creator();

    m_modules.push_front(entry);
	add_init( [&]{entry.m_module->init();} );
	add_cleanup( [&]{entry.m_module->cleanup();} );
}

void limebar::Bar::loop()
{
	for (;;) {
        bool redraw = false;

        if (m_X_handler.connection_has_error())
            break;

        if ( poll(&m_pollfds[0], m_pollfds.size(), -1) > 0 )
        {
        	for (int i=0; i<m_pollfds.size(); ++i)
        	{
        		if (m_pollfds[i].revents)
        		{
        			redraw = m_pollcbs[m_pollfds[i].fd](m_pollfds[i]);
        		}
        	}
        }

        /*if (poll(m_pollin, 3, -1) > 0) {
            if (m_pollin[1].revents & POLLIN) { // The event comes from i3
        	    m_conn.handle_event();
                redraw = true;
        	}
        }*/

        if (redraw)	m_X_handler.redraw_all();

        m_X_handler.flush();
    }
}

bool limebar::Bar::handle_input(struct pollfd &fd)
{
	bool redraw = false;
	if (fd.revents & POLLHUP) {      // No more data...
        if (m_permanent) fd.fd = -1;   // ...null the fd and continue polling :D
        else exit(EXIT_SUCCESS);                         // ...bail out
    }
    if (fd.revents & POLLIN) { // New input, process it
    	std::string message;
    	if ( !std::getline (std::cin, message).good() )
			exit(EXIT_FAILURE);
		parse_input(message);
        redraw = true;
    }
    return redraw;
}

bool limebar::Bar::handle_xcb(struct pollfd &fd)
{
	bool redraw = false;


    if (fd.revents & POLLIN) { // The event comes from the Xorg server
		xcb_generic_event_t *ev;
    	xcb_button_press_event_t *press_ev;
        while ( (ev = m_X_handler.get_event()) ) {
            xcb_expose_event_t *expose_ev = (xcb_expose_event_t *)ev;

            switch (ev->response_type & 0x7F) {
                case XCB_EXPOSE:
                    if (expose_ev->count == 0)
                        redraw = true;
                    break;
                case XCB_BUTTON_PRESS:
                    press_ev = (xcb_button_press_event_t *)ev;
                    {
                    	std::map< std::string, limebar::Monitor >::iterator it =
                    		std::find_if (m_X_handler.get_monitors().begin(), m_X_handler.get_monitors().end(),
                    			[&](const std::pair<const std::string, limebar::Monitor> &pair){return press_ev->event == pair.second.get_window();});
                    	it->second.on_click(press_ev->event_x, press_ev->detail);
                    }
                    break;
            }

            free(ev);
        }
    }
    return redraw;
}

void limebar::Bar::handle_attr(const std::string &input, size_t &pos_i, size_t &pos_j)
{
	if ( ++pos_i == pos_j ) return;

	uint8_t attr;
	switch ( input[pos_i] )
	{
		case 'u': attr = ATTR_UNDERLINE; break;
		case 'o': attr = ATTR_OVERLINE; break;
		default: return;
	}
	switch ( input[pos_i++ - 1] )
	{
		case '+': m_attr |=  attr; break;
		case '-': m_attr &= ~attr; break;
		case '!': m_attr ^=  attr; break;
	}
}

void limebar::Bar::handle_label_use(std::string &input, size_t &pos_i, size_t &pos_j)
{
	if (++pos_i == pos_j or input[pos_i] != ':' or ++pos_i == pos_j) return;
    
    size_t pos_k = find_non_escaped(input, ":", pos_i, pos_j);
    if (pos_k >= pos_j)
    {
        pos_i = pos_j;
        return;
    }

    std::map< std::string, std::string >::iterator label_it = m_labels.find(input.substr(pos_i, pos_k - pos_i));
    if ( label_it != m_labels.end() )
        parse_render(label_it->second);

    pos_i = pos_k + 1;
}

void limebar::Bar::handle_label_set(std::string &input, size_t &pos)
{
	if (++pos == input.size() or input[pos] != ':' or ++pos == input.size()) return;
    
    size_t pos_i = find_non_escaped(input, ":", pos);
    if (pos_i == std::string::npos)
    {
        pos = std::string::npos;
        return;
    }

    size_t pos_j = find_non_escaped(input, ":", pos_i + 1);
    if (pos_j == std::string::npos)
    {
        pos = std::string::npos;
        return;
    }

    m_labels[input.substr(pos, pos_i - pos)] = input.substr(pos_i+1, pos_j - pos_i - 1);
    pos = pos_j + 1;
}

void limebar::Bar::handle_area(std::string &input, size_t &pos_i, size_t &pos_j)
{
	uint8_t     button = XCB_BUTTON_INDEX_1;
    bool		print_pos;
    size_t      pos_k;
             
    pos_i++;
	if ( !isdigit(input[pos_i]) and input[pos_i] != 'W' and input[pos_i] != ':' ) goto close_area;

	if ( isdigit(input[pos_i]) )
        button = input[pos_i++] - '0';

    if (input[pos_i] == 'W') {
        print_pos = true;
        pos_i++;
    }
    else
    	print_pos = false;

    if (input[pos_i++] != ':')
    {
    	pos_i = pos_j;
    	return;
    }
    
    pos_k = find_non_escaped(input, ":", pos_i, pos_j);
    if (pos_k >= pos_j)
    {
        pos_i = pos_j;
        return;
    }

    m_areas.emplace_front(button, m_pos_x, m_align, m_X_handler.get_current_monitor(), input.substr(pos_i, pos_k - pos_i), print_pos);

    pos_i = pos_k + 1;
    return;

close_area:
    std::cout << "Test" << std::endl;
	if (m_areas.empty()) return;
	if ( m_areas.begin()->complete(m_pos_x, m_align, m_X_handler.get_current_monitor()) )
	{
    	std::cout << "add area" << std::endl;
		m_X_handler.get_current_monitor()->second.add_area(m_areas);
	}
	else
	{
    	std::cout << "delete area" << std::endl;
		m_areas.pop_front();
	}
}

void limebar::Bar::parse_input(std::string input)
{
	std::string pre_render_str, render_str;
	size_t pos_i = 0, pos_j = find_non_escaped(input, "#[", 0);

	while (pos_j != std::string::npos)
	{
		render_str.append( input.substr(pos_i, pos_j - pos_i) );

		pos_i = pos_j;
		pos_j = find_non_escaped(input, "]", pos_i);
		if (pos_j == std::string::npos) return;

		pre_render_str.append( input.substr(pos_i+2, (pos_j++) - pos_i - 2) );

		pos_i = pos_j;
		pos_j = find_non_escaped(input, "#[", pos_i);
	}

	if (pos_i < input.size())
		render_str.append( input.substr(pos_i) );

	parse_pre_render(pre_render_str);
    pre_render();
    m_X_handler.clear_all();

	if ( !(render_str.empty() or m_duplicate) )
        m_old_render_str = render_str;
    else if (m_duplicate)
    {
    	m_old_render_str = "";
  		for (std::map<std::string, limebar::Monitor>::iterator it=m_X_handler.get_monitors().begin(); it!=m_X_handler.get_monitors().end(); ++it)
    	{
    	    std::string cpy = render_str;
			pos_i = find_non_escaped(cpy, "%s", 0);
			while (pos_i != std::string::npos)
			{
				cpy.replace(pos_i, 2, it->first);
				pos_i = find_non_escaped(cpy, "%s", pos_i);
			}
			pos_i = find_non_escaped(cpy, "%x", 0);
			while (pos_i != std::string::npos)
			{
				cpy.replace( pos_i, 2, static_cast<std::ostringstream*>( &(std::ostringstream() << it->second.get_dimensions().x) )->str() );
				pos_i = find_non_escaped(cpy, "%x", pos_i);
			}
			pos_i = find_non_escaped(cpy, "%w", 0);
			while (pos_i != std::string::npos)
			{
				cpy.replace( pos_i, 2, static_cast<std::ostringstream*>( &(std::ostringstream() << it->second.get_dimensions().width) )->str() );
				pos_i = find_non_escaped(cpy, "%w", pos_i);
			}
			m_old_render_str.append("%{S:" + it->first + ":F-B-U-}");
			m_old_render_str.append(cpy);
    	}
    }
    parse_render(m_old_render_str);
}

void limebar::Bar::parse_pre_render(std::string input)
{
	size_t pos = 0;

	while (pos < input.size())
	{
		std::map< char, std::function<void(std::string &, size_t &)> >::iterator it = m_parse_non_render.find(input[pos]);
		if (it == m_parse_non_render.end())
			return;

		it->second(input, pos);
	}
}

void limebar::Bar::parse_render(std::string input)
{
	size_t pos_i = 0, pos_j = find_non_escaped(input, "%{", 0);

	while (pos_j != std::string::npos)
	{
		if (pos_i < pos_j) m_X_handler.draw_string(input.substr(pos_i, pos_j - pos_i));

		pos_i = pos_j + 2;
		pos_j = find_non_escaped(input, "}", pos_i);
		if (pos_j == std::string::npos) return;

		while (pos_i < pos_j)
		{
			std::map< char, std::function<void(std::string &, size_t &, size_t &)> >::iterator it = m_parse_render.find(input[pos_i]);
			if (it == m_parse_render.end())
				break;

			it->second(input, pos_i, pos_j);
		}

		pos_i = pos_j + 1;
		pos_j = find_non_escaped(input, "%{", pos_i);
	}

	if (pos_i < input.size())
		m_X_handler.draw_string(input.substr(pos_i));
}
