#pragma once

#include <string>

#include <cstdint>

typedef union rgba_t {
    struct {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t a;
    };
    uint32_t v;
} rgba_t;

double get_R(const rgba_t &color);
double get_G(const rgba_t &color);
double get_B(const rgba_t &color);
double get_A(const rgba_t &color);

enum Alignment {
    ALIGN_L = 0,
    ALIGN_C,
    ALIGN_R
};

enum {
    GC_DRAW = 0,
    GC_CLEAR,
    GC_ATTR,
    GC_MAX
};

enum {
	ATTR_OVERLINE  = 1<<0,
	ATTR_UNDERLINE = 1<<1
};

bool is_escaped(std::string &input, size_t &pos);
bool is_escaped(std::string &input, size_t &pos_i, size_t &pos_j);

size_t find_non_escaped(std::string &input, std::string s, size_t pos);
size_t find_non_escaped(std::string &input, std::string s, size_t pos_i, size_t &pos_j);

std::string escape(const std::string &input, std::string s);