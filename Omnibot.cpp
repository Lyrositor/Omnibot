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

#include <ctime>
#include <iostream>
#include <readline.h>
#include <string.h>

#include "Omnibot.h"
#include "ConsoleParser.h"
#include "Utils.h"
#include "Vault.h"

// Creates Omnibot and logs the player in for the first time.
Omnibot::Omnibot() {

    // Booting up Omnibot.
    cout << "Welcome to Omnibot." << endl;
    pthread_t threads[1];
    resMgr = new plResManager(PlasmaVer::pvMoul);
    sdlMgr = new plSDLMgr();
    authClient = NULL;
    gameClient = NULL;
    pingThread = NULL;
    Vault::get(this);
    if (!loadIni())
        return;
    loginPlayer();
    setActivePlayer();
    //loadPython();

}

// Destroys Omnibot.
Omnibot::~Omnibot() {

    //unloadPython()
    delete sdlMgr;
    delete resMgr;
    if (gameClient != NULL) {
        logoutActivePlayer();
        delete gameClient;
    }
    if (authClient != NULL)
        delete authClient;

}

// Checks the current Age the player is in, and sets the Game Client to this
// Age.
void Omnibot::checkCurrentAge() {

    Vault& vault = Vault::get();
    VaultNode info = vault.findNode(currentAgeId);
    if (info != 0 && gameClient != 0) {
        VaultNode childInfo;
        vector<VaultNode> children = info.getChildren();
        for (VaultNode child: children) {
            if (child.getNodeType() == plVault::kNodeAgeInfo) {
                childInfo = child;
                break;
            }
        }
        if (childInfo != 0) {
            gameClient->setAgeInfo(childInfo);
        } else {
            gameClient->setAgeInfo(info);
        }
    }

}

// Joins the active player to the specified Age.
void Omnibot::joinAge(plString name, plUuid uuid) {

    currentAgeName = name;
    uint32_t transId = authClient->sendAgeRequest(name, uuid);
    while (!Contains(authClient->queuedAgeReplies, transId));
    ageReply a = authClient->queuedAgeReplies[transId];
    authClient->queuedAgeReplies.erase(transId);
    startGameClient(a.gameServerAddress, a.ageInstanceId, a.mcpId, a.ageVaultId);

}

// Load the server.ini file.
bool Omnibot::loadIni() {

    // Read the server.ini file.
    ifstream server;
    server.open("server.ini");
    if (!server.good()) {
        cout << "ERROR: Could not open server.ini, aborting." << endl;
        return false;
    }
    ConsoleParser ini(server);
    server.close();

    // Check that the file is valid.
    if (ini.keys().size() < 5) {
        cout << "ERROR: Incomplete server.ini." << endl;
        return false;
    }

    ReverseCopy(DecodeBase64(ini["Server.Auth.N"][0]), Server.Auth.N, 64);
    ReverseCopy(DecodeBase64(ini["Server.Auth.X"][0]), Server.Auth.X, 64);
    ReverseCopy(DecodeBase64(ini["Server.Game.N"][0]), Server.Game.N, 64);
    ReverseCopy(DecodeBase64(ini["Server.Game.X"][0]), Server.Game.X, 64);
    Server.Auth.Host = ini["Server.Auth.Host"][0];
    Server.Game.Host = ini["Server.Game.Host"][0];
    Server.User.Name = "";
    Server.User.Pass = "";
    Server.User.KI = 0;
    string name = "Server.User.Name";
    string pass = "Server.User.Pass";
    string ki = "Server.User.KI";
    if (Contains(ini.keys(), name) && Contains(ini.keys(), pass) && Contains(ini.keys(), ki)) {
        Server.User.Name = ini["Server.User.Name"][0];
        Server.User.Pass = ini["Server.User.Pass"][0];
        Server.User.KI = atoi(ini["Server.User.KI"][0].c_str());
    }
    return true;

}

// Loads the SDL settings.
void Omnibot::loadStateDescriptors(hsStream* S) {

    plEncryptedStream* str = new plEncryptedStream(PlasmaVer::pvMoul);
    str->setKey(ntdKeys);
    str->open(S, fmRead, plEncryptedStream::kEncDroid);

    sdlMgr->ReadDescriptors(str);

    delete str;
    delete S;
    S = NULL;

}

// Logins the player to the auth server.
void Omnibot::loginPlayer() {

    if (gameClient != NULL) {
        logoutActivePlayer();
        delete gameClient;
        gameClient = NULL;
    }

    if (authClient != NULL)
        delete authClient;

    while (Server.User.Name == "" || Server.User.Pass == "") {
        Server.User.Name = readline("Username: ");
        Server.User.Pass = readline("Password: ");
    }
    authClient = new AuthClient(this);
    authClient->startLogin(Server.User.Name, Server.User.Pass);
    while (authClient->loggingIn == 0);
    if (authClient->loggingIn == 2) {
        authClient->loggingIn = 0;
        Server.User.Name = "";
        Server.User.Pass = "";
        loginPlayer();
    }

}

// Logs out the active player.
void Omnibot::logoutActivePlayer() {

    if (activePlayer == 0)
        return;
    if (gameClient != NULL) {
        delete gameClient;
        gameClient = NULL;
    }
    Vault& vault = Vault::get();
    for (VaultNode child : vault.findNode(activePlayer).getChildren()) {
        if (child.getNodeType() == plVault::kNodePlayerInfo) {
            child.setInt32(0, 0);
            child.setUuid(0, plUuid());
            child.setString64(0, plString());
            if (authClient->isConnected()) {
                authClient->sendVaultNodeSave(child.getNodeIdx(), plUuid(), child);
            }
        }
    }
    delete authClient;
    authClient = NULL;
    cout << Server.User.Name << " has been logged out." << endl;
    Server.User.Name = "";
    Server.User.Pass = "";
    Server.User.KI = 0;
    activePlayer = 0;
    stopPingLoop();

}

// Runs the main Omnibot process, waiting for terminal commands.
void Omnibot::run() {

    // Display the command prompt.
    Vault& vault = Vault::get();
    char rl_prompt[32];
    snprintf(rl_prompt, 32, "Omnibot 1.%u> ", BUILD_ID);
    string input;
    while (true) {
        input = readline(rl_prompt);
        if (input == "quit" || input == "exit") {
            break;
        } else if (input == "join age") {
            if (activePlayer == 0) {
                loginPlayer();
                setActivePlayer();
            }
            plString age = readline("Age filename: ");
            plString id = readline("Age uuid: ");
            plUuid uuid = plUuid();
            try {
                uuid.fromString(id);
            }
            catch (hsBadParamException) {
                cout << "Creating new Age." << endl;
                plString instanceName = readline("Age instance name: ");
                plString ageDescription = readline("Age description: ");
                uint32_t transId = authClient->sendVaultInitAgeRequest(uuid, age, instanceName, plString(), ageDescription, 0, -1);
                while (!Contains(authClient->queuedVaultIds, transId));
                VaultNode ageNode = vault.findNode(authClient->queuedVaultIds[transId]);
                authClient->queuedVaultIds.erase(transId);
                uuid = ageNode.getUuid(0);
            }
            joinAge(age, uuid);
        } else if (input == "login") {
            loginPlayer();
            setActivePlayer();
            //loadPython()
        } else if (input == "logout") {
            logoutActivePlayer();
            //unloadPython()
        } else {
            cout << "Invalid command. Available commands include: join age, login, logout and quit." << endl;
        }
    }
    logoutActivePlayer();

}

// Sets which player is the active player.
void Omnibot::setActivePlayer() {

    if (authClient == NULL)
        return;

    // Load the list of players that can be used.
    map<uint32_t, plString> playerList;
    for (authPlayer player : authClient->players)
        playerList[player.ID] = player.Name;

    // Get the user's choice.
    string ki;
    while (!Contains(playerList, Server.User.KI)) {
        cout << "Choose a player:" << endl;
        for (pair<uint32_t, plString> player : playerList)
            cout << " * " << player.first << " - " << player.second << endl;
        ki = readline("ID: ");
        Server.User.KI = atoi(ki.c_str());
    }

    // Set the selected player as the active player.
    if (gameClient != NULL)
        logoutActivePlayer();
    activePlayer = Server.User.KI;
    authClient->sendAcctSetPlayerRequest(Server.User.KI);
    cout << "Using player " << Server.User.KI << "." << endl;

}

// Sets the NTD encryption keys to be used.
void Omnibot::setEncryptionKeys(uint32_t k0, uint32_t k1, uint32_t k2, uint32_t k3) {

    ntdKeys[0] = k0;
    ntdKeys[1] = k1;
    ntdKeys[2] = k2;
    ntdKeys[3] = k3;

}

// Sets the specified player as online in the specified Age.
void Omnibot::setOnline(uint32_t playerId, plString ageFilename, plUuid ageUuid) {

    Vault& vault = Vault::get();
    VaultPlayerInfoNode playerNode = vault.getPlayerVault();
    for (VaultNode node : playerNode.getChildren()) {
        if (node.getNodeType() == plVault::kNodePlayerInfo) {
            node.setInt32(0, 1);
            node.setUuid(0, ageUuid);
            node.setString64(0, ageFilename);
            authClient->sendVaultNodeSave(node.getNodeIdx(), plUuid(), node);
            break;
        }
    }

}

// Starts the Game Client for the new Age.
void Omnibot::startGameClient(uint32_t serverAddr, plUuid ageId, uint32_t mcpId, uint32_t ageVaultId) {

    Vault& vault = Vault::get();
    currentAgeId = ageVaultId;
    authClient->sendVaultNodeFetch(currentAgeId);
    VaultPlayerInfoNode player = vault.getPlayerVault();
    if (gameClient != NULL)
        delete gameClient;
    gameClient = new GameClient(this);
    gameClient->setPlayer(player);
    gameClient->setJoinInfo(player.getUuid(0), ageId);
    gameClient->joinAge(serverAddr, mcpId);
    checkCurrentAge();

}

// Keeps sending pings until the thread is terminated.
void sendPing(AuthClient** authClient, GameClient** gameClient) {

    time_t savedTime = time(NULL);
    while (true) {
        while (time(NULL) - savedTime < 20 && *authClient != NULL);
        if (*authClient == NULL)
            break;
        (*authClient)->sendPingRequest(1000);
        if (*gameClient != NULL)
            (*gameClient)->sendPingRequest(1000);
        savedTime = time(NULL);
        while (time(NULL) - savedTime < 1 && *authClient != NULL);
    }

}

// Starts the ping loop for to keep the connection alive.
void Omnibot::startPingLoop() {

    stopPingLoop();
    pingThread = new thread(sendPing, &authClient, &gameClient);

}

// Stops the ping loop.
void Omnibot::stopPingLoop() {

    if (pingThread == NULL)
        return;
    pingThread->join();
    delete pingThread;
    pingThread = NULL;

}
