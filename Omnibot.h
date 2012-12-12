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

#ifndef OMNIBOT_H
#define OMNIBOT_H

#include <string>
#include <thread>

#include <SDL/plSDLMgr.h>

#include "AuthClient.h"
#include "GameClient.h"

// Uru Live build ID.
#define BUILD_ID 912
static const plUuid s_moulUuid("ea489821-6c35-4bd0-9dae-bb17c585e680");

using namespace std;

// Omnibot main class.
class Omnibot {

public:
    Omnibot();
    ~Omnibot();

    AuthClient* authClient;
    GameClient* gameClient;

    uint32_t activePlayer;
    plString currentAgeName;
    uint32_t currentAgeId;

    bool loadIni();
    void loginPlayer();
    void logoutActivePlayer();
    void run();
    void setActivePlayer();
    void startPingLoop();
    void stopPingLoop();

    // Auth Server events.
    void setEncryptionKeys(uint32_t k0, uint32_t k1, uint32_t k2, uint32_t k3);
    void loadStateDescriptors(hsStream* S);

    // Game Server events.
    void checkCurrentAge();
    void joinAge(plString name, plUuid uuid);
    void setOnline(uint32_t playerId, plString ageFilename, plUuid ageUuid);
    void startGameClient(uint32_t serverAddr, plUuid ageId, uint32_t mcpId, uint32_t ageVaultId);

    struct {
        struct {
            unsigned char N[64];
            unsigned char X[64];
            string Host;
        } Auth;
        struct {
            unsigned char N[64];
            unsigned char X[64];
            string Host;
        } Game;
        struct {
            string Name;
            string Pass;
            uint32_t KI;
        } User;
    } Server;

    plResManager* getResManager() const { return resMgr; }
    plSDLMgr* getSDLMgr() const { return sdlMgr; }

private:
    plResManager* resMgr;
    plSDLMgr* sdlMgr;
    thread* pingThread;

    uint32_t ntdKeys[4];

};

#endif // OMNIBOT_H
