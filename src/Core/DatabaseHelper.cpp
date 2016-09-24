#include "DatabaseHelper.hpp"

#include <boost/regex.hpp>

void limebar::DatabaseHelper::get_boolean (XrmDatabase &db, const char *name, const char *cl, bool &destination)
{
	XrmValue v;
    char *type;
    int status;

    status = XrmGetResource(db, name, cl, &type, &v);
    if (status) {
        std::string tmp_str = (char *) v.addr;
        if ( tmp_str.compare("true") == 0 )
            destination = true;
        else if ( tmp_str.compare("false") == 0 )
            destination = false;
    }
}

void limebar::DatabaseHelper::get_color   (XrmDatabase &db, const char *name, const char *cl, std::string &destination)
{
    XrmValue v;
    char *type;
    int status;

    status = XrmGetResource(db, name, cl, &type, &v);
    if (status) {
        std::string tmp_color = (char *) v.addr;
        if ( is_valid_color(tmp_color) )
            destination = tmp_color;
    }
}

void limebar::DatabaseHelper::get_geometry(XrmDatabase &db, const char *name, const char *cl, std::string &destination)
{
	XrmValue v;
    char *type;
    int status;

    status = XrmGetResource(db, name, cl, &type, &v);
    if (status) {
        std::string tmp_geo = (char *) v.addr;
        if ( regex_match(tmp_geo, boost::regex("(\\d*)x(\\d*)\\+(\\d*)\\+(\\d*)")) )
            destination = tmp_geo;
    }
}

void limebar::DatabaseHelper::get_string  (XrmDatabase &db, const char *name, const char *cl, std::string &destination)
{
    XrmValue v;
    char *type;
    int status;

    status = XrmGetResource(db, name, cl, &type, &v);
    if (status) {
        destination = (char *) v.addr;
    }
}

void limebar::DatabaseHelper::get_unsigned(XrmDatabase &db, const char *name, const char *cl, unsigned int &destination)
{
    XrmValue v;
    char *type;
    int status;

    status = XrmGetResource(db, name, cl, &type, &v);
    if (status) {
        std::string conv((char *) v.addr);
        try {
            int res_tmp = std::stoi(conv);
            if (res_tmp >= 0)
                destination = res_tmp;
        }
        catch (...) {}
    }
}

bool limebar::DatabaseHelper::is_valid_color(const std::string &color)
{
    return regex_match(color, boost::regex("#(?:[0-9a-fA-F]{8}|[0-9a-fA-F]{6}|[0-9a-fA-F]{3})"));
}
