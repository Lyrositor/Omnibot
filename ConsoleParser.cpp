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

#include <utility>

#include "ConsoleParser.h"

enum ParseState { COMMAND, ARGUMENT, STRING, COMMENT };

// Start parsing the specified server.ini file for Plasma console commands.
ConsoleParser::ConsoleParser(ifstream& file) {

    ParseState state = COMMAND;
    string cmd;
    string arg;
    vector<string>* args = new vector<string>();
    while (!file.eof()) {
        char c = file.get();
        if (c == '\r')
            continue;
        switch (state) {
            case COMMAND:
                // Reads the first "word" on the line.
                switch (c) {
                    case ' ':
                        if (cmd.length() > 0)
                            state = ARGUMENT;
                        break;
                    case '#':
                        state = COMMENT;
                        break;
                    case '\n':
                        if (cmd.length() > 0) {
                            data[cmd] = args;
                            cmd.clear();
                            args->clear();
                        }
                        break;
                    default:
                        cmd.append(1, c);
                        break;
                }
                break;
            case ARGUMENT:
                // Reads space separated "words".
                switch(c) {
                    case '\n':
                    case '#':
                    case '"':
                    case ' ':
                        if (arg.length() > 0) {
                            args->push_back(arg);
                            arg.clear();
                        }
                        if (c == '\n') {
                            state = COMMAND;
                            data[cmd] = args;
                            cmd.clear();
                            args = new vector<string>();
                        } else if (c == '#') {
                            state = COMMENT;
                            data[cmd] = args;
                            cmd.clear();
                            args = new vector<string>();
                        } else if (c == '"') {
                            state = STRING;
                        }
                        break;
                    default:
                        arg.append(1, c);
                        break;
                }
                break;
            case STRING:
                // Reads until a closing " or the end of the line; don't
                // bother with escape sequences for now.
                if (c == '"' || c == '\n') {
                    args->push_back(arg);
                    arg.clear();
                    state = ARGUMENT;
                } else {
                    arg.append(1, c);
                }
            case COMMENT:
                // Ignores all characters until a line break.
                if (c == '\n')
                    state = COMMAND;
                break;
        }
    }
    delete args;

}

// Gets the value for the specified console command.
vector<string> ConsoleParser::operator[](string key) {

    return *data[key];

}

// Gets a list of console command names contained in the file.
vector<string> ConsoleParser::keys() {

    vector<string> returnValues;
    for (pair<string, vector<string>*> entry : data)
        returnValues.push_back(entry.first);
    return returnValues;

}
