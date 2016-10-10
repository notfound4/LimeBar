#include "Wrapper/Wrapper_Module.hpp"
#include "Wrapper/Module_Wrapper.hpp"
#include "remind/remind.hpp"
#include "Wrapper/Remind/Remind_Thread.hpp"

#include <cstring>
#include <iostream>
#include <sstream>

namespace limebar
{
	class Wrapper_remind : public Wrapper_Module {
	public:
		explicit Wrapper_remind(Module_wrapper *parent) : Wrapper_Module(parent)
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
           		std::cerr << "Remind: wrong number of arguments." << std::endl;
           		return;
           	}
           	if (args[1] != 0)
           	{
           		id += " ";
           		id += args[1];
           	}
           	else
           	{
           		std::cerr << "Remind: wrong number of arguments." << std::endl;
           		return;
           	}
           	if (args[2] != 0)
           	{
           		id += " ";
           		id += args[2];
           	}
           	else
           	{
           		std::cerr << "Remind: wrong number of arguments." << std::endl;
           		return;
           	}
           	if (args[3] != 0)
           	{
           		std::cerr << "Remind: wrong number of arguments." << std::endl;
           		return;
           	}

           	if (!m_parent->check_thread(cat, id))
           	{
           		unsigned int nb = static_cast<Module_remind*>(LIMEBAR.get_module("remind"))->get_entries().size();
           		std::ostringstream os;
           		os << nb;
           		char *const arg_fork[5] = {"dzen_remind", args[1], args[2], const_cast<char*>(os.str().c_str()), 0};

				int input[2];
				int output[2];
				if(pipe(input) == -1)
				{
					std::cerr << "Remind wrapper: cannot open pipe." << std::endl;
					return;
				}
				if(pipe(output) == -1)
				{
					std::cerr << "Remind wrapper: cannot open pipe." << std::endl;
					close(input[0]);
					close(input[1]);
					return;
				}
				LIMEBAR.add_fd({ .fd = output[0], .events = POLLIN, .revents = 0 }, [&](struct pollfd &fd, bool &reparse, bool &redraw){
					if (fd.revents & POLLIN) {
						remove_entry(fd.fd);
						reparse = true;
						redraw = true;
					}
				});
				m_parent->get_threads()[cat].reset(new limebar::Remind_Thread(arg_fork, id, input, output, nb));

				int i = 0;
				for (; i < nb and i < static_cast<Module_remind*>(LIMEBAR.get_module("remind"))->get_entries().size(); ++i)
				{
					write( input[1], static_cast<Module_remind*>(LIMEBAR.get_module("remind"))->get_entries()[i].c_str(),
						strlen(static_cast<Module_remind*>(LIMEBAR.get_module("remind"))->get_entries()[i].c_str()) );
					write( input[1], "\n", 1 );
				}
				for(; i < nb; ++i)
				{
					write( input[1], "\n", 1 );
				}
   			}
		}

	private:
		void remove_entry(int fd)
		{
			char buf[256];
			char *tmp = buf;
			int ret;
			if ( ( ret = read(fd, buf, sizeof(buf)) ) > 0 )
			{
				buf[ret-1] = 0;
				std::istringstream ss(buf);
				std::string item;
				while (std::getline(ss, item, '\n')) {
					std::size_t found = item.find_first_not_of(" ");
					if (found != std::string::npos)
						static_cast<Module_remind*>(LIMEBAR.get_module("remind"))->remove_entry(item.substr(found));
				}
			}
			limebar::Wrapper_Thread *thread = m_parent->get_threads()["dzen"].get();
			if ( thread != nullptr and thread->get_id().substr(0, 6).compare("remind") == 0 )
			{
				limebar::Remind_Thread *remind_thread = static_cast<limebar::Remind_Thread *>(thread);
				remind_thread->print_entries();
			}
		}
	};
}

// the class factories
extern "C" limebar::Wrapper_Module* create(limebar::Module_wrapper *parent) {
    return new limebar::Wrapper_remind(parent);
}

extern "C" void destroy(limebar::Wrapper_Module* p) {
    delete p;
}