#include "Bar.hpp"
#include "DatabaseHelper.hpp"
#include "Module.hpp"

#include <ctime>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

extern "C" {
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
}

#define FIFO_FILE "/tmp/portage_fifo"
#define UPDATE_FILE "/etc/limebar/portage_nb"
#define UPDATING_FILE "/etc/limebar/portage_updating"
#define TIME_FILE "/usr/portage/metadata/timestamp.chk"

#define DATE_FORMAT "%d-%m-%Y"

namespace limebar
{
	class Module_portage : public Module {
	public:
		Module_portage() : m_date_format(DATE_FORMAT)
		{
			struct stat st;
    		if (stat(FIFO_FILE, &st) != 0)
    			mkfifo(FIFO_FILE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

    		m_fd = open(FIFO_FILE, O_RDONLY | O_NONBLOCK);

			LIMEBAR.add_fd({ .fd = m_fd, .events = POLLIN, .revents = 0 }, [&](struct pollfd &fd, bool &reparse, bool &redraw){
        		got_update(fd, reparse, redraw);
			});

			LIMEBAR.add_config([&](XrmDatabase &db){config(db);});

    		update();
		}

		~Module_portage()
		{
			close(m_fd);
		}

	private:
		void config(XrmDatabase &db)
		{
			limebar::DatabaseHelper::get_string( db, "limebar.portage.date-format", "limebar.portage.date-format", m_date_format );
		}

		void got_update(struct pollfd &fd, bool &reparse, bool &redraw)
		{
			if (fd.revents & POLLHUP) {
				LIMEBAR.remove_fd(m_fd);
				close(m_fd);
				m_fd = open(FIFO_FILE, O_RDONLY | O_NONBLOCK);
				LIMEBAR.add_fd({ .fd = m_fd, .events = POLLIN, .revents = 0 }, [&](struct pollfd &fd, bool &reparse, bool &redraw){
					got_update(fd, reparse, redraw);
				});
			}
			if (fd.revents & POLLIN) { // The event comes from i3
				update();
				reparse = true;
				redraw = true;
  			}
		}

		void update()
		{
			char buf[256];
			int ret;
			while ( ( ret = read(m_fd, buf, sizeof(buf)) ) > 0 ){}
	
			std::ifstream file(TIME_FILE);
			if (file.is_open())
  			{
  				std::string line;
  			  	getline (file, line);
	
  			  	struct std::tm when;
  			  	std::istringstream input(line);
  			  	input  >> std::get_time(&when,"%a, %d %b %Y %H:%M:%S +0000");
	
  			  	std::ostringstream output;
  			  	output << std::put_time(&when, m_date_format.c_str());
	
  			  	LIMEBAR.get_labels()["portage_date"] = output.str();
	
  			  	file.close();
  			}
			file.open(UPDATING_FILE);
			if (file.is_open())
  			{
  				std::string line;
  			  	getline (file, line);
  			  	file.close();

  			  	if (line.empty())
  			  	{
					file.open(UPDATE_FILE);
					if (file.is_open())
  					{
  					  	getline (file, line);
  			  			file.close();
  			  		}
  			  	}
	
  			  	std::ostringstream formatted;
  			  	formatted << std::right << std::setw(5) << line;
  			  	LIMEBAR.get_labels()["portage_nb"] = formatted.str();
  			}
		}

	private:
		int m_fd;
		std::string m_date_format;
	};
}

// the class factories
extern "C" limebar::Module* create() {
    return new limebar::Module_portage;
}

extern "C" void destroy(limebar::Module* p) {
    delete p;
}