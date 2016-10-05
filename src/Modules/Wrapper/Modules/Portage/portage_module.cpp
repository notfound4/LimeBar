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
	class Module_portage : public Wrapper_Module {
	public:
		explicit Module_portage(Module_wrapper *parent) : Wrapper_Module(parent)
		{

		}

		void handle_output(char **args)
		{
			std::string cat = "dzen";
			std::string id = "";

           	if (args[0] != 0)
           	{
           		id += args[0];
           	}
           	else
           	{
           		std::cerr << "Calendar: wrong number of arguments." << std::endl;
           		return;
           	}
           	if (args[1] != 0)
           	{
           		id += " ";
           		id += args[1];
           	}
           	else
           	{
           		std::cerr << "Calendar: wrong number of arguments." << std::endl;
           		return;
           	}
           	if (args[2] != 0)
           	{
           		id += " ";
           		id += args[2];
           	}
           	else
           	{
           		std::cerr << "Calendar: wrong number of arguments." << std::endl;
           		return;
           	}
           	if (args[3] != 0)
           	{
           		std::cerr << "Calendar: wrong number of arguments." << std::endl;
           		return;
           	}

           	char *const arg_fork[4] = {"dzen_portage", args[1], args[2], 0};
			
			m_parent->add_thread(cat, arg_fork, id, -1, m_parent->get_std_output());
		}
	};
}

// the class factories
extern "C" limebar::Wrapper_Module* create(limebar::Module_wrapper *parent) {
    return new limebar::Module_portage(parent);
}

extern "C" void destroy(limebar::Wrapper_Module* p) {
    delete p;
}