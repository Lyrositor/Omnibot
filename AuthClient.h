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

#ifndef AUTHCLIENT_H
#define AUTHCLIENT_H

#include <map>
#include <string>
#include <vector>

#include <auth/pnAuthClient.h>
#include <Stream/hsRAMStream.h>

using namespace std;

class Omnibot;
class pnVaultNode;
class pnVaultNodeRef;

struct authPlayer {

    uint32_t ID;
    plString Name;
    plString avatar;

};

struct ageReply {

    plUuid ageInstanceId;
    uint32_t ageVaultId;
    uint32_t gameServerAddress;
    uint32_t mcpId;

};

class AuthClient : public pnAuthClient {

public:
    AuthClient(Omnibot* omnibot);

    plUuid acctUuid;
    int loggingIn;
    vector<authPlayer> players;
    map<uint32_t, ageReply> queuedAgeReplies;
    map<uint32_t, vector<pnVaultNodeRef>> queuedVaultNodeRefs;
    map<uint32_t, pnVaultNode> queuedVaultNodes;
    map<uint32_t, vector<uint32_t>> queuedVaultIdLists;
    map<uint32_t, uint32_t> queuedVaultIds;

    void startLogin(string user, string pass);
    void onClientRegisterReply(uint32_t serverChallenge);
    void onAcctLoginReply(uint32_t transId, ENetError result, const plUuid& acctUuid, uint32_t acctFlags, uint32_t billingType, const uint32_t* encryptKey);
    void onAcctPlayerInfo(uint32_t transId, uint32_t playerId, const plString &playerName, const plString& avatarModel, uint32_t explorer);
    void onAcctSetPlayerReply(uint32_t transId, ENetError result);
    void onAgeReply(uint32_t transId, ENetError result, uint32_t mcpId, const plUuid &ageInstanceId, uint32_t ageVaultId, uint32_t gameServerAddress);
    void onVaultInitAgeReply(uint32_t transId, ENetError result, uint32_t ageId, uint32_t ageInfoId);
    void onPublicAgeList(uint32_t transId, ENetError result, size_t count, const pnNetAgeInfo* ages);
    void onPingReply(uint32_t transId, uint32_t pingTimeMs) { };

    void onFileListReply(uint32_t transId, ENetError, size_t count, const pnAuthFileItem* files);
    void onFileDownloadChunk(uint32_t transId, ENetError result, uint32_t totalSize, uint32_t chunkOffset, size_t chunkSize, const unsigned char* chunkData);

    void onVaultNodeRefsFetched(uint32_t transId, ENetError result, size_t count, const pnVaultNodeRef* refs);
    void onVaultNodeFetched(uint32_t transId, ENetError result, const pnVaultNode& node);
    void onVaultNodeChanged(uint32_t nodeId, const plUuid& revisionId);
    void onVaultNodeCreated(uint32_t transId, ENetError result, uint32_t nodeId);
    void onVaultNodeFindReply(uint32_t transId, ENetError result, size_t count, const uint32_t *nodes);
    void onVaultSaveNodeReply(uint32_t transId, ENetError result);

private:
    Omnibot* parent;
    plString user;
    plString pass;

    map<uint32_t, hsStream*> sdlFiles;

};

#endif // AUTHCLIENT_H
