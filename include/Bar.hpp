#pragma once

#include <map>

extern "C" {
#include <poll.h>
}

namespace limebar {

class Bar
{
public:
	explicit Bar();
	void loop();
	void parse_input(const std::string &input);

private:
	void parse_non_label(const std::string &input);
	void parse_non_label(const std::string &input, size_t &pos_i);
	void handle_string(const std::string &input, size_t pos_i, size_t pos_j);
	void handle_attr(const std::string &input, size_t &pos_i);

private:
	struct pollfd                        m_pollin[2];
	std::map< std::string, std::string > m_labels_map;
};

}