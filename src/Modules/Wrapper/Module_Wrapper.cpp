#include "Wrapper/Module_Wrapper.hpp"

#include "Bar.hpp"
#include "Types.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <thread>
#include <chrono>

extern "C" {
#include <dlfcn.h>
#include <signal.h>
#include <unistd.h>
}

void HandleSignal(int sig, siginfo_t *si, void *context);

std::function<void(int, siginfo_t*, void*)> jumper;

limebar::Module_wrapper::Module_wrapper()
{
	if(pipe(m_pipe) == -1)
	{
		std::cerr << "Module wrapper: cannot open pipe." << std::endl;
		m_pipe[0] = -1;
		m_pipe[1] = -1;
		return;
	}

	LIMEBAR.add_fd({ .fd = m_pipe[0], .events = POLLIN, .revents = 0 }, [&](struct pollfd &fd, bool &reparse, bool &redraw){
		handle_output();
	});

	LIMEBAR.add_init([&]{init();});
}

limebar::Module_wrapper::~Module_wrapper()
{
	if(m_pipe[0] > 0)
		close(m_pipe[0]);
	if(m_pipe[1] > 0)
		close(m_pipe[1]);

	for (std::map< std::string, limebar::wrapper_module_entry >::iterator it = m_modules.begin(); it != m_modules.end(); ++it)
	{
		it->second.m_destroyer(it->second.m_module);
		dlclose(it->second.m_library);
	}
	m_modules.clear();

	sigset_t mask, oldmask, allmask;
	sigfillset(&allmask);
	sigfillset(&mask);
   	sigdelset(&mask, SIGCHLD);

   	sigprocmask (SIG_BLOCK, &allmask, &oldmask);
	while(!m_threads.empty())
	{
		m_threads.begin()->second->kill();
		sigsuspend (&mask);
	}
	sigprocmask (SIG_SETMASK, &oldmask, NULL);
}

void limebar::Module_wrapper::handle_output()
{
	char buf[512];
	int ret;
	if ( ( ret = read(m_pipe[0], buf, sizeof(buf)) ) > 0 )
	{
		buf[ret-1] = 0;
		char *split_str[32];
		split_cmd(buf, strlen(buf)+1, split_str, 32);

		std::string module_str = split_str[0];
		Wrapper_Module *mod = add_module(module_str);
		if (mod != nullptr)
			mod->handle_output(split_str);
	}
}

void limebar::Module_wrapper::init()
{
	m_std_out = dup(STDOUT_FILENO);
    if(m_std_out == -1){
    	std::cerr << "Wrapper: error in dup(STDOUT_FILENO)" << std::endl;
        return;
    }
	if (m_pipe[1] != -1)
	{
		dup2(m_pipe[1], STDOUT_FILENO);
		close(m_pipe[1]);
		m_pipe[1] = -1;

		struct sigaction sVal;
		sVal.sa_flags = SA_SIGINFO;
		sVal.sa_sigaction = HandleSignal;
		jumper = [&](int sig, siginfo_t *si, void *context){handle_signal(sig, si, context);};
		sigaction(SIGCHLD, &sVal, NULL);
	}
}

int limebar::Module_wrapper::get_std_output()
{
	return m_std_out;
}

limebar::Wrapper_Module *limebar::Module_wrapper::add_module(const std::string &name)
{
	std::map< std::string, wrapper_module_entry >::iterator it = m_modules.find(name);
	if (it != m_modules.end())
		return it->second.m_module;

	std::string library_str = "libwrapper-";
	library_str += name;
	library_str += ".so";

	limebar::wrapper_module_entry entry;

	// load the triangle library
	entry.m_library = dlopen(library_str.c_str(), RTLD_NOW);
	if (!entry.m_library) {
	    std::cerr << "Cannot load library: " << dlerror() << std::endl;
	    return nullptr;
	}

	// reset errors
	dlerror();
	
	// load the symbols
	entry.m_creator = (create_wrapper_module_t*) dlsym(entry.m_library, "create");
	const char* dlsym_error = dlerror();
	if (dlsym_error) {
	    std::cerr << "Cannot load symbol create: " << dlsym_error << std::endl;
	    dlclose(entry.m_library);
	    return nullptr;
	}
	
	entry.m_destroyer = (destroy_wrapper_module_t*) dlsym(entry.m_library, "destroy");
	dlsym_error = dlerror();
	if (dlsym_error) {
	    std::cerr << "Cannot load symbol destroy: " << dlsym_error << std::endl;
	    dlclose(entry.m_library);
	    return nullptr;
	}

	// create an instance of the class
	entry.m_module = entry.m_creator(this);

	m_modules[name] = entry;

	return entry.m_module;
}

void limebar::Module_wrapper::handle_signal(int sig, siginfo_t *si, void *context)
{
	if(si->si_code == CLD_EXITED || si->si_code == CLD_KILLED)
		remove_thread(si->si_pid);
}

void limebar::Module_wrapper::add_thread(const std::string &category, char *const*args, const std::string &id, int input, int output)
{
	std::map< std::string, std::unique_ptr<Wrapper_Thread> >::iterator it = m_threads.find(category);
	if (it != m_threads.end())
	{
		bool do_exit = (it->second->get_id() == id);
		sigset_t mask, oldmask, allmask;
		sigfillset(&allmask);
		sigfillset(&mask);
   		sigdelset(&mask, SIGCHLD);

   		sigprocmask (SIG_BLOCK, &allmask, &oldmask);
		it->second->kill();
		sigsuspend (&mask);
		sigprocmask (SIG_SETMASK, &oldmask, NULL);

		if (do_exit)
			return;
    	std::this_thread::sleep_for (std::chrono::milliseconds(300));
	}

	Wrapper_Thread *t = new Wrapper_Thread(args, id, input, output);
	m_threads[category].reset(t);
}

void limebar::Module_wrapper::remove_thread(pid_t pid)
{
	for (std::map< std::string, std::unique_ptr<Wrapper_Thread> >::iterator it=m_threads.begin(); it!=m_threads.end(); ++it)
	{
		if (it->second->get_pid() == pid)
		{
			m_threads.erase(it);
			return;
		}
	}
}

// the class factories
extern "C" limebar::Module* create() {
    return new limebar::Module_wrapper;
}

extern "C" void destroy(limebar::Module* p) {
    delete p;
}

void HandleSignal(int sig, siginfo_t *si, void *context)
{
    jumper(sig, si, context);
}