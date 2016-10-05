#pragma once

#include "Module.hpp"

#include "Bar.hpp"
#include "Types.hpp"
#include "Wrapper/Wrapper_Module.hpp"
#include "Wrapper/Wrapper_Thread.hpp"

#include <memory>

namespace limebar
{
	class Module_wrapper : public Module {
	public:
		Module_wrapper();

		~Module_wrapper();

		void add_thread(const std::string &category, char *const*args, const std::string &id, int input, int output);

		void remove_thread(pid_t pid);

		int get_std_output();

	private:
		void handle_output();

		void init();

		Wrapper_Module *add_module(const std::string &name);

		void handle_signal(int sig, siginfo_t *si, void *context);

	private:
		int 														m_pipe[2];
		std::map< std::string, wrapper_module_entry > 				m_modules;
		std::map< std::string, std::unique_ptr<Wrapper_Thread> > 	m_threads;
		int                                                         m_std_out;
	};
}