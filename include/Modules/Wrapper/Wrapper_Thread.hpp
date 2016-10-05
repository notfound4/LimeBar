#pragma once

#include <string>

extern "C" {
#include <unistd.h>
}

namespace limebar
{
	class Wrapper_Thread
	{
	public:
		Wrapper_Thread(char *const*args, const std::string &id, int input, int output);
		~Wrapper_Thread();

		Wrapper_Thread() = delete;
		Wrapper_Thread(Wrapper_Thread& that) = delete;
		Wrapper_Thread(const Wrapper_Thread& that) = delete;
		Wrapper_Thread &operator=( Wrapper_Thread &rhs ) = delete;
		Wrapper_Thread &operator=( const Wrapper_Thread &rhs ) = delete;

		std::string &get_id();
		pid_t get_pid();

		void kill();

	private:
		std::string m_id;
		pid_t       m_pid;
	};
}