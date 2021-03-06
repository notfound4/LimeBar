#include "Bar.hpp"
#include "OptionPrinter.hpp"

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp" 

#include <iostream>
#include <string>

extern "C" {
#include <signal.h>
}

using namespace limebar;

namespace 
{ 
    const size_t ERROR_IN_COMMAND_LINE = 1; 
    const size_t SUCCESS = 0; 
    const size_t ERROR_UNHANDLED_EXCEPTION = 2;

}

void
cleanup (void)
{
    LIMEBAR.cleanup();
}

void
sighandle (int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
        exit(EXIT_SUCCESS);
}

int
main (int argc, char **argv)
{
    /*for (int i=0; i<argc; ++i)
    {
        std::cout << argv[i] << std::endl;
    }*/
	try
	{
        LIMEBAR.config();

        // Handle options
		namespace po = boost::program_options;
        namespace fs = boost::filesystem;

        std::string appName = fs::basename(argv[0]);
        std::string command;

        po::options_description desc("Options");
        desc.add_options()
        ("help,h", "Print help messages")
        ("geometry,g", po::value< std::string >(), "Set the window geometry. If a parameter is omitted it's filled with the default value. If the y parameter \
is specified along with the -b switch then the position is relative to the bottom of the screen. Format: widthxheight+x+y")
        ("command,c", po::value< std::string >(), "Set the formating text to be used as the input as default. Usefull if you use labels.")
        ("bottom,b", po::value< bool >(), "Dock the bar at the bottom of the screen.")
        ("dock,d", po::value< bool >(), "Force docking without asking the window manager. This is needed if the window manager isn't EWMH compliant.")
        ("font,f", po::value< std::vector< std::string > >()->multitoken(), "Define the fonts to load")
        ("permanent,p", po::value< bool >(), "Make the bar permanent, don't exit after the standard input is closed.")
        ("duplicate,D", po::value< bool >(), "Duplicate the input to all the output.")
        ("name,n", po::value< std::string >(), "Set the WM_NAME atom value for the bar.")
        ("underline,u", po::value< unsigned int >(), "Sets the underline width in pixels. The default is 1.")
        ("foreground-color,F", po::value< std::string >(), "Set the foreground color of the bar. color must be specified in the hex format \
    (#aarrggbb, #rrggbb, #rgb). If no compositor such as compton or xcompmgr is running the alpha channel is silently ignored.")
        ("background-color,B", po::value< std::string >(), "Set the background color of the bar. Accepts the same color formats as -B.")
        ("underline-color,U", po::value< std::string >(), "Set the underline color of the bar. Accepts the same color formats as -B.")
        ("modules,m", po::value< std::vector< std::string > >()->multitoken(), "Define the modules to load");

        po::variables_map vm;

        try 
        {
            po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);

            if ( vm.count("help")  )
            { 
                std::cout << "limebar - implementation of a standalone workspace bar heavily inspired by lemonbar." << std::endl << std::endl; 
                rad::OptionPrinter::printStandardAppDesc(appName, std::cout, desc); 
                return SUCCESS; 
            }
            if ( vm.count("geometry"         )  ) LIMEBAR.get_X_handler().set_geometry(vm["geometry"].as<std::string>());
            if ( vm.count("bottom"           )  ) LIMEBAR.get_X_handler().set_bottom(vm["bottom"].as<bool>());
            if ( vm.count("dock"             )  ) LIMEBAR.get_X_handler().set_dock  (vm["dock"].as<bool>());
            if ( vm.count("font"             )  )
            {
            	const std::vector< std::string > &fonts_array = vm["font"].as<std::vector< std::string > >();
            	for ( auto font_str : fonts_array )
            	{
            		LIMEBAR.get_font_handler().set_font(font_str);
            	}
            	if ( !fonts_array.empty() )
           			LIMEBAR.get_font_handler().set_font(fonts_array[0], true);
           	}
            if ( vm.count("permanent"        )  ) LIMEBAR.set_permanent                (vm["permanent"].as<bool>());
            if ( vm.count("duplicate"        )  ) LIMEBAR.set_duplicate                (vm["duplicate"].as<bool>());
            if ( vm.count("name"             )  ) LIMEBAR.get_X_handler().set_wm_name  (vm["name"].as<std::string>());
            if ( vm.count("underline"        )  ) LIMEBAR.get_X_handler().set_underline(vm["underline"].as<unsigned int>());
            if ( vm.count("command"          )  ) command                             = vm["command"].as<std::string>();
            if ( vm.count("foreground-color" )  ) LIMEBAR.get_color_handler().set_foreground(vm["foreground-color"].as<std::string>(), true);
            if ( vm.count("background-color" )  ) LIMEBAR.get_color_handler().set_background(vm["background-color"].as<std::string>(), true);
            if ( vm.count("underline-color"  )  ) LIMEBAR.get_color_handler().set_underline (vm["underline-color"].as<std::string>() , true);
            if ( vm.count("modules"          )  )
            {
                const std::vector< std::string > &modules_array = vm["modules"].as<std::vector< std::string > >();
                for ( auto module_str : modules_array )
                {
                    LIMEBAR.add_module(module_str);
                }
            }

            po::notify(vm);
        } 
        catch(boost::program_options::required_option& e) 
        { 
            rad::OptionPrinter::formatRequiredOptionError(e); 
            std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
            rad::OptionPrinter::printStandardAppDesc(appName, std::cout, desc); 
            return ERROR_IN_COMMAND_LINE; 
        } 
        catch(boost::program_options::error& e) 
        { 
            std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
            rad::OptionPrinter::printStandardAppDesc(appName, std::cout, desc); 
            return ERROR_IN_COMMAND_LINE; 
        }

        LIMEBAR.init();
        LIMEBAR.pre_render();

    	// Install the parachute!
    	atexit(cleanup);
    	signal(SIGINT, sighandle);
    	signal(SIGTERM, sighandle);

        if (!command.empty())
        {
        	LIMEBAR.parse_input(command);
        }

        LIMEBAR.loop();

	}
    catch(std::exception& e) 
    { 
        std::cerr << "Unhandled Exception reached the top of main: " 
        << e.what() << ", application will now exit" << std::endl;
        return ERROR_UNHANDLED_EXCEPTION; 

    }

	return EXIT_SUCCESS;
}