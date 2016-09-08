#include "Resource.hpp"

extern "C" {
#include <unistd.h>
#include <xcb/randr.h>
#include <xcb/xcb_ewmh.h>
}

#include <boost/regex.hpp>

#include <iostream>

#define DEFAULT_Y_OFFSET 4

#define COLOR_FOREGROUND 0xffffffff
#define COLOR_BACKGROUND 0xff000000
#define COLOR_UNDERLINE  0xff9f9f9f

#define SPACING   0
#define COMMAND   ""
#define SHOW_ALL  false
#define BOTTOM    false
#define DOCK      false
#define FONT      "Sans 12"
#define PERMANENT false
#define WM_NAME   "limebar"
#define UNDERLINE 1
#define WS_FORMAT "%wo: %ws"
#define WIDTH     -1
#define HEIGHT    -1
#define X         0
#define Y         0



double get_R(const rgba_t &color)
{
    return static_cast<double>(color.r) / 255.;
}
double get_G(const rgba_t &color)
{
    return static_cast<double>(color.g) / 255.;
}
double get_B(const rgba_t &color)
{
    return static_cast<double>(color.b) / 255.;
}
double get_A(const rgba_t &color)
{
    return static_cast<double>(color.a) / 255.;
}

Resource::Resource()
{
    m_default_foreground.v = COLOR_FOREGROUND;
    m_default_background.v = COLOR_BACKGROUND;
    m_default_underline.v  = COLOR_UNDERLINE ;
    m_width     = WIDTH;
    m_height    = HEIGHT;
    m_x         = X;
    m_y         = Y;
    m_command   = COMMAND;
    m_bottom    = BOTTOM;
    m_dock      = DOCK;
    m_permanent = PERMANENT;
    m_wm_name   = WM_NAME;
    m_underline = UNDERLINE;

    m_default_font_desc = nullptr;
}

void Resource::update_gc ()
{
    uint32_t uint_array[] = { m_current_foreground.v };
    xcb_change_gc(m_connection, m_gc[GC_DRAW], XCB_GC_FOREGROUND, uint_array);
    uint_array[0] = m_current_background.v;
    xcb_change_gc(m_connection, m_gc[GC_CLEAR], XCB_GC_FOREGROUND, uint_array);
    uint_array[0] = m_current_underline.v;
    xcb_change_gc(m_connection, m_gc[GC_ATTR], XCB_GC_FOREGROUND, uint_array);
}

void Resource::set_color(rgba_t &destination, const std::string &source)
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

void Resource::set_foreground(const std::string &source, bool is_default)
{
    if ( regex_match(source, boost::regex("#(?:[0-9a-fA-F]{8}|[0-9a-fA-F]{6}|[0-9a-fA-F]{3})")) )
    {
        if (is_default)
            set_color(m_default_foreground, source);
        else
        {
            set_color(m_current_foreground, source);
            update_gc();
        }
    }
}

void Resource::set_background(const std::string &source, bool is_default)
{
    if ( regex_match(source, boost::regex("#(?:[0-9a-fA-F]{8}|[0-9a-fA-F]{6}|[0-9a-fA-F]{3})")) )
    {
        if (is_default)
            set_color(m_default_background, source);
        else
        {
            set_color(m_current_background, source);
            update_gc();
        }
    }
}

void Resource::set_underline(const std::string &source, bool is_default)
{
    if ( regex_match(source, boost::regex("#(?:[0-9a-fA-F]{8}|[0-9a-fA-F]{6}|[0-9a-fA-F]{3})")) )
    {
        if (is_default)
            set_color(m_default_underline, source);
        else
        {
            set_color(m_current_underline, source);
            update_gc();
        }
    }
}

void Resource::switch_colors()
{
    rgba_t tmp = m_current_foreground;
    m_current_foreground = m_current_background;
    m_current_background = tmp;
    update_gc();
}

void Resource::set_font(const std::string &source, bool is_default)
{
    if (source.empty()) return;

    if (is_default)
    {
        if ( m_fonts.find(source) != m_fonts.end() )
            m_default_font_desc = m_fonts[source];
        else
        {
            m_default_font_desc = pango_font_description_from_string(source.c_str());
            m_fonts[source] = m_default_font_desc;
        }
    }
    else
    {
        if ( m_fonts.find(source) != m_fonts.end() )
            m_font_desc = m_fonts[source];
        else
        {
            m_font_desc = pango_font_description_from_string(source.c_str());
            m_fonts[source] = m_font_desc;
        }
    }
}

void Resource::set_geometry(const std::string &source)
{
    boost::regex re = boost::regex("(\\d*)x(\\d*)\\+(\\d*)\\+(\\d*)");
    if ( regex_match(source, re) )
    {
        boost::sregex_token_iterator i(source.begin(), source.end(), re, {1, 2, 3, 4});
        std::string tmp = *i;
        if (!tmp.empty())
            m_width = std::stoi(tmp);
        tmp = *(++i);
        if (!tmp.empty())
            m_height = std::stoi(tmp);
        tmp = *(++i);
        if (!tmp.empty())
            m_x = std::stoi(tmp);
        tmp = *(++i);
        if (!tmp.empty())
            m_y = std::stoi(tmp);
    }
}

void Resource::reset_to_default()
{
    m_current_monitor = m_default_monitor;
    m_current_foreground.v = m_default_foreground.v;
    m_current_background.v = m_default_background.v;
    m_current_underline.v  = m_default_underline.v;
    m_attr  = 0;
    m_pos_x = 0;
    m_align = ALIGN_L;
    m_font_desc = m_default_font_desc;
    update_gc();
}
    
unsigned int Resource::get_font_height()
{
    return pango_font_description_get_size(m_font_desc) / PANGO_SCALE;
}
    
unsigned int Resource::get_max_font_height()
{
    unsigned int ret = 0;
    for (auto desc : m_fonts)
    {
        unsigned int h = pango_font_description_get_size(desc.second) / PANGO_SCALE;
        if (h > ret)
            ret = h;
    }
    return ret;
}

void Resource::init()
{
    if (m_default_font_desc == nullptr)
        set_font(FONT, true);

    m_connection = xcb_connect (NULL, NULL);
    if (xcb_connection_has_error(m_connection)) {
        std::cerr << "Couldn't connect to X" << std::endl;
        exit(EXIT_FAILURE);
    }

    m_screen = xcb_setup_roots_iterator( xcb_get_setup(m_connection) ).data;

    m_depth = 32;
    m_visualtype = get_visualtype();
    if (m_visualtype == NULL) {
        m_depth = 24;
        m_visualtype = get_visualtype();
    }

    m_colormap = xcb_generate_id(m_connection);
    xcb_create_colormap(m_connection, XCB_COLORMAP_ALLOC_NONE, m_colormap, m_screen->root, m_visualtype->visual_id);

    const xcb_query_extension_reply_t  *reply = xcb_get_extension_data(m_connection, &xcb_randr_id);
    if (reply->present) {
        init_randr();
    }

    if (m_monitors.empty()) {
        // If I fits I sits
        if (m_width < 0)
            m_width = m_screen->width_in_pixels - m_x;

        // Adjust the height
        if (m_height < 0 || m_height > m_screen->height_in_pixels)
            m_height = get_max_font_height() + m_underline + DEFAULT_Y_OFFSET;

        // Check the geometry
        if (m_x + m_width > m_screen->width_in_pixels ||
            m_y + m_height > m_screen->height_in_pixels) {
            std::cerr << "The geometry specified doesn't fit the screen!" << std::endl;
            exit(EXIT_FAILURE);
        }

        // If no RandR outputs, fall back to using whole screen
        xcb_rectangle_t rect_tmp = {static_cast<int16_t>(m_x), 0, static_cast<uint16_t>(m_width), m_screen->height_in_pixels};
        m_monitors.emplace(std::piecewise_construct, std::make_tuple("screen"), std::forward_as_tuple(rect_tmp));
    }

    init_ewmh();

    uint32_t uint_array[] = { m_default_foreground.v };
    m_gc[GC_DRAW] = xcb_generate_id(m_connection);
    xcb_create_gc(m_connection, m_gc[GC_DRAW], m_monitors.begin()->second.get_pixmap(), XCB_GC_FOREGROUND, uint_array);

    uint_array[0] = m_default_background.v;
    m_gc[GC_CLEAR] = xcb_generate_id(m_connection);
    xcb_create_gc(m_connection, m_gc[GC_CLEAR], m_monitors.begin()->second.get_pixmap(), XCB_GC_FOREGROUND, uint_array);

    uint_array[0] = m_default_underline.v;
    m_gc[GC_ATTR] = xcb_generate_id(m_connection);
    xcb_create_gc(m_connection, m_gc[GC_ATTR], m_monitors.begin()->second.get_pixmap(), XCB_GC_FOREGROUND, uint_array);

    for (auto it = m_monitors.begin(); it != m_monitors.end(); ++it)
    {
        Monitor &mon = it->second;
        mon.clear();
        xcb_map_window(m_connection, mon.get_window());

        // Make sure that the window really gets in the place it's supposed to be
        // Some WM such as Openbox need this
        uint32_t coord_array[] = { static_cast<uint32_t>(mon.get_dimensions().x), static_cast<uint32_t>(mon.get_dimensions().y) };
        xcb_configure_window( m_connection, mon.get_window(), XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, coord_array );
    }
    xcb_flush(m_connection);
}

void Resource::init_ewmh()
{
    const char *atom_names[] = {
        "_NET_WM_WINDOW_TYPE",
        "_NET_WM_WINDOW_TYPE_DOCK",
        "_NET_WM_DESKTOP",
        "_NET_WM_STRUT_PARTIAL",
        "_NET_WM_STRUT",
        "_NET_WM_STATE",
        // Leave those at the end since are batch-set
        "_NET_WM_STATE_STICKY",
        "_NET_WM_STATE_ABOVE",
    };
    const int atoms = sizeof(atom_names)/sizeof(char *);
    xcb_intern_atom_cookie_t atom_cookie[atoms];
    xcb_atom_t atom_list[atoms];
    xcb_intern_atom_reply_t *atom_reply;

    // As suggested fetch all the cookies first (yum!) and then retrieve the
    // atoms to exploit the async'ness
    for (int i = 0; i < atoms; i++)
        atom_cookie[i] = xcb_intern_atom(m_connection, 0, strlen(atom_names[i]), atom_names[i]);

    for (int i = 0; i < atoms; i++) {
        atom_reply = xcb_intern_atom_reply(m_connection, atom_cookie[i], NULL);
        if (!atom_reply)
            return;
        atom_list[i] = atom_reply->atom;
        free(atom_reply);
    }

    for (auto it = m_monitors.begin(); it != m_monitors.end(); ++it)
    {
        int strut[12] = {0};
        if (m_bottom)
        {
            strut[3] = m_height;
            strut[10] = (*it).second.get_dimensions().x;
            strut[11] = strut[10] + (*it).second.get_dimensions().width;
        }
        else
        {
            strut[2] = m_height;
            strut[8] = (*it).second.get_dimensions().x;
            strut[9] = strut[10] + (*it).second.get_dimensions().width;
        }

        xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE, (*it).second.get_window(),
            atom_list[NET_WM_WINDOW_TYPE], XCB_ATOM_ATOM, 32, 1, &atom_list[NET_WM_WINDOW_TYPE_DOCK]);

        xcb_change_property(m_connection, XCB_PROP_MODE_APPEND,  (*it).second.get_window(),
            atom_list[NET_WM_STATE], XCB_ATOM_ATOM, 32, 2, &atom_list[NET_WM_STATE_STICKY]);

        xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE, (*it).second.get_window(),
            atom_list[NET_WM_DESKTOP], XCB_ATOM_CARDINAL, 32, 1, (const uint32_t []){ static_cast<uint32_t>(-1) } );

        xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE, (*it).second.get_window(),
            atom_list[NET_WM_STRUT_PARTIAL], XCB_ATOM_CARDINAL, 32, 12, strut);

        xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE, (*it).second.get_window(),
            atom_list[NET_WM_STRUT], XCB_ATOM_CARDINAL, 32, 4, strut);

        xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE, (*it).second.get_window(),
            XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, m_wm_name.size(), m_wm_name.c_str());
    }

}

void Resource::init_randr()
{
    xcb_randr_get_screen_resources_current_reply_t *rres_reply;
    xcb_randr_output_t *outputs;
    int i, num;

    rres_reply = xcb_randr_get_screen_resources_current_reply(m_connection,
            xcb_randr_get_screen_resources_current(m_connection, m_screen->root), NULL);

    if (!rres_reply) {
        std::cerr << "Failed to get current randr screen resources." << std::endl;
        return;
    }

    num = xcb_randr_get_screen_resources_current_outputs_length(rres_reply);
    outputs = xcb_randr_get_screen_resources_current_outputs(rres_reply);


    // There should be at least one output
    if (num < 1) {
        free(rres_reply);
        return;
    }

    std::map< std::string, xcb_rectangle_t > rects_map;

    // Get all outputs
    for (i = 0; i < num; i++) {
        xcb_randr_get_output_info_reply_t *oi_reply;
        xcb_randr_get_crtc_info_reply_t *ci_reply;

        oi_reply = xcb_randr_get_output_info_reply(m_connection, xcb_randr_get_output_info(m_connection, outputs[i], XCB_CURRENT_TIME), NULL);

        // Output disconnected or not attached to any CRTC ?
        if (!oi_reply || oi_reply->crtc == XCB_NONE || oi_reply->connection != XCB_RANDR_CONNECTION_CONNECTED) {
            free(oi_reply);
            continue;
        }

        ci_reply = xcb_randr_get_crtc_info_reply(m_connection,
                xcb_randr_get_crtc_info(m_connection, oi_reply->crtc, XCB_CURRENT_TIME), NULL);

        if (!ci_reply) {
            free(oi_reply);
            std::cerr << "Failed to get RandR ctrc info." << std::endl;
            free(rres_reply);
            return;
        }

        uint8_t *n_ptr = xcb_randr_get_output_info_name (oi_reply);
        int n_len = xcb_randr_get_output_info_name_length (oi_reply);
        std::string name(n_ptr, n_ptr + n_len);
        free(oi_reply);
        
        rects_map[name] = (xcb_rectangle_t){ ci_reply->x, ci_reply->y, ci_reply->width, ci_reply->height };

        free(ci_reply);
    }

    free(rres_reply);

    // Check for clones and inactive outputs
    for ( auto rect : rects_map )
    {
        for ( auto rect2 : rects_map )
        {
            if (rect.first.compare(rect2.first) != 0) {
                if (rect2.second.x >= rect.second.x && rect2.second.x + rect2.second.width <= rect.second.x + rect.second.width &&
                    rect2.second.y >= rect.second.y && rect2.second.y + rect2.second.height <= rect.second.y + rect.second.height) {
                    rects_map.erase(rect.first);
                }
            }
        }
    }

    if (rects_map.size() < 1) {
        std::cerr << "No usable RandR output found." << std::endl;
        return;
    }

    create_monitors(rects_map);
}

void Resource::create_monitors(const std::map< std::string, xcb_rectangle_t > &rects_map)
{
    int width = 0, height = 0, min_x = 100000;

    for ( auto rect : rects_map )
    {
        int h = rect.second.y + rect.second.height;
        width += rect.second.width;
        if (h >= height)
            height = h;
        if (rect.second.x < min_x)
            min_x = rect.second.x;
    }

    if (m_width < 0)
        m_width = width - m_x;

    // Use the first font height as all the font heights have been set to the biggest of the set
    if (m_height < 0 || m_height > height)
        m_height = get_max_font_height() + m_underline + DEFAULT_Y_OFFSET;

    // Check the geometry
    if (m_x + m_width > width || m_y + m_height > height) {
        std::cerr << "The geometry specified doesn't fit the screen!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Left is a positive number or zero therefore monitors with zero width are excluded
    for ( auto rect : rects_map )
    {
        if (rect.second.y + rect.second.height < m_y)
            continue;
        if (rect.second.width + rect.second.x > m_x + min_x)
        {
            if (rect.second.x < m_x + min_x)
            {
                rect.second.width -= m_x + min_x - rect.second.x;
                rect.second.x = m_x + min_x;
            }
            if (rect.second.width > min_x + m_x - rect.second.x + m_width)
                if (min_x + m_x - rect.second.x + m_width < 0)
                    rect.second.width = 0;
                else
                    rect.second.width = min_x + m_x - rect.second.x + m_width;

            if (rect.second.width != 0)
                m_monitors.emplace(std::piecewise_construct, std::make_tuple(rect.first),
                    std::forward_as_tuple(rect.second));
        }
    }
    m_default_monitor = m_monitors.begin();
}

int  Resource::get_descent(PangoContext *pango_context)
{
    if ( m_descents.find(m_font_desc) != m_descents.end() )
    {
        return m_descents[m_font_desc];
    }
    else
    {
        PangoFont *font = pango_context_load_font(pango_context, m_font_desc);
        PangoFontMetrics *metrics = pango_font_get_metrics(font, NULL);
        int ret = pango_font_metrics_get_descent(metrics) / PANGO_SCALE;

        g_object_unref(font);

        m_descents[m_font_desc] = ret;
        return ret;
    }
}

void Resource::cleanup()
{
    for ( auto font : m_fonts )
        pango_font_description_free(font.second);
    m_fonts.clear();
    m_descents.clear();

    m_monitors.clear();
    xcb_free_colormap(m_connection, m_colormap);

    if (m_gc[GC_DRAW])
        xcb_free_gc(m_connection, m_gc[GC_DRAW]);
    if (m_gc[GC_CLEAR])
        xcb_free_gc(m_connection, m_gc[GC_CLEAR]);
    if (m_gc[GC_ATTR])
        xcb_free_gc(m_connection, m_gc[GC_ATTR]);

    xcb_disconnect(m_connection);
}

xcb_visualtype_t *Resource::get_visualtype() {
    xcb_depth_iterator_t depth_iter = xcb_screen_allowed_depths_iterator (m_screen);

    for (; depth_iter.rem; xcb_depth_next (&depth_iter)) {
        if(depth_iter.data->depth == m_depth) {
            return xcb_depth_visuals_iterator(depth_iter.data).data;
        }
    }
    return NULL;
}

void Resource::redraw_all()
{
    for (auto it = m_monitors.begin(); it != m_monitors.end(); ++it)
    {
        it->second.redraw(m_gc[GC_DRAW]);
    }
}

void Resource::clear_all()
{
    for (auto it = m_monitors.begin(); it != m_monitors.end(); ++it)
    {
        it->second.clear();
    }
}