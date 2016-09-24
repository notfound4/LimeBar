#pragma once

#include <string>

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xresource.h>
}

namespace limebar
{
	class DatabaseHelper
	{
	public:
		static void get_boolean (XrmDatabase &db, const char *name, const char *cl, bool &destination);
		static void get_color   (XrmDatabase &db, const char *name, const char *cl, std::string &destination);
		static void get_geometry(XrmDatabase &db, const char *name, const char *cl, std::string &destination);
		static void get_string  (XrmDatabase &db, const char *name, const char *cl, std::string &destination);
		static void get_unsigned(XrmDatabase &db, const char *name, const char *cl, unsigned int &destination);
		static bool is_valid_color(const std::string &color);
	private:
		DatabaseHelper();
	};
}