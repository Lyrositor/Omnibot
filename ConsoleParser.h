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

#ifndef CONSOLEPARSER_H
#define CONSOLEPARSER_H

#include <fstream>
#include <map>
#include <string>
#include <vector>

using namespace std;

class ConsoleParser {

public:
    ConsoleParser(ifstream& file);
    vector<string> operator[](string key);
    vector<string> keys();

private:
    map<string, vector<string>*> data;

};

#endif // CONSOLEPARSER_H
