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
#include <utility>

#include "Vault.h"
#include "Omnibot.h"
#include "Utils.h"

////////////////
// VAULT NODE //
////////////////

// Adds a reference to a child node.
void VaultNode::addChild(uint32_t id) {

    Vault& vault = Vault::get();
    pnVaultNodeRef templateRef;
    templateRef.fParent = fNodeIdx;
    templateRef.fChild = id;
    templateRef.fOwner = vault.omnibot->activePlayer;
    vault.addNodeRef(templateRef);

}

// Returns a list of child nodes.
vector<VaultNode> VaultNode::getChildren() {

    Vault& vault = Vault::get();
    vector<pnVaultNodeRef> refs = vault.findNodeRefs(fNodeIdx);
    vector<uint32_t> queuedIds;
    for (pnVaultNodeRef ref : refs) {
        queuedIds.push_back(vault.omnibot->authClient->sendVaultNodeFetch(ref.fChild));
    }
    vector<VaultNode> nodes;
    for (uint32_t transId : queuedIds) {
        while (!Contains(vault.omnibot->authClient->queuedVaultNodes, transId));
        VaultNode node(vault.omnibot->authClient->queuedVaultNodes[transId]);
        nodes.push_back(node);
        vault.omnibot->authClient->queuedVaultNodes.erase(transId);
    }
    return nodes;

}

// Removes a reference to a child node.
void VaultNode::removeChild(uint32_t id) {

    Vault& vault = Vault::get();
    pnVaultNodeRef templateRef;
    templateRef.fParent = fNodeIdx;
    templateRef.fChild = id;
    vault.removeNodeRef(templateRef);

}

// Saves the changes made to this node to the Vault.
void VaultNode::save() {

    Vault& vault = Vault::get();
    vault.save(*this);

}

// Sends the current node to another player.
void VaultNode::sendTo(uint32_t playerId) {

    Vault& vault = Vault::get();
    vault.sendNode(fNodeIdx, playerId);

}

// Upcasts this node to a PlayerInfo node.
VaultPlayerInfoNode VaultNode::upcastToPlayerInfoNode() {

    return VaultPlayerInfoNode(*this);

}

//////////////////////////////
// VAULT NODE - PLAYER INFO //
//////////////////////////////

///////////
// VAULT //
///////////

// Adds a new node to the Vault using a template node.
VaultNode Vault::addNode(VaultNode& templateNode) {

    uint32_t transId = omnibot->authClient->sendVaultNodeCreate(const_cast<const VaultNode&> (templateNode));
    while (!Contains(omnibot->authClient->queuedVaultIds, transId));
    uint32_t nodeId = omnibot->authClient->queuedVaultIds[transId];
    omnibot->authClient->queuedVaultIds.erase(transId);
    return findNode(nodeId);

}

// Adds a node reference using a template node reference.
void Vault::addNodeRef(pnVaultNodeRef& templateRef) {

    omnibot->authClient->sendVaultNodeAdd(templateRef.fParent, templateRef.fChild, templateRef.fOwner);

}

// Fetches the specified node according to its ID.
VaultNode Vault::findNode(uint32_t id) {

    uint32_t transId = omnibot->authClient->sendVaultNodeFetch(id);
    while (!Contains(omnibot->authClient->queuedVaultNodes, transId));
    VaultNode node(omnibot->authClient->queuedVaultNodes[transId]);
    omnibot->authClient->queuedVaultNodes.erase(transId);
    return node;

}

// Fetches an array of node references attached to a node, specified by its ID.
vector<pnVaultNodeRef> Vault::findNodeRefs(uint32_t id) {

    uint32_t transId = omnibot->authClient->sendVaultFetchNodeRefs(id);
    while (!Contains(omnibot->authClient->queuedVaultNodeRefs, transId));
    vector<pnVaultNodeRef> refs = omnibot->authClient->queuedVaultNodeRefs[transId];
    omnibot->authClient->queuedVaultNodeRefs.erase(transId);
    return refs;

}

// Gets the current player's Vault.
VaultPlayerInfoNode Vault::getPlayerVault() {

    VaultNode node = findNode(omnibot->activePlayer);
    VaultPlayerInfoNode playerNode = node.upcastToPlayerInfoNode();
    return playerNode;

}

// Finds an array of nodes matching the specified template.
vector<uint32_t> Vault::listNodes(VaultNode& templateNode) {

    uint32_t transId = omnibot->authClient->sendVaultNodeFind(const_cast<const VaultNode&> (templateNode));
    while (!Contains(omnibot->authClient->queuedVaultIdLists, transId));
    vector<uint32_t> nodes = omnibot->authClient->queuedVaultIdLists[transId];
    omnibot->authClient->queuedVaultIdLists.erase(transId);
    return nodes;

}

// Removes a node reference using a template node reference.
void Vault::removeNodeRef(pnVaultNodeRef& templateRef) {

    omnibot->authClient->sendVaultNodeRemove(templateRef.fParent, templateRef.fChild);

}

// Saves the specified node to the Vault using the specified template.
void Vault::save(VaultNode& templateNode) {

    omnibot->authClient->sendVaultNodeSave(templateNode.getNodeIdx(), plUuid(), const_cast<const VaultNode&> (templateNode));

}

// Sends a node to another player.
void Vault::sendNode(uint32_t nodeId, uint32_t playerId) {

    omnibot->authClient->sendVaultSendNode(nodeId, playerId);

}
