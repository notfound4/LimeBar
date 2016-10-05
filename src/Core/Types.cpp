#include "Types.hpp"

#include <iostream>

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

std::string escape(const std::string &input, std::string s)
{
	std::string ret;

	size_t pos_i = 0, pos_j = input.find(s);
	while ( pos_j != std::string::npos )
	{
		unsigned int count = 0;
		while(pos_j > pos_i and input[--pos_j] == '\\' )
		{
			count++;
		}
		if (input[pos_j] != '\\') pos_j++;

		ret.append(input, pos_i, pos_j - pos_i);
		ret.append(2*count + 1, '\\');
		ret.append(1, ':');

		pos_i = pos_j + count + s.size();
		pos_j = input.find(s, pos_i);
	}
	ret.append(input, pos_i, std::string::npos);
	return ret;
}

void handle_sq(char *buf, size_t buflen, size_t &pos_w, size_t &pos_r)
{
	while (pos_r < buflen)
	{
		if (buf[pos_r] == '\0')
		{
			break;
		}
		else if (buf[pos_r] == '\'')
		{
			++pos_r;
			break;
		}
		else if (buf[pos_r] == '\\')
		{
			if (buf[++pos_r] == '\0')
			{
				buf[pos_w] == '\0';
				break;
			}
			else if (buf[pos_r] == '\'')
			{
				buf[pos_w++] = buf[pos_r++];
			}
			else
			{
				buf[pos_w++] = '\\';
				buf[pos_w++] = buf[pos_r++];
			}
		}
		else
		{
			buf[pos_w++] = buf[pos_r++];
		}
	}
}

void handle_dq(char *buf, size_t buflen, size_t &pos_w, size_t &pos_r)
{
	while (pos_r < buflen)
	{
		if (buf[pos_r] == '\0')
		{
			break;
		}
		else if (buf[pos_r] == '\"')
		{
			++pos_r;
			break;
		}
		else if (buf[pos_r] == '\\')
		{
			if (buf[++pos_r] == '\0')
			{
				buf[pos_w] == '\0';
				break;
			}
			else if (buf[pos_r] == '\\' or buf[pos_r] == '\"')
			{
				buf[pos_w++] = buf[pos_r++];
			}
			else
			{
				buf[pos_w++] = '\\';
				buf[pos_w++] = buf[pos_r++];
			}
		}
		else
		{
			buf[pos_w++] = buf[pos_r++];
		}
	}
}

void handle_word(char *buf, size_t buflen, size_t &pos_w, size_t &pos_r)
{
	while (pos_r < buflen)
	{
		if (buf[pos_r] == '\0')
		{
			buf[pos_w] = '\0';
			break;
		}
		else if (buf[pos_r] == '\\')
		{
			if (buf[++pos_r] == '\0')
			{
				buf[pos_w] = '\0';
				break;
			}
			buf[pos_w++] = buf[pos_r++];
		}
		else if (isspace(buf[pos_r]))
		{
			buf[pos_w++] = '\0';
			pos_r++;
			break;
		}
		else if (buf[pos_r] == '\'')
		{
			handle_sq(buf, buflen, pos_w, ++pos_r);
		}
		else if (buf[pos_r] == '\"')
		{
			handle_dq(buf, buflen, pos_w, ++pos_r);
		}
		else
		{
			buf[pos_w++] = buf[pos_r++];
		}
	}
}

void split_cmd(char *buf, size_t buflen, char **split, size_t splitlen)
{
	size_t pos_w = 0, pos_r = 0, pos_split = 0;
	while (pos_r < buflen)
	{
		if (isspace(buf[pos_r]))
		{
			pos_r++;
		}
		else if (buf[pos_r] == '\0')
		{
			break;
		}
		else
		{
			if (pos_split == splitlen-1) break;
			split[pos_split++] = buf + pos_w;
			handle_word(buf, buflen, pos_w, pos_r);
		}
	}
	split[pos_split] = '\0';
}