#pragma once

typedef union rgba_t {
    struct {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t a;
    };
    uint32_t v;
} rgba_t;

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