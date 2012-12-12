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

#include <algorithm>
#include <iostream>
#include <utility>

#include "AuthClient.h"
#include "Omnibot.h"
#include "Utils.h"

// Calls the constructor for pnAuthClient and initializes it.
AuthClient::AuthClient(Omnibot* omnibot) : pnAuthClient(omnibot->getResManager()), parent(omnibot) {

    setKeys(parent->Server.Auth.X, parent->Server.Auth.N);
    setClientInfo(BUILD_ID, 50, 1, s_moulUuid);
    loggingIn = 0;

}

// Called when the account login has been completed.
void AuthClient::onAcctLoginReply(uint32_t, ENetError result, const plUuid& acctUuid, uint32_t, uint32_t, const uint32_t* encryptKey) {

    if (result != kNetSuccess) {
        cout << "ERROR: Authentication failed." << endl;
        loggingIn = 2;
        return;
    }

    cout << "Successfully authenticated." << endl;
    loggingIn = 1;
    this->acctUuid = acctUuid;
    this->sendFileListRequest("SDL", "sdl");

    parent->setEncryptionKeys(encryptKey[0], encryptKey[1], encryptKey[2], encryptKey[3]);
    parent->startPingLoop();

}


// Called when the list of avatars is fetched.
void AuthClient::onAcctPlayerInfo(uint32_t, uint32_t playerId, const plString& playerName, const plString& avatarModel, uint32_t) {

    authPlayer player;
    player.ID = playerId;
    player.Name = playerName;
    player.avatar = avatarModel;
    players.push_back(player);

}

// Called when the active player has been set.
void AuthClient::onAcctSetPlayerReply(uint32_t transId, ENetError result) {

    if (result != kNetSuccess) {
        cout << "ERROR: Failed to set account player: " << GetNetErrorString(result) << endl;
        return;
    }

}

// Called after an Age request has been completed.
void AuthClient::onAgeReply(uint32_t transId, ENetError result, uint32_t mcpId, const plUuid &ageInstanceId, uint32_t ageVaultId, uint32_t gameServerAddress) {

    if (result == kNetSuccess) {
        queuedAgeReplies[transId] = {ageInstanceId, ageVaultId, gameServerAddress, mcpId};
    } else {
        cout << "ERROR: Age request failed: " << GetNetErrorString(result) << endl;
    }

}

// Called once the server has registered this client.
void AuthClient::onClientRegisterReply(uint32_t serverChallenge) {

    cout << "Logging in as " << user << "." << endl;
    sendAcctLoginRequest(serverChallenge, rand(), user, pass);

}

// Called when a chunk of a file has finished downloading.
void AuthClient::onFileDownloadChunk(uint32_t transId, ENetError result, uint32_t totalSize, uint32_t chunkOffset, size_t chunkSize, const unsigned char* chunkData) {

    if (result != kNetSuccess) {
        cout << "ERROR: Failed to download file: " << GetNetErrorString(result) << endl;
        return;
    }

    hsStream* S = sdlFiles[transId];
    S->write(chunkSize, chunkData);

    if (chunkOffset + chunkSize == totalSize) {
        S->rewind();
        parent->loadStateDescriptors(S);
    }

}

// Called when a list of files to download has been sent.
void AuthClient::onFileListReply(uint32_t, ENetError, size_t count, const pnAuthFileItem* files) {

    for (size_t i = 0; i < count; i++) {
        uint32_t fileTrans;
        fileTrans = this->sendFileDownloadRequest(files[i].fFilename);
        sdlFiles[fileTrans] = new hsRAMStream(PlasmaVer::pvMoul);
    }

}

// Called when a new Age has been created in the Vault.
void AuthClient::onVaultInitAgeReply(uint32_t transId, ENetError result, uint32_t ageId, uint32_t) {

    if (result != kNetSuccess) {
        cout << "ERROR: Failed to create Age: " << GetNetErrorString(result) << endl;
        return;
    }
    queuedVaultIds[transId] = ageId;

}

// Called when the list of public Ages is received.
void AuthClient::onPublicAgeList(uint32_t, ENetError result, size_t count, const pnNetAgeInfo* ages) {

}

// Called when a Vault node has been changed.
void AuthClient::onVaultNodeChanged(uint32_t nodeId, const plUuid&) {

}

// Called when a Vault node has been created.
void AuthClient::onVaultNodeCreated(uint32_t transId, ENetError result, uint32_t nodeId) {

    if (result != kNetSuccess) {
        cout << "ERROR: Failed to create node: " << GetNetErrorString(result) << endl;
        return;
    }
    queuedVaultIds[transId] = nodeId;

}

// Called when a Vault node has been fetched using its ID.
void AuthClient::onVaultNodeFetched(uint32_t transId, ENetError result, const pnVaultNode& node) {

    if (result != kNetSuccess) {
        cout << "ERROR: Failed to fetch node: " << GetNetErrorString(result) << endl;
        return;
    }
    queuedVaultNodes[transId] = node;

}

// Called when a list of node IDs has been found.
void AuthClient::onVaultNodeFindReply(uint32_t transId, ENetError result, size_t count, const uint32_t* nodes) {

    if (result != kNetSuccess) {
        cout << "ERROR: Failed to find node: " << GetNetErrorString(result) << endl;
        return;
    }
    vector<uint32_t> nodeIds;
    for (int i = 0; i < count; i++)
        nodeIds.push_back(nodes[i]);
    queuedVaultIdLists[transId] = nodeIds;

}

// Called when a list of node refs has been fetched.
void AuthClient::onVaultNodeRefsFetched(uint32_t transId, ENetError, size_t count, const pnVaultNodeRef* refs) {

    vector<pnVaultNodeRef> nodeRefs;
    for (int i = 0; i < count; i++)
        nodeRefs.push_back(refs[i]);
    queuedVaultNodeRefs[transId] = nodeRefs;

}

// Called when a Vault node has been saved to the Vault.
void AuthClient::onVaultSaveNodeReply(uint32_t transId, ENetError result) {

}

// Begin the login process by registering the client to the server.
void AuthClient::startLogin(string user, string pass) {

    // Apparently HSPlasma still doesn't lowercase the username.
    transform(user.begin(), user.end(), user.begin(), ::tolower);
    this->user = plString(const_cast <char *>(user.c_str()));
    this->pass = plString(const_cast <char *>(pass.c_str()));
    if (pnAuthClient::connect(parent->Server.Auth.Host.c_str()) != kNetSuccess) {
        cout << "ERROR: Could not connect to server." << endl;
        loggingIn = 2;
        return;
    }
    sendClientRegisterRequest();

}
