#include "Module.hpp"

#include "Bar.hpp"
#include "DatabaseHelper.hpp"

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "i3ipc++/ipc.hpp"

#define FOCUSED_FOREGROUND "#a0a0a0"
#define FOCUSED_BACKGROUND "#152873"
#define FOCUSED_UNDERLINE  "#1E39A1"

#define FOCUSED_INACTIVE_FOREGROUND "#062497"
#define FOCUSED_INACTIVE_BACKGROUND "#5f676a"
#define FOCUSED_INACTIVE_UNDERLINE  "#333333"

#define UNFOCUSED_FOREGROUND "#A0A0A0"
#define UNFOCUSED_BACKGROUND "#2b2b2b"
#define UNFOCUSED_UNDERLINE  "#3a3a3a"

#define URGENT_FOREGROUND "#ffffff"
#define URGENT_BACKGROUND "#900000"
#define URGENT_UNDERLINE  "#2f343a"

#define SPACING   2
#define SHOW_ALL  false
#define WS_FORMAT "%{+u+o|} %ws %{-u-o|}"

namespace limebar
{
	struct color_category_t
	{
		std::string foreground;
		std::string background;
		std::string underline;
	};

	class Module_i3 : public Module {
	public:
		Module_i3()
		{
			m_focused.foreground = FOCUSED_FOREGROUND;
    		m_focused.background = FOCUSED_BACKGROUND;
    		m_focused.underline  = FOCUSED_UNDERLINE;
		
    		m_focused_inactive.foreground = FOCUSED_INACTIVE_FOREGROUND;
    		m_focused_inactive.background = FOCUSED_INACTIVE_BACKGROUND;
    		m_focused_inactive.underline  = FOCUSED_INACTIVE_UNDERLINE ;
		
    		m_unfocused.foreground = UNFOCUSED_FOREGROUND;
    		m_unfocused.background = UNFOCUSED_BACKGROUND;
    		m_unfocused.underline  = UNFOCUSED_UNDERLINE ;
		
    		m_urgent.foreground = URGENT_FOREGROUND;
    		m_urgent.background = URGENT_BACKGROUND;
    		m_urgent.underline  = URGENT_UNDERLINE ;
			
			m_spacing   = SPACING;
			m_show_all  = SHOW_ALL;
			m_ws_format = WS_FORMAT;

			// Initialize i3ipcpp
			m_conn.subscribe(i3ipc::ET_WORKSPACE | i3ipc::ET_OUTPUT | i3ipc::ET_MODE);
    		m_conn.signal_workspace_event.connect([&](const i3ipc::workspace_event_t&  ev) {
        		workspaces_change();
    		});
    		m_conn.signal_output_event.connect([&]() {
    		    outputs_change();
    		});
			m_conn.signal_mode_event.connect( [&](const i3ipc::mode_t&  m) { update_mode(m); } );
			LIMEBAR.get_labels()["i3binding"] = "default";

			m_conn.connect_event_socket();

			LIMEBAR.add_fd({ .fd = m_conn.get_event_socket_fd(), .events = POLLIN, .revents = 0 }, [&](struct pollfd &fd){
				if (fd.revents & POLLIN) { // The event comes from i3
        	    	m_conn.handle_event();
        		}
				return true;
			});

			LIMEBAR.add_config([&](XrmDatabase &db){config(db);});
		
    		// Get workspace and outputs
    		outputs_change();
		}

	private:
		void config(XrmDatabase &db)
		{
			limebar::DatabaseHelper::get_color    ( db, "limebar.focused.foreground"         , "limebar.focused.foreground"         , m_focused.foreground          );
    		limebar::DatabaseHelper::get_color    ( db, "limebar.focused.background"         , "limebar.focused.background"         , m_focused.background          );
    		limebar::DatabaseHelper::get_color    ( db, "limebar.focused.underline"          , "limebar.focused.underline"          , m_focused.underline           );
		
    		limebar::DatabaseHelper::get_color    ( db, "limebar.focused_inactive.foreground", "limebar.focused_inactive.foreground", m_focused_inactive.foreground );
    		limebar::DatabaseHelper::get_color    ( db, "limebar.focused_inactive.background", "limebar.focused_inactive.background", m_focused_inactive.background );
    		limebar::DatabaseHelper::get_color    ( db, "limebar.focused_inactive.underline" , "limebar.focused_inactive.underline" , m_focused_inactive.underline  );
		
    		limebar::DatabaseHelper::get_color    ( db, "limebar.unfocused.foreground"       , "limebar.unfocused.foreground"       , m_unfocused.foreground        );
    		limebar::DatabaseHelper::get_color    ( db, "limebar.unfocused.background"       , "limebar.unfocused.background"       , m_unfocused.background        );
    		limebar::DatabaseHelper::get_color    ( db, "limebar.unfocused.underline"        , "limebar.unfocused.underline"        , m_unfocused.underline         );
		
    		limebar::DatabaseHelper::get_color    ( db, "limebar.urgent.foreground"          , "limebar.urgent.foreground"          , m_urgent.foreground           );
    		limebar::DatabaseHelper::get_color    ( db, "limebar.urgent.background"          , "limebar.urgent.background"          , m_urgent.background           );
    		limebar::DatabaseHelper::get_color    ( db, "limebar.urgent.underline"           , "limebar.urgent.underline"           , m_urgent.underline            );
		
    		limebar::DatabaseHelper::get_unsigned ( db, "limebar.ws-spacing"                 , "limebar.ws-spacing"                 , m_spacing                     );
    		limebar::DatabaseHelper::get_boolean  ( db, "limebar.show-all"                   , "limebar.show-all"                   , m_show_all                    );
    		limebar::DatabaseHelper::get_string   ( db, "limebar.ws-format"                  , "limebar.ws-format"                  , m_ws_format                   );
		}
		void outputs_change()
		{
			got_outputs(m_conn.get_outputs());
    		workspaces_change();
		}

		void got_outputs(const std::vector< std::shared_ptr<i3ipc::output_t> > &new_outputs)
		{
			// Handle new outputs
    		for ( auto output : new_outputs )
    		{
    		    // Keep only active output
    		    if ( !output->active )
    		        continue;
		
    		    // Check if the output already exists
    		    auto old_it = m_outputs.find(output->name);
    		    if ( old_it != m_outputs.end() )
    		    {    
    		        // Check if mode changed
    		        i3ipc::rect_t old_rect = old_it->second;
    		        i3ipc::rect_t new_rect = output->rect;
    		        if ( old_rect.x == new_rect.x && old_rect.y == new_rect.y && old_rect.width == new_rect.width && old_rect.height == new_rect.height )
    		            continue;
    		        m_outputs.erase ( old_it );
    		    }
		
    		    // Create the output
    		    m_outputs[ output->name ] = output->rect;
    		}
		
    		// Handle old outputs
    		for ( auto output : m_outputs )
    		{
    		    for ( auto new_output : new_outputs )
    		    {
    		        if ( output.first.compare(new_output->name) == 0 )
    		        {
    		            goto next;
    		        }
    		    }
    		    m_outputs.erase ( m_outputs.find(output.first) );
		
    		    next:;
    		}
		
    		// Reset the bar
    		if (LIMEBAR.is_init())
    		{
    			LIMEBAR.cleanup();
    		    LIMEBAR.init();
    		}
		}

		void workspaces_change()
		{
			m_i3_workspaces = m_conn.get_workspaces();
    		update_outputs();
		}

		void update_outputs()
		{
			for ( auto output : m_outputs )
			{
				std::string message;
		
				for ( auto workspace : m_i3_workspaces )
				{
					if ( !m_show_all and 0 != workspace->output.compare(output.first) )
						continue;
					const color_category_t &color_to_use = (workspace->urgent) ? m_urgent : (workspace->focused) ? m_focused :
						(workspace->visible) ? m_focused_inactive : m_unfocused;
					message += "%{F:" + color_to_use.foreground + ":B:" + color_to_use.background + ":U:" + color_to_use.underline + ":";
					message += "A:i3-msg workspace number \"" + escape(workspace->name, ":") + "\":}";
		
					std::string ws_str = m_ws_format;
					size_t pos_i=0;
					pos_i = ws_str.find("%ws");
					while (pos_i != std::string::npos)
					{
						ws_str.replace(pos_i, 3, workspace->name);
						pos_i = ws_str.find("%ws", pos_i);
					}
					pos_i = ws_str.find("%wo");
					while (pos_i != std::string::npos)
					{
						ws_str.replace( pos_i, 3, static_cast<std::ostringstream*>( &(std::ostringstream() << workspace->num) )->str() );
						pos_i = ws_str.find("%wo", pos_i);
					}
					message += ws_str;
		
					message += "%{AF-B-U-O" + std::to_string(m_spacing) + "}";
				}
		
				LIMEBAR.get_labels()["ws" + output.first] = message;
			}
			LIMEBAR.parse_input("");
		}

		void update_mode(const i3ipc::mode_t&  m)
		{
			LIMEBAR.get_labels()["i3binding"] = m.change;
			LIMEBAR.parse_input("");
		}
    	
    private:
    	i3ipc::connection                                  m_conn;
		std::vector< std::shared_ptr<i3ipc::workspace_t> > m_i3_workspaces;
		std::map< std::string, i3ipc::rect_t >             m_outputs;

		color_category_t   m_focused;
		color_category_t   m_focused_inactive;
		color_category_t   m_unfocused;
		color_category_t   m_urgent;
		unsigned int       m_spacing;
		bool               m_show_all;
		std::string        m_ws_format;
	};

}

// the class factories
extern "C" limebar::Module* create() {
    return new limebar::Module_i3;
}

extern "C" void destroy(limebar::Module* p) {
    delete p;
}