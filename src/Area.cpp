#include "Area.hpp"
#include "Resource.hpp"

#include <iostream>

Area::Area(uint8_t button, uint16_t begin, uint16_t end, Alignment align, const std::string &command, bool print_pos) :
m_begin(begin), m_end(end), m_align(align), m_button(button), m_command(command), m_print_pos(print_pos) {}

bool Area::is_valid(uint16_t pos, uint8_t button) const
{
	return button == m_button and pos > m_begin and pos < m_end;
}

void Area::shift(uint16_t delta)
{
	if (m_align == Resource::instance().m_align) {
        m_begin -= delta;
        m_end   -= delta;
    }
}

std::ostream& operator<<(std::ostream &os, const Area &obj)
{
	if (!obj.m_command.empty())
	{
		std::cout << obj.m_command;
		if (obj.m_print_pos)
			std::cout << obj.m_begin;
	}
	return os;
}