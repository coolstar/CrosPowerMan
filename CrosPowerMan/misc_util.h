/* Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __CROS_EC_MISC_UTIL_H
#define __CROS_EC_MISC_UTIL_H

/* Don't use a macro where an inline will do... */
static int MIN(int a, int b) { return a < b ? a : b; }

/**
 * Check if a string contains only printable characters.
 *
 * @param buf		Null-terminated string to check
 * @return non-zero if buf contains only printable characters; zero if not.
 */
int is_string_printable(const char *buf);
#endif
