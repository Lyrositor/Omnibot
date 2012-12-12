/*
    Copyright (C) 2012 Lyrositor

    This file is part of Omnibot.

    Omnibot is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Omnibot is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Omnibot.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

#include "Utils.h"

static const char reverse_table[128] = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64
};

// Base64 decoding to plain text.
// Source: http://stackoverflow.com/questions/5288076/doing-base64-encoding-and-decoding-in-openssl-c
string DecodeBase64(const string &ascdata) {

    string retVal;
    const string::const_iterator last = ascdata.end();
    int bits_collected = 0;
    unsigned int accumulator = 0;

    for (string::const_iterator i = ascdata.begin(); i != last; ++i) {
        const int c = *i;
        if (isspace(c) || c == '=') {
            // Skip whitespace and padding. Be liberal in what you accept.
            continue;
        }
        if ((c > 127) || (c < 0) || (reverse_table[c] > 63)) {
            throw invalid_argument("This contains characters not legal in a base64 encoded string."); 
        }
        accumulator = (accumulator << 6) | reverse_table[c];
        bits_collected += 6;
        if (bits_collected >= 8) {
            bits_collected -= 8;
            retVal += (char)((accumulator >> bits_collected) & 0xffu);
        }
    }
    return retVal;

}

// Copies a string in reverse.
void ReverseCopy(string src, unsigned char* dst, int size) {

    for (int i = 0; i < size; i++)
        dst[i] = src[size - i - 1];

}
