#include "Wrapper/Wrapper_Module.hpp"
#include "Wrapper/Module_Wrapper.hpp"

#include <iostream>

extern "C" {
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
}

namespace limebar
{
	class Module_i3msg : public Wrapper_Module {
	public:
		explicit Module_i3msg(Module_wrapper *parent) : Wrapper_Module(parent)
		{

		}

		void handle_output(char **args)
		{
			std::string cat = "i3-msg";
           	int devNull = open("/dev/null", O_WRONLY);
			sigset_t mask, oldmask, allmask;
			sigfillset(&allmask);
			sigfillset(&mask);
   			sigdelset(&mask, SIGCHLD);
	
   			sigprocmask (SIG_BLOCK, &allmask, &oldmask);
			m_parent->add_thread(cat, args, cat, -1, devNull);
			sigsuspend (&mask);
			sigprocmask (SIG_SETMASK, &oldmask, NULL);
		}
	};
}

// the class factories
extern "C" limebar::Wrapper_Module* create(limebar::Module_wrapper *parent) {
    return new limebar::Module_i3msg(parent);
}

extern "C" void destroy(limebar::Wrapper_Module* p) {
    delete p;
}