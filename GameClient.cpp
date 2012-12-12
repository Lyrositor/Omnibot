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

#include <iostream>
#include <string>

#include <Stream/hsRAMStream.h>
#include <PRP/NetMessage/plNetMsgLoadClone.h>
#include <PRP/NetMessage/plNetMsgMembersList.h>
#include <PRP/Message/plLoadAvatarMsg.h>
#include <PRP/Message/pfKIMsg.h>

#include "GameClient.h"
#include "Omnibot.h"

// Calls the constructor for pnGameClient and initializes it.
GameClient::GameClient(Omnibot* omnibot) : pnGameClient(omnibot->getResManager()), fPlayerNode(), fAgeInfoNode(), parent(omnibot) {

    setKeys(omnibot->Server.Game.X, omnibot->Server.Game.N);
    setClientInfo(BUILD_ID, 50, 1, s_moulUuid);

}

// Send a request to join the specified Age.
void GameClient::joinAge(uint32_t serverAddr, uint32_t mcpId) {

    plString serverString = plString::Format("%u.%u.%u.%u",
                                             (serverAddr & 0xFF000000) >> 24,
                                             (serverAddr & 0x00FF0000) >> 16,
                                             (serverAddr & 0x0000FF00) >> 8,
                                             (serverAddr & 0x000000FF) >> 0
                                             );
    uint32_t result;
    if ((result = pnGameClient::connect(serverString.cstr())) != kNetSuccess) {
        cout << "ERROR: Could not connect to Game Server: " << GetNetErrorString(result) << endl;
        return;
    }
    fMcpId = mcpId;
    sendJoinAgeRequest(mcpId, fAccountId, fPlayerNode.getNodeIdx());

}

// Set up the avatar once an Age has been joined.
void GameClient::onJoinAgeReply(uint32_t, ENetError result) {

    if (result != kNetSuccess) {
        cout << "ERROR: Failed to join Age: " << GetNetErrorString(result) << endl;
        return;
    }
    plKeyData* clientMgr = new plKeyData();
    clientMgr->setName("kNetClientMgr_KEY");
    clientMgr->setID(0);
    clientMgr->setType(0x0052); // plNetClientMger
    plLocation clientMgrLoc(PlasmaVer::pvMoul);
    clientMgrLoc.setVirtual();
    clientMgrLoc.setFlags(0x0000);
    clientMgr->setLocation(clientMgrLoc);
    plKeyData* playerKey = new plKeyData();
    if (true) {
        playerKey->setName("Male");
        playerKey->setID(78);
        plLocation playerKeyLoc(PlasmaVer::pvMoul);
        playerKeyLoc.setPageNum(1);
        playerKeyLoc.setSeqPrefix(-6);
        playerKeyLoc.setFlags(0x0004);
        playerKey->setLocation(playerKeyLoc);
    }
    playerKey->setType(0x0001); // plSceneObject
    playerKey->setCloneIDs(2, fPlayerNode.getNodeIdx()); // Not sure what the 2 signifies.
    plKeyData* avMgr = new plKeyData();
    avMgr->setName("kAvatarMgr_KEY");
    plLocation avMgrLoc(PlasmaVer::pvMoul);
    avMgrLoc.setVirtual();
    avMgrLoc.setFlags(0x0000);
    avMgr->setLocation(avMgrLoc);
    avMgr->setType(0x00F4); //plAvatarMgr
    avMgr->setID(0);
    plLoadAvatarMsg* loadAvMsg = new plLoadAvatarMsg();
    loadAvMsg->addReceiver(plKey(clientMgr));
    loadAvMsg->setBCastFlags(0x00000840);
    loadAvMsg->setCloneKey(plKey(playerKey));
    loadAvMsg->setRequestor(plKey(avMgr));
    loadAvMsg->setOriginatingPlayerID(fPlayerNode.getNodeIdx());
    loadAvMsg->setUserData(0);
    loadAvMsg->setValidMsg(1);
    loadAvMsg->setIsLoading(1);
    loadAvMsg->setIsPlayer(1);
    plNetMsgLoadClone loadClone;
    loadClone.setFlags(0x00041001);
    loadClone.setTimeSent(plUnifiedTime::GetCurrentTime());
    loadClone.setPlayerID(fPlayerNode.getNodeIdx());
    loadClone.setMessage(loadAvMsg);
    loadClone.setIsPlayer(1);
    loadClone.setIsLoading(1);
    loadClone.setIsInitialState(0);
    loadClone.setCompressionType(0);
    loadClone.setObject(playerKey->getUoid());
    propagateMessage(&loadClone);
    plNetMsgMembersListReq listReq;
    listReq.setFlags(0x00061001);
    listReq.setTimeSent(plUnifiedTime::GetCurrentTime());
    listReq.setPlayerID(fPlayerNode.getNodeIdx());
    propagateMessage(&listReq);

}

// Called by the server when it sends a reply to a ping.
void GameClient::onPingReply(uint32_t pingTimeMs) {

}

// Called once a chat message has been sent by someone.
void GameClient::onPropagateMessage(plCreatable *msg) {

    hsRAMStream S(PlasmaVer::pvMoul);
    pfPrcHelper prc(&S);
    msg->prcWrite(&prc);
    char* data = new char[S.size() + 1];
    S.copyTo(data, S.size());
    data[S.size()] = 0;
    delete[] data;
    if (msg->ClassIndex() == kNetMsgGameMessageDirected) {
        plMessage* gameMsg = ((plNetMsgGameMessageDirected*)msg)->getMessage();
        if (gameMsg->ClassIndex() == kKIMsg) {
            pfKIMsg* kiMsg = (pfKIMsg*)gameMsg;
            string user = kiMsg->getUser().cstr();
            string message = kiMsg->getString().cstr();
            if (kiMsg->getFlags() & pfKIMsg::kStatusMsg) { // 0x10 (/me action)
                //emit receivedGameMsg(message + "\n");
            } else if (kiMsg->getFlags() & pfKIMsg::kUNUSED1) { // Sender is out-of-age (and sending us a location).
                int splitIndex = int(message.find(">>"));
                //emit receivedGameMsg("From " + user + " in " + message.mid(2, splitIndex - 2) + ": " + message.mid(splitIndex + 2) + "\n");
            } else if (kiMsg->getFlags() & (pfKIMsg::kPrivateMsg | pfKIMsg::kNeighborMsg)) {
                //emit receivedGameMsg("From " + user + ": " + message + "\n");
            } else { // Anything else.
                //emit receivedGameMsg(user + ": " + message + "\n");
            }
        }
    } else if (msg->ClassIndex() == kNetMsgMembersList) {
        fAgePlayers.clear();
        plNetMsgMembersList* membersList = (plNetMsgMembersList*)msg;
        for (unsigned int i = 0; i < membersList->getMembers().getSize(); i++) {
            const plNetMsgMemberInfoHelper* info = &membersList->getMembers()[i];
            const plClientGuid* guid = &info->getClientGuid();
            fAgePlayers.append(guid->getPlayerID());
        }
    } else if (msg->ClassIndex() == kNetMsgMemberUpdate) {
        plNetMsgMemberUpdate* memberUpdate = (plNetMsgMemberUpdate*)msg;
        const plNetMsgMemberInfoHelper* info = &memberUpdate->getMember();
        const plClientGuid* guid = &info->getClientGuid();
        if (memberUpdate->getAddMember()) {
            // Add Age member.
            fAgePlayers.append(guid->getPlayerID());
            //emit receivedGameMsg(plString::Format("* %s joined the age\n", guid->getPlayerName().cstr()).cstr());
        } else {
            // Remove Age member.
            fAgePlayers.remove(fAgePlayers.find(guid->getPlayerID()));
            //emit receivedGameMsg("* Someone left the age\n");
        }
    }

}

// Set the Game Client to this Age.
void GameClient::setAgeInfo(VaultNode ageInfo) {

    fAgeInfoNode = ageInfo;
    parent->setOnline(fPlayerNode.getNodeIdx(), fAgeInfoNode.getString64(1), fAgeInfoNode.getUuid(0));

}

// Set the Game Client to the active player.
void GameClient::setPlayer(VaultPlayerInfoNode player) {

    fPlayerNode = player;

}

// Sends a message to all players in the current Age.
void GameClient::sendAgeChat(plString message) {

    plNetMsgGameMessageDirected gameMsg;
    pfKIMsg* kiMsg = new pfKIMsg();
    gameMsg.setFlags(0x00049001);
    gameMsg.setTimeSent(plUnifiedTime::GetCurrentTime());
    gameMsg.setPlayerID(fPlayerNode.getNodeIdx());
    gameMsg.setReceivers(fAgePlayers);
    kiMsg->setBCastFlags(0x00000248);
    kiMsg->setCommand(0);
    if (message.startsWith("/me")) {
        kiMsg->setFlags(pfKIMsg::kStatusMsg);
    } else {
        kiMsg->setFlags(0x00); // Standard Age chat.
    }
    kiMsg->setPlayerID(fPlayerNode.getNodeIdx());
    kiMsg->setUser(fPlayerNode.getIString64(0));
    kiMsg->setString(message);
    gameMsg.setMessage(kiMsg);
    propagateMessage(&gameMsg);

}

// Sends a message to the player's buddies.
void GameClient::sendBuddies(plString message, vector<uint32_t> buddies, int type) {

    hsTArray<uint32_t> targets;
    for (uint32_t target: buddies) {
        targets.append(target);
    }
    plNetMsgGameMessageDirected gameMsg;
    pfKIMsg* kiMsg = new pfKIMsg();
    gameMsg.setFlags(0x00049001);
    gameMsg.setTimeSent(plUnifiedTime::GetCurrentTime());
    gameMsg.setPlayerID(fPlayerNode.getNodeIdx());
    gameMsg.setReceivers(targets);
    kiMsg->setBCastFlags(0x00004248);
    kiMsg->setCommand(0);
    if (type == 1) {
        kiMsg->setFlags(pfKIMsg::kUNUSED1); // BUDDIES 0x08
    } else {
        kiMsg->setFlags(pfKIMsg::kNeighborMsg | pfKIMsg::kUNUSED1); // NEIGHBORS 0x28
    }
    kiMsg->setPlayerID(fPlayerNode.getNodeIdx());
    kiMsg->setUser(fPlayerNode.getIString64(0));
    if (fAgeInfoNode.getNodeIdx() != 0) {
        //kiMsg->setString(plString::Format("<<%s>>%s", fAgeInfoNode.displayName().cstr(), message.cstr()));
    } else {
        kiMsg->setString(plString::Format("<<%s>>%s", "???", message.cstr()));
    }
    gameMsg.setMessage(kiMsg);
    propagateMessage(&gameMsg);

}

// Sends a private message to a player.
void GameClient::sendPrivate(plString message, uint32_t target) {

    hsTArray<uint32_t> targets;
    targets.append(target);
    plNetMsgGameMessageDirected gameMsg;
    pfKIMsg* kiMsg = new pfKIMsg();
    gameMsg.setFlags(0x00049001);
    gameMsg.setTimeSent(plUnifiedTime::GetCurrentTime());
    gameMsg.setPlayerID(fPlayerNode.getNodeIdx());
    gameMsg.setReceivers(targets);
    kiMsg->setBCastFlags(0x00004248);
    kiMsg->setCommand(0);
    kiMsg->setFlags(pfKIMsg::kPrivateMsg | pfKIMsg::kUNUSED1); // Private message 0x09
    kiMsg->setPlayerID(fPlayerNode.getNodeIdx());
    kiMsg->setUser(fPlayerNode.getIString64(0));
    if (fAgeInfoNode.getNodeIdx() != 0) {
        //kiMsg->setString(plString::Format("<<%s>>%s", fAgeInfoNode.displayName().cstr(), message.cstr()));
    } else {
        kiMsg->setString(plString::Format("<<%s>>%s", "???", message.cstr()));
    }
    gameMsg.setMessage(kiMsg);
    propagateMessage(&gameMsg);

}
