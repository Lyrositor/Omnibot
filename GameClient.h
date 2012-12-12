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

#ifndef GAMECLIENT_H
#define GAMECLIENT_H

#include <vector>

#include <game/pnGameClient.h>

#include "Vault.h"

using namespace std;

class Omnibot;

class GameClient : public pnGameClient {

public:
    GameClient(Omnibot* omnibot = 0);

    void joinAge(uint32_t serverAddr, uint32_t mcpId);
    void onJoinAgeReply(uint32_t transId, ENetError result);
    void onPingReply(uint32_t pingTimeMs);
    void onPropagateMessage(plCreatable* msg);
    void setAgeInfo(VaultNode ageInfo);
    void setPlayer(VaultPlayerInfoNode player);

    void sendAgeChat(plString message);
    void sendBuddies(plString message, vector<uint32_t> targets, int type);
    void sendPrivate(plString message, uint32_t target);

private:
    Omnibot* parent;
    uint32_t fMcpId;
    hsTArray<unsigned int> fAgePlayers;
    VaultPlayerInfoNode fPlayerNode;
    VaultNode fAgeInfoNode;

};

#endif // GAMECLIENT_H
