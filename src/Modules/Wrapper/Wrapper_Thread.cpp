#include "Wrapper/Wrapper_Thread.hpp"

#include <iostream>

extern "C" {
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
}

limebar::Wrapper_Thread::Wrapper_Thread(char *const*args, const std::string &id, int input, int output) : m_id(id)
{
	m_pid = fork();
    if (m_pid == -1) {
       std::cerr << "Wrapper thread: error while forking." << std::endl;
       return;
    }

    if (m_pid == 0) {
    	if (input >= 0)
    		dup2(input, STDIN_FILENO);
    	if (output >= 0)
    		dup2(output, STDOUT_FILENO);
        setpgid(0,0);
		execvp (args[0], args);
    }
}

limebar::Wrapper_Thread::~Wrapper_Thread()
{
	pid_t w;
	w = waitpid(m_pid, NULL, 0);
}

std::string &limebar::Wrapper_Thread::get_id()
{
	return m_id;
}

pid_t limebar::Wrapper_Thread::get_pid()
{
	return m_pid;
}

void limebar::Wrapper_Thread::kill()
{
    ::kill(-m_pid, SIGTERM);
}