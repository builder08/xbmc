/*-------------------------------------------------------------------------
 * C-Pluff, a plug-in framework for C
 * Copyright 2007 Johannes Lehtinen
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *-----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "test.h"

static int testvar;

static void cause_fatal_error(void) {
	cp_context_t *ctx;
	
	cp_init();
	ctx = init_context(CP_LOG_ERROR + 1, NULL);
	cp_release_info(ctx, &testvar);
	cp_destroy();
}

void fatalerrordefault(void) {
	cause_fatal_error();
}

static void error_handler(const char *msg) {
	free_test_resources();
	exit(0);
}

void fatalerrorhandled(void) {
	cp_set_fatal_error_handler(error_handler);
	cause_fatal_error();
	free_test_resources();
	exit(1);
}

void fatalerrorreset(void) {
	cp_set_fatal_error_handler(error_handler);
	cp_set_fatal_error_handler(NULL);
	cause_fatal_error();
}
