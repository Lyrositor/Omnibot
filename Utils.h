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

#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <map>
#include <vector>

using namespace std;

template <typename T> bool Contains(vector<T> list, T value) {

    return find(list.begin(), list.end(), value) != list.end();

}

template <typename T, typename A> bool Contains(map<T, A> &list, T key) {

    return list.find(key) != list.end();

}

string DecodeBase64(const string &ascdata);
void ReverseCopy(string src, unsigned char* dst, int size);

#endif // UTILS_H
