#pragma once

#include <string>

#include "Types.hpp"

namespace limebar {

class Area
{
	friend std::ostream& operator<<(std::ostream &os, const Area &obj);
public:
	Area(uint8_t button, uint16_t begin, uint16_t end, Alignment align, const std::string &command, bool print_pos);
	Area() = delete;
	Area(Area &that) = delete;
	Area(const Area &that) = delete;
	Area &operator=( Area &rhs ) = delete;
	Area &operator=( const Area &rhs ) = delete;

	bool is_valid(uint16_t pos, uint8_t button) const;
	void shift(uint16_t delta);

private:
	uint16_t  									m_begin, m_end;
	Alignment 									m_align;
	uint8_t   									m_button;
	std::string                                 m_command;
	bool										m_print_pos;
};

std::ostream& operator<<(std::ostream &os, const Area &obj);

}