#include "Types.hpp"

double get_R(const rgba_t &color)
{
    return static_cast<double>(color.r) / 255.;
}
double get_G(const rgba_t &color)
{
    return static_cast<double>(color.g) / 255.;
}
double get_B(const rgba_t &color)
{
    return static_cast<double>(color.b) / 255.;
}
double get_A(const rgba_t &color)
{
    return static_cast<double>(color.a) / 255.;
}

bool is_escaped(std::string &input, size_t &pos)
{
	unsigned int count = 0;
	if (pos == 0) return false;

	while(input[--pos] == '\\' )
	{
		count++;
		if (pos == 0) break;
	}
	if (input[pos] != '\\') pos++;

	input.replace(pos, count, count/2, '\\');
	pos += count/2;

	if ( (count & 1) )
		return true;
	return false;
}

bool is_escaped(std::string &input, size_t &pos_i, size_t &pos_j)
{
	unsigned int count = 0;
	if (pos_i == 0) return false;

	while(input[--pos_i] == '\\' )
	{
		count++;
		if (pos_i == 0) break;
	}
	if (input[pos_i] != '\\') pos_i++;

	input.replace(pos_i, count, count/2, '\\');
	pos_i += count/2;
	pos_j -= (count + 1)/2;

	if ( (count & 1) )
		return true;
	return false;
}

size_t find_non_escaped(std::string &input, std::string s, size_t pos)
{
	pos = input.find(s, pos);

	while ( pos != std::string::npos and is_escaped(input, pos) )
	{
		pos = input.find(s, pos + 1);
	}
	return pos;
}

size_t find_non_escaped(std::string &input, std::string s, size_t pos_i, size_t &pos_j)
{
	pos_i = input.find(s, pos_i);

	while ( pos_i < pos_j and is_escaped(input, pos_i, pos_j) )
	{
		pos_i = input.find(s, pos_i + 1);
	}
	return pos_i;
}