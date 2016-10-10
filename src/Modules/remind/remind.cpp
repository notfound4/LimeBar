#include "remind/remind.hpp"

#include "Bar.hpp"
#include "DatabaseHelper.hpp"

#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>

extern "C" {
#include <sys/stat.h>
#include <sys/wait.h>
}

#define REM_FILE "~/.config/remind"
#define FIFO_FILE "/tmp/remind_fifo"

std::string get_user() {
	char const* home = getenv("HOME");

	if ( home or ( home = getenv("USERPROFILE") ) )
		return home;
	else
	{
		char const *hdrive = getenv("HOMEDRIVE");
        char const *hpath  = getenv("HOMEPATH");
		return std::string(hdrive) + hpath;
	}
}

limebar::Module_remind::Module_remind() : m_file(REM_FILE)
{
	struct stat st;
	if (stat(FIFO_FILE, &st) != 0)
		mkfifo(FIFO_FILE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

	m_fd = open(FIFO_FILE, O_RDONLY | O_NONBLOCK);

	LIMEBAR.add_fd({ .fd = m_fd, .events = POLLIN, .revents = 0 }, [&](struct pollfd &fd, bool &reparse, bool &redraw){
		got_update(fd, reparse, redraw);
	});

	LIMEBAR.add_config([&](XrmDatabase &db){config(db);});

	int pipes[2];
	if(pipe(pipes) == -1)
	{
		std::cerr << "Module wrapper: cannot open pipe." << std::endl;
		pipes[0] = -1;
		pipes[1] = -1;
		return;
	}
	pid_t pid = fork();
	if (pid == -1) {
	   	std::cerr << "Remind: error while forking." << std::endl;
	   	return;
	}

	if (pid == 0) {
		std::string file_str;
		if (m_file[0] == '~')
		{
			file_str = get_user();
			file_str += m_file.substr(1);
		}
		else
		{
			file_str = m_file;
		}
		char *const args[5] = {"remind", "-q", "-g", const_cast<char*>(file_str.c_str()), 0};
		close(pipes[0]);
		dup2(pipes[1], STDOUT_FILENO);
		close(pipes[1]);
		execvp (args[0], args);
	}
	close(pipes[1]);
	pid_t w = waitpid(pid, NULL, 0);

	char buf[1024];
	int ret;
	if ( ( ret = read(pipes[0], buf, sizeof(buf)) ) > 0 )
	{
		buf[ret-1] = 0;
		std::istringstream ss(buf);
		std::string item;
		bool is_first = true;
		while (std::getline(ss, item, '\n')) {
			if (!item.empty())
			{
				if (is_first)
					is_first = false;
				else
				{
					add_entry(item);
				}
			}
		}
	}
	close(pipes[0]);

  	std::ostringstream formatted;
  	formatted << std::right << std::setw(2) << m_entries.size();
  	LIMEBAR.get_labels()["remind_nb"] = formatted.str();
}

limebar::Module_remind::~Module_remind()
{
	close(m_fd);
}

void limebar::Module_remind::add_entry(const std::string &entry)
{
	m_entries.push_back(entry);
  	std::ostringstream formatted;
  	formatted << std::right << std::setw(2) << m_entries.size();
  	LIMEBAR.get_labels()["remind_nb"] = formatted.str();
}

void limebar::Module_remind::remove_entry(const std::string &entry)
{
	std::vector< std::string >::iterator it = m_entries.begin();
	for ( ; it != m_entries.end(); ++it)
	{
		if (it->compare(entry) == 0)
		{
			m_entries.erase(it);
			break;
		}
	}
  	std::ostringstream formatted;
  	formatted << std::right << std::setw(2) << m_entries.size();
  	LIMEBAR.get_labels()["remind_nb"] = formatted.str();
}

std::vector< std::string > &limebar::Module_remind::get_entries()
{
	return m_entries;
}

void limebar::Module_remind::config(XrmDatabase &db)
{
	limebar::DatabaseHelper::get_string( db, "limebar.remind.file", "limebar.remind.file", m_file );
}

void limebar::Module_remind::got_update(struct pollfd &fd, bool &reparse, bool &redraw)
{
	if (fd.revents & POLLHUP) {
		LIMEBAR.remove_fd(m_fd);
		close(m_fd);
		m_fd = open(FIFO_FILE, O_RDONLY | O_NONBLOCK);
		LIMEBAR.add_fd({ .fd = m_fd, .events = POLLIN, .revents = 0 }, [&](struct pollfd &fd, bool &reparse, bool &redraw){
			got_update(fd, reparse, redraw);
		});
	}
	if (fd.revents & POLLIN) {
		update();
		reparse = true;
		redraw = true;
	}
}

void limebar::Module_remind::update()
{
	char buf[1024];
	int ret;
	if ( ( ret = read(m_fd, buf, sizeof(buf)) ) > 0 )
	{
		buf[ret-1] = 0;
		std::istringstream ss(buf);
		std::string item;
		while (std::getline(ss, item, '\n')) {
			if (!item.empty())
			{
				add_entry(item);
			}
		}
	}
}

// the class factories
extern "C" limebar::Module* create() {
    return new limebar::Module_remind;
}

extern "C" void destroy(limebar::Module* p) {
    delete p;
}