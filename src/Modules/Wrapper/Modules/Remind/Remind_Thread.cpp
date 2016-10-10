#include "remind/remind.hpp"
#include "Wrapper/Remind/Remind_Thread.hpp"

#include "Bar.hpp"

#include <cstring>

#define MAX_LINES 10

limebar::Remind_Thread::Remind_Thread(char *const*args, const std::string &id, int input[2], int output[2], unsigned int nb_lines)
: Wrapper_Thread(args, id, input, output)
{
	m_input_fd = output[0];
	m_output_fd = input[1];

	if (nb_lines > MAX_LINES)
		m_nb_lines = MAX_LINES;
	else
		m_nb_lines = nb_lines;
}

limebar::Remind_Thread::~Remind_Thread()
{
	LIMEBAR.remove_fd(m_input_fd);
	close(m_input_fd);
	close(m_output_fd);
}

void limebar::Remind_Thread::print_entries()
{
	int i = 0;
	for (; i < m_nb_lines and i < static_cast<Module_remind*>(LIMEBAR.get_module("remind"))->get_entries().size(); ++i)
	{
		write( m_output_fd, static_cast<Module_remind*>(LIMEBAR.get_module("remind"))->get_entries()[i].c_str(),
			strlen(static_cast<Module_remind*>(LIMEBAR.get_module("remind"))->get_entries()[i].c_str()) );
		write( m_output_fd, "\n", 1 );
	}
	for(; i < m_nb_lines; ++i)
	{
		write( m_output_fd, "\n", 1 );
	}
}