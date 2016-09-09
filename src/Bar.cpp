#include "Bar.hpp"
#include "Resource.hpp"

#include <algorithm>
#include <iostream>
#include <cctype>

extern "C" {
#include <unistd.h>
}

namespace limebar {

Bar::Bar() : m_pollin{ { .fd = STDIN_FILENO, .events = POLLIN }, { .fd = -1, .events = POLLIN }, }
{
    m_pollin[1].fd = xcb_get_file_descriptor(Resource::instance().m_connection);
}

void Bar::loop()
{
    xcb_generic_event_t *ev;
    xcb_expose_event_t *expose_ev;
    xcb_button_press_event_t *press_ev;

    for (;;) {
        bool redraw = false;

        if (xcb_connection_has_error(Resource::instance().m_connection))
            break;

        if (poll(m_pollin, 2, -1) > 0) {
            if (m_pollin[0].revents & POLLHUP) {      // No more data...
                if (Resource::instance().m_permanent) m_pollin[0].fd = -1;   // ...null the fd and continue polling :D
                else break;                         // ...bail out
            }
            if (m_pollin[0].revents & POLLIN) { // New input, process it
				static char format_text[4096] = {0, };
				if ( !std::cin.getline(format_text, sizeof(format_text)).good() )
					break;
				parse_input(format_text);
                redraw = true;
            }
            if (m_pollin[1].revents & POLLIN) { // The event comes from the Xorg server
                while ( (ev = xcb_poll_for_event(Resource::instance().m_connection)) ) {
                    expose_ev = (xcb_expose_event_t *)ev;

                    switch (ev->response_type & 0x7F) {
                        case XCB_EXPOSE:
                            if (expose_ev->count == 0)
                                redraw = true;
                            break;
                        case XCB_BUTTON_PRESS:
                            press_ev = (xcb_button_press_event_t *)ev;
                            {
                            	std::map< std::string, Monitor >::iterator it =
                            		std::find_if (Resource::instance().m_monitors.begin(), Resource::instance().m_monitors.end(),
                            			[&](const std::pair<const std::string, Monitor> &pair){return press_ev->event == pair.second.get_window();});
                            	it->second.on_click(press_ev->event_x, press_ev->detail);
                            }
                            break;
                    }

                    free(ev);
                }
            }
        }

        if (redraw)	Resource::instance().redraw_all();

        xcb_flush(Resource::instance().m_connection);
    }
}

void Bar::parse_input(const std::string &input)
{
	std::string non_label("");
	size_t pos_i = 0, pos_j = input.find("%{Ls:");

	while (pos_j != std::string::npos)
	{
		std::string label_name("");

		non_label.append(input, pos_i, pos_j - pos_i);
		pos_i = pos_j + 5;

		pos_j = input.find_first_of('}', pos_i);
		if (pos_j == std::string::npos) return;
		label_name = input.substr(pos_i, pos_j - pos_i);
		pos_i = pos_j + 1;

		pos_j = input.find("%{Ls}", pos_i);
		if (pos_j == std::string::npos) return;
		m_labels_map[label_name] = input.substr(pos_i, pos_j - pos_i);
		pos_i = pos_j + 5;

		pos_j = input.find("%{Ls:", pos_i);
	}

	if (pos_i < input.size())
		non_label.append(input, pos_i, std::string::npos);

	if (non_label.empty()) return;


	Resource::instance().reset_to_default();
	Resource::instance().clear_all();
	parse_non_label(non_label);
}

rgba_t parse_color(const std::string &input, size_t &pos_i, const rgba_t &def)
{
	switch ( input[pos_i++] )
	{
		case '-': return def;
		case '#':
			try
			{
				rgba_t tmp;
				size_t j;
				tmp.v = std::stoul(input.substr(pos_i,std::string::npos), &j, 16);

				switch (j)
				{
					case 3:
						// Expand the #rgb format into #rrggbb (aa is set to 0xff)
            			tmp.v = (tmp.v & 0xf00) * 0x1100
            			      | (tmp.v & 0x0f0) * 0x0110
            			      | (tmp.v & 0x00f) * 0x0011;
            		case 6:
            			// If the code is in #rrggbb form then assume it's opaque
            			tmp.a = 255;
            			break;

            		case 8:
            			break;
            		default:
            			std::cerr << "Invalid color format!" << std::endl;
						return def;
				}
				// Premultiply the alpha in
    			if (tmp.a != 0) {
    			    // The components are clamped automagically as the rgba_t is made of uint8_t
    			    tmp.r = (tmp.r * tmp.a) / 255;
    			    tmp.g = (tmp.g * tmp.a) / 255;
    			    tmp.b = (tmp.b * tmp.a) / 255;
    			}
    			pos_i += j;
    			return tmp;
			}
			catch (...)
			{
				std::cerr << "Invalid color format!" << std::endl;
				return def;
			}
			break;
		default:
			std::cerr << "Invalid color format!" << std::endl;
			return def;
	}
}

void Bar::parse_non_label(const std::string &input)
{
	size_t pos_i = 0;
	parse_non_label(input, pos_i);
}

void Bar::parse_non_label(const std::string &input, size_t &pos_i)
{
	size_t pos_j = input.find("%{", pos_i);
	while (pos_j != std::string::npos)
	{
		if (pos_i < pos_j) handle_string(input, pos_i, pos_j);

		pos_i = pos_j + 2;
		pos_j = input.find_first_of('}', pos_i);
		if (pos_j == std::string::npos)
		{
			pos_i = std::string::npos;
			return;
		}

		while (pos_i < pos_j)
		{
			while ( isspace(input[pos_i]) ) pos_i++;
			switch ( input[pos_i++] )
			{
                case '+': handle_attr(input, pos_i); break;
                case '-': handle_attr(input, pos_i); break;
                case '!': handle_attr(input, pos_i); break;

                case '|':
                	Resource::instance().m_current_monitor->second.draw_vertical_line();
                	Resource::instance().m_pos_x += Resource::instance().m_underline;
                	break;

				case 'm':
					handle_string(Resource::instance().m_current_monitor->first, 0, std::string::npos);
					break;

				case 'R':
					Resource::instance().switch_colors();
					break;

                case 'l': Resource::instance().m_pos_x = 0; Resource::instance().m_align = ALIGN_L; break;
                case 'c': Resource::instance().m_pos_x = 0; Resource::instance().m_align = ALIGN_C; break;
                case 'r': Resource::instance().m_pos_x = 0; Resource::instance().m_align = ALIGN_R; break;

                case 'A':
                	{
                		if (pos_i == pos_j) return;
                	
                	    uint8_t     button = XCB_BUTTON_INDEX_1;
                		uint16_t  	begin, end;
                		Alignment 	align;
                		std::string command;
                		bool		print_pos;
             
                	   	if ( isdigit(input[pos_i]) )
                	   	    button = input[pos_i++] - '0';

    					if (input[pos_i] == 'W') {
    					    print_pos = true;
    					    pos_i++;
    					}
    					else
    						print_pos = false;

    					if (input[pos_i++] != ':')
    						goto next_block;

    					command = input.substr(pos_i, pos_j - pos_i);
    					pos_i = pos_j + 1;

    					begin = Resource::instance().m_pos_x;
    					align = Resource::instance().m_align;

    					parse_non_label(input, pos_i);
    					pos_j = pos_i;

        				switch (align) {
        				    case ALIGN_L:
        				        end = Resource::instance().m_pos_x;
        				        break;
        				    case ALIGN_C:
        				    	end = ( Resource::instance().m_current_monitor->second.get_dimensions().width + Resource::instance().m_pos_x ) / 2;
        				    	begin += end - Resource::instance().m_pos_x;
        				        break;
        				    case ALIGN_R:
        				    	end = Resource::instance().m_current_monitor->second.get_dimensions().width;
        				    	begin += end - Resource::instance().m_pos_x;
        				        break;
        				}

    					Resource::instance().m_current_monitor->second.add_area(button, begin, end, align, command, print_pos);
             
                	    goto next_block;
                	}

                case 'T':
                	if (input[pos_i] == '-')
                	{
                		Resource::instance().m_font_desc = Resource::instance().m_default_font_desc;
                		pos_i++;
                	}
                	else
                	{
                		Resource::instance().set_font( input.substr(pos_i, pos_j - pos_i) );
                		goto next_block;
                	}
                	break;

                case 'S':
                {
                	std::string out_str = input.substr(pos_i, pos_j - pos_i);
                	std::map<std::string, Monitor>::iterator tmp_it = Resource::instance().m_monitors.find(out_str);
                	if ( tmp_it != Resource::instance().m_monitors.end() )
                	{
                		Resource::instance().m_current_monitor = tmp_it;
                		Resource::instance().m_pos_x = 0;
                	}
                	goto next_block;
                }

                case 'I':
                {
                	std::string image_str = input.substr(pos_i, pos_j - pos_i);
                	Resource::instance().m_current_monitor->second.draw_image(image_str);
                }

                case 'L':
                {
                	if (input[pos_i++] != 'u' or input[pos_i++] != ':') continue;
                	std::string label_str = input.substr(pos_i, pos_j - pos_i);
                	std::map< std::string, std::string >::iterator label_it = m_labels_map.find(label_str);
                	if ( label_it != m_labels_map.end() )
                		parse_non_label(label_it->second);
                	goto next_block;
                }

                case 'F': Resource::instance().m_current_foreground = parse_color(input, pos_i, Resource::instance().m_default_foreground);
                	Resource::instance().update_gc(); break;
                case 'B': Resource::instance().m_current_background = parse_color(input, pos_i, Resource::instance().m_default_background);
                	Resource::instance().update_gc(); break;
                case 'U': Resource::instance().m_current_underline  = parse_color(input, pos_i, Resource::instance().m_default_underline );
                	Resource::instance().update_gc(); break;

                case 'O':
                	try
                	{
                		size_t pos_k;
                    	uint16_t w = std::stoul( input.substr(pos_i, std::string::npos), &pos_k, 10 );

                    	Resource::instance().m_current_monitor->second.draw_shift(w);
                        Resource::instance().m_current_monitor->second.area_shift(w);
                    	Resource::instance().m_pos_x += w;

                    	pos_i += pos_k;
                    }
                    catch (...)
                    {
                    	continue;
                    }
                    break;

				default:
					goto next_block;
			}
		}

next_block:
		pos_i = pos_j + 1;
		pos_j = input.find("%{", pos_i);
	}

	if (pos_i < input.size())
		handle_string(input, pos_i, pos_j);

	pos_i = pos_j;
}

void Bar::handle_string(const std::string &input, size_t pos_i, size_t pos_j)
{
	Resource::instance().m_current_monitor->second.draw_string(input.substr(pos_i, pos_j - pos_i));
}

void Bar::handle_attr(const std::string &input, size_t &pos_i)
{
	uint8_t attr;
	switch ( input[pos_i++] )
	{
		case 'u': attr = ATTR_UNDERLINE; break;
		case 'o': attr = ATTR_OVERLINE; break;
		default: return;
	}
	switch ( input[pos_i - 2] )
	{
		case '+': Resource::instance().m_attr |=  attr; break;
		case '-': Resource::instance().m_attr &= ~attr; break;
		case '!': Resource::instance().m_attr ^=  attr; break;
	}
}

}