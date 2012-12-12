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

#ifndef VAULT_H
#define VAULT_H

#include <mutex>
#include <vector>

#include <auth/pnVaultNode.h>

using namespace std;

class Omnibot;
class VaultAgeInfoListNode;
class VaultAgeInfoNode;
class VaultAgeLinkNode;
class VaultChronicleNode;
class VaultFolderNode;
class VaultImageNode;
class VaultMarkerGameNode;
class VaultPlayerInfoListNode;
class VaultPlayerInfoNode;
class VaultSDLNode;
class VaultPlayerNode;
class VaultSystemNode;
class VaultTextNoteNode;

// Vault nodes.
class VaultNode : public pnVaultNode {

public:
    VaultNode() : pnVaultNode() { }
    VaultNode(pnVaultNode node) : pnVaultNode(node) { }
    bool operator==(uint32_t id) { return (fNodeIdx == id); }
    bool operator!=(uint32_t id) { return (fNodeIdx != id); }

    void addChild(uint32_t id);
    vector<VaultNode> getChildren();
    void removeChild(uint32_t id);
    void save();
    void sendTo(uint32_t playerId);

    VaultAgeInfoListNode upcastToAgeInfoListNode();
    VaultAgeInfoNode upcastToAgeInfoNode();
    VaultAgeLinkNode upcastToAgeLinkNode();
    VaultChronicleNode upcastToChronicleNode();
    VaultFolderNode upcastToFolderNode();
    VaultImageNode upcastToImageNode();
    VaultMarkerGameNode upcastToMarkerGameNode();
    VaultPlayerInfoListNode upcastToPlayerInfoListNode();
    VaultPlayerInfoNode upcastToPlayerInfoNode();
    VaultPlayerNode upcastToPlayerNode();
    VaultSDLNode upcastToSDLNode();
    VaultSystemNode upcastToSystemNode();
    VaultTextNoteNode upcastToTextNoteNode();

private:
    vector<VaultNode> children;

};

class VaultAgeInfoListNode : public VaultNode {

};

class VaultAgeInfoNode : public VaultNode {

};

class VaultAgeLinkNode : public VaultNode {

};

class VaultChronicleNode : public VaultNode {

};

class VaultFolderNode : public VaultNode {

};

class VaultImageNode : public VaultNode {

};

class VaultMarkerGameNode : public VaultNode {

};

class VaultPlayerInfoListNode : public VaultNode {

};

class VaultPlayerInfoNode : public VaultNode {

public:
    VaultPlayerInfoNode() : VaultNode() { }
    VaultPlayerInfoNode(VaultNode node) : VaultNode(node) { }

};

class VaultPlayerNode : public VaultNode {

};

class VaultSDLNode : public VaultNode {

};

class VaultSystemNode : public VaultNode {

};

class VaultTextNoteNode : public VaultNode {

};

// The Vault manager.
class Vault {

public:
    ~Vault() {}
    static Vault& get(Omnibot* omnibot = NULL) {
        static Vault instance;
        if (omnibot != NULL)
            instance.omnibot = omnibot;
        return instance;
    };

    // Vault operations.
    VaultNode addNode(VaultNode& templateNode);
    void addNodeRef(pnVaultNodeRef& ref);
    VaultNode findNode(uint32_t id);
    vector<pnVaultNodeRef> findNodeRefs(uint32_t id);
    vector<uint32_t> listNodes(VaultNode& templateNode);
    void save(VaultNode& templateNode);
    void sendNode(uint32_t nodeId, uint32_t playerId);
    void removeNodeRef(pnVaultNodeRef& templateRef);

    // Local player's Vault.
    VaultPlayerInfoNode getPlayerVault();

    Omnibot* omnibot;

private:
    // Make sure Vault is a singleton.
    Vault() {}
    Vault(Vault const&);
    void operator=(Vault const&);

};

#endif // VAULT_H
