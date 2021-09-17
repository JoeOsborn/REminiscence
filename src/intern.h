
/*
 * REminiscence - Flashback interpreter
 * Copyright (C) 2005-2015 Gregory Montoir (cyx@users.sourceforge.net)
 */

#ifndef INTERN_H__
#define INTERN_H__

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#include <retro_inline.h>
#include <libretro.h>

#include "intern_structs.h"

extern retro_log_printf_t          log_cb;

template<typename T>
static INLINE void SWAP(T &a, T &b) {
	T tmp = a;
	a = b;
	b = tmp;
}

struct AnimBuffers {
	struct AnimBufferState *_states[4];
	uint8_t         _curPos[4];

	void addState(uint8_t stateNum, int16_t x, int16_t y, const uint8_t *dataPtr, struct LivePGE *pge, uint8_t w,
	              uint8_t h);
};

extern Options    g_options;

#endif // INTERN_H__
