#include "FontHandler.hpp"

#include "DatabaseHelper.hpp"
#include "Types.hpp"

#define FONT "Sans 12"

limebar::FontHandler::FontHandler()
{
	set_font(FONT, true);
}

limebar::FontHandler::~FontHandler()
{
	for ( auto font : m_fonts )
        pango_font_description_free(font.second);
    m_fonts.clear();
    m_descents.clear();
}

unsigned int limebar::FontHandler::get_font_height()
{
	return pango_font_description_get_size(m_current_font_desc) / PANGO_SCALE;
}

unsigned int limebar::FontHandler::get_max_font_height()
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

int limebar::FontHandler::get_descent(PangoContext *pango_context)
{
	if ( m_descents.find(m_current_font_desc) != m_descents.end() )
    {
        return m_descents[m_current_font_desc];
    }
    else
    {
        PangoFont *font = pango_context_load_font(pango_context, m_current_font_desc);
        PangoFontMetrics *metrics = pango_font_get_metrics(font, NULL);
        int ret = pango_font_metrics_get_descent(metrics) / PANGO_SCALE;

        g_object_unref(font);

        m_descents[m_current_font_desc] = ret;
        return ret;
    }
}

void limebar::FontHandler::set_font(const std::string &font_name, bool is_default)
{
	if (font_name.empty()) return;

    if (is_default)
    {
    	m_default_font_desc = get_font(font_name);
    }
    else
    {
    	m_current_font_desc = get_font(font_name);
    }
}

PangoFontDescription *limebar::FontHandler::get_current_font()
{
	return m_current_font_desc;
}

PangoFontDescription *limebar::FontHandler::get_font(const std::string &font_name)
{
    if ( m_fonts.find(font_name) == m_fonts.end() )
        m_fonts[font_name] = pango_font_description_from_string(font_name.c_str());
    return m_fonts[font_name];
}

void limebar::FontHandler::reset_to_default()
{
	m_current_font_desc = m_default_font_desc;
}

void limebar::FontHandler::config(XrmDatabase &db)
{
    std::string font_tmp;
    limebar::DatabaseHelper::get_string ( db, "limebar.font", "limebar.font", font_tmp );
    set_font(font_tmp, true);
}

void limebar::FontHandler::handle_font(std::string &input, size_t &pos_i, size_t &pos_j)
{
    if (++pos_i == pos_j) return;
    switch (input[pos_i])
    {
        case '-':
            m_current_font_desc = m_default_font_desc;
            pos_i++;
            break;
        case ':':
        {
            if (++pos_i == pos_j) return;
            size_t pos_k = find_non_escaped(input, ":", pos_i, pos_j);
            if (pos_k >= pos_j)
            {
                pos_i = pos_j;
                return;
            }
            set_font(input.substr(pos_i, pos_k - pos_i));
            pos_i = pos_k + 1;
            break;
        }
        default:
            return;
    }
}