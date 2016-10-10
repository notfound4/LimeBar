#pragma once

#include "Wrapper/Wrapper_Thread.hpp"

namespace limebar
{
	class Remind_Thread : public Wrapper_Thread
	{
	public:
		Remind_Thread(char *const*args, const std::string &id, int input[2], int output[2], unsigned int nb_lines);
		~Remind_Thread();

		void print_entries();

		Remind_Thread() = delete;
		Remind_Thread(Remind_Thread& that) = delete;
		Remind_Thread(const Remind_Thread& that) = delete;
		Remind_Thread &operator=( Remind_Thread &rhs ) = delete;
		Remind_Thread &operator=( const Remind_Thread &rhs ) = delete;

	private:
		int m_input_fd;
		int m_output_fd;
		unsigned int m_nb_lines;
	};
}