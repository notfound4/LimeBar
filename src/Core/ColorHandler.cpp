#include "ColorHandler.hpp"

#include "DatabaseHelper.hpp"

#include <iostream>

#include <boost/regex.hpp>

#define COLOR_FOREGROUND 0xffffffff
#define COLOR_BACKGROUND 0xff000000
#define COLOR_UNDERLINE  0xff9f9f9f

limebar::ColorHandler::ColorHandler()
{
    m_default_foreground.v = COLOR_FOREGROUND;
    m_default_background.v = COLOR_BACKGROUND;
    m_default_underline.v  = COLOR_UNDERLINE ;
    m_current_foreground = m_default_foreground;
    m_current_background = m_default_background;
    m_current_underline  = m_default_underline;
}

void limebar::ColorHandler::set_color(rgba_t &destination, const std::string &source)
{
	destination.v = (uint32_t) std::stoul(source.substr(1), nullptr, 16);

    switch (source.size()) {
        case 4:
            // Expand the #rgb format into #rrggbb (aa is set to 0xff)
            destination.v = (destination.v & 0xf00) * 0x1100
                          | (destination.v & 0x0f0) * 0x0110
                          | (destination.v & 0x00f) * 0x0011;
        case 7:
            // If the code is in #rrggbb form then assume it's opaque
            destination.a = 255;
            break;
        default:
            ;
    }
    destination.r = (destination.r * destination.a) / 255;
    destination.g = (destination.g * destination.a) / 255;
    destination.b = (destination.b * destination.a) / 255;
}

rgba_t &limebar::ColorHandler::get_foreground()
{
	return m_current_foreground;
}

rgba_t &limebar::ColorHandler::get_background()
{
	return m_current_background;
}

rgba_t &limebar::ColorHandler::get_underline ()
{
	return m_current_underline;
}

void limebar::ColorHandler::set_foreground(const std::string &source, bool is_default)
{
    if ( regex_match(source, boost::regex("#(?:[0-9a-fA-F]{8}|[0-9a-fA-F]{6}|[0-9a-fA-F]{3})")) )
    {
        if (is_default)
            set_color(m_default_foreground, source);
        else
        {
            set_color(m_current_foreground, source);
        }
    }
}

void limebar::ColorHandler::set_background(const std::string &source, bool is_default)
{
    if ( regex_match(source, boost::regex("#(?:[0-9a-fA-F]{8}|[0-9a-fA-F]{6}|[0-9a-fA-F]{3})")) )
    {
        if (is_default)
            set_color(m_default_background, source);
        else
        {
            set_color(m_current_background, source);
        }
    }
}

void limebar::ColorHandler::set_underline (const std::string &source, bool is_default)
{
    if ( regex_match(source, boost::regex("#(?:[0-9a-fA-F]{8}|[0-9a-fA-F]{6}|[0-9a-fA-F]{3})")) )
    {
        if (is_default)
            set_color(m_default_underline, source);
        else
        {
            set_color(m_current_underline, source);
        }
    }
}

void limebar::ColorHandler::switch_colors()
{
    rgba_t tmp = m_current_foreground;
    m_current_foreground = m_current_background;
    m_current_background = tmp;
}

void limebar::ColorHandler::reset_to_default()
{
    m_current_foreground = m_default_foreground;
    m_current_background = m_default_background;
    m_current_underline  = m_default_underline;
}

void limebar::ColorHandler::config(XrmDatabase &db)
{
    std::string str_tmp;
    limebar::DatabaseHelper::get_color ( db, "limebar.foreground", "limebar.foreground", str_tmp );
    set_foreground(str_tmp, true);

    limebar::DatabaseHelper::get_color ( db, "limebar.background", "limebar.background", str_tmp );
    set_background(str_tmp, true);

    limebar::DatabaseHelper::get_color ( db, "limebar.underline",  "limebar.underline",  str_tmp );
    set_underline (str_tmp, true);
}

void limebar::ColorHandler::handle_foreground(std::string &input, size_t &pos_i, size_t &pos_j)
{
    handle_color(m_current_foreground, m_default_foreground, input, pos_i, pos_j);
}

void limebar::ColorHandler::handle_background(std::string &input, size_t &pos_i, size_t &pos_j)
{
    handle_color(m_current_background, m_default_background, input, pos_i, pos_j);
}

void limebar::ColorHandler::handle_underline (std::string &input, size_t &pos_i, size_t &pos_j)
{
    handle_color(m_current_underline,  m_default_underline,  input, pos_i, pos_j);
}

void limebar::ColorHandler::handle_color(rgba_t &destination, rgba_t &def, std::string &input, size_t &pos_i, size_t &pos_j)
{
    if (++pos_i == pos_j) return;
    switch ( input[pos_i] )
    {
        case '-':
            destination = def;
            return;
        case ':':
        {
            if (++pos_i == pos_j) return;
    
            size_t pos_k = find_non_escaped(input, ":", pos_i, pos_j);
            if (pos_k >= pos_j)
            {
                pos_i = pos_j;
                return;
            }
            if (input[pos_i] != '#' or ++pos_i >= pos_k)
            {
                std::cerr << "Invalid color format!" << std::endl;
                pos_i = pos_k+1;
                return;
            }
            try
            {
                rgba_t tmp;
                size_t j;
                tmp.v = std::stoul(input.substr(pos_i, pos_k - pos_i), &j, 16);

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
                        pos_i = pos_k + 1;
                        return;
                }
                // Premultiply the alpha in
                if (tmp.a != 0) {
                    // The components are clamped automagically as the rgba_t is made of uint8_t
                    tmp.r = (tmp.r * tmp.a) / 255;
                    tmp.g = (tmp.g * tmp.a) / 255;
                    tmp.b = (tmp.b * tmp.a) / 255;
                }
                pos_i = pos_k + 1;
                destination = tmp;
                return;
            }
            catch (...)
            {
                pos_i = pos_k+1;
                std::cerr << "Invalid color format!" << std::endl;
                return;
            }
            break;
        }
        default:
            std::cerr << "Invalid color format!" << std::endl;
            return;
    }
}