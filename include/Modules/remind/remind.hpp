#pragma once

#include "Module.hpp"

#include <string>
#include <vector>

extern "C" {
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <X11/Xresource.h>
}

namespace limebar
{

	class Module_remind : public Module {
	public:
		Module_remind();
		~Module_remind();

		void add_entry(const std::string &entry);
		void remove_entry(const std::string &entry);
		std::vector< std::string > &get_entries();

	private:
		void config(XrmDatabase &db);
		void got_update(struct pollfd &fd, bool &reparse, bool &redraw);
		void update();

	private:
		std::string 				m_file;
		int 						m_fd;
		std::vector< std::string >	m_entries;
	};

}