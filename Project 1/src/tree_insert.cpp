#include <iostream>
#include <algorithm>
#include "dtypes.h"
#include "tree.h"

using namespace std;

void Tree::insert(int key, Record *pRecord) {  //in Leaf Node
    /*
     * Inserts a key-pointer pair into the B+ tree index.
     */

    // search the tree for the key
    vector<Record *> *result = search(key, false);

    // if the key exists, simply add the currentNode Record pointer to the existing vector and return
    if (result != nullptr) {
        (*result).push_back(pRecord);
        return;
    }

    // check if the B+ tree is empty
    if (rootNode == nullptr) {
        // the tree is empty, create new rootNode and store the first Record pointer
        rootNode = new Node;
        rootNode->isLeafNode = true;
        rootNode->keys.push_back(key);
        new(&rootNode->pointer.pData) vector<vector<Record *>>;
        rootNode->pointer.pData.push_back(vector<Record *>{pRecord});
        return;
    } else {
        // the tree is not empty, need to look for a slot for the currentNode record
        Node *currentNode = rootNode;
        Node *parentNode = nullptr;

        // traverse until the leaf node
        while (!currentNode->isLeafNode) {
            parentNode = currentNode;
            int idx = upper_bound(currentNode->keys.begin(), currentNode->keys.end(), key) - currentNode->keys.begin();
            currentNode = currentNode->pointer.pNode[idx];
        }

        // check if the currentNode node at the currentNode has space for another key-pointer pair
        if (currentNode->keys.size() < n) {
            // the currentNode node is not full, so we have to find the correct position to insert it
            int idx = upper_bound(currentNode->keys.begin(), currentNode->keys.end(), key) - currentNode->keys.begin();

            // temporarily append the key-pointer pair to the vectors to expand its size
            currentNode->keys.push_back(key);
            currentNode->pointer.pData.push_back(vector<Record *>{pRecord});

            if (idx != currentNode->keys.size() - 1) {
                // shift the existing records
                for (auto j = currentNode->keys.size() - 1;
                     j > idx; j--) {  // shifting the position for keys and datapointer
                    currentNode->keys[j] = currentNode->keys[j - 1];
                    currentNode->pointer.pData[j] = currentNode->pointer.pData[j - 1];
                }

                // finally, insert the key-pointer pair into the node
                currentNode->keys[idx] = key;
                currentNode->pointer.pData[idx] = vector<Record *>{pRecord};
            }
        } else {
            // the currentNode node is full, we have to split the node
            vector<int> virtualNode(currentNode->keys);
            vector<vector<Record *>> virtualDataNode(currentNode->pointer.pData);

            // find the correct position to insert it
            int i = upper_bound(currentNode->keys.begin(), currentNode->keys.end(), key) - currentNode->keys.begin();

            // temporarily append the key-pointer pair to the vectors to expand its size
            virtualNode.push_back(key);
            virtualDataNode.push_back(vector<Record *>{pRecord});

            if (i != virtualNode.size() - 1) {
                // shift the existing records
                for (auto j = virtualNode.size() - 1; j > i; j--) {
                    virtualNode[j] = virtualNode[j - 1];
                    virtualDataNode[j] = virtualDataNode[j - 1];
                }

                // finally, insert the key-pointer pair into the node
                virtualNode[i] = key;
                virtualDataNode[i] = vector<Record *>{pRecord};
            }

            // create new leaf node
            Node *newLeafNode = new Node;
            newLeafNode->isLeafNode = true;
            new(&newLeafNode->pointer.pData) vector<Record *>;

            // swap pNextLeaf pointers
            Node *temp = currentNode->pNextLeaf;
            currentNode->pNextLeaf = newLeafNode;
            newLeafNode->pNextLeaf = temp;

            // resize and copy key-pointer pairs into the old node
            currentNode->keys.resize((n) / 2 + 1);
            currentNode->pointer.pData.resize((n) / 2 + 1);
            for (i = 0; i <= (n) / 2; i++) {
                currentNode->keys[i] = virtualNode[i];
                currentNode->pointer.pData[i] = virtualDataNode[i];
            }

            // copy key-pointer pairs into the newly created node
            for (i = (n) / 2 + 1; i < virtualNode.size(); i++) {
                newLeafNode->keys.push_back(virtualNode[i]);
                newLeafNode->pointer.pData.push_back(virtualDataNode[i]);
            }

            // if currentNode points to rootNode, create a new node
            if (currentNode == rootNode) {
                Node *newRootNode = new Node;
                newRootNode->keys.push_back(newLeafNode->keys[0]);
                new(&newRootNode->pointer.pNode) vector<Node *>;
                newRootNode->pointer.pNode.push_back(currentNode);
                newRootNode->pointer.pNode.push_back(newLeafNode);
                rootNode = newRootNode;
            } else {
                // insert new key into the parentNode
                insertInternal(newLeafNode->keys[0], &parentNode, &newLeafNode);
            }
        }
    }
}

void Tree::insertInternal(int x, Node **currentNode, Node **child) {
    /*
     * Inserts a key into an internal node
     */

    // check if the currentNode node is full
    if ((*currentNode)->keys.size() < maxInternalChild - 1) {
        // the currentNode node is not full, so we have to find the correct position to insert it
        int idx = upper_bound((*currentNode)->keys.begin(), (*currentNode)->keys.end(), x) - (*currentNode)->keys.begin();

        // temporarily append the key-pointer pair to the vectors to expand its size
        (*currentNode)->keys.push_back(x);
        (*currentNode)->pointer.pNode.push_back(*child);

        // shift the existing key-pointer pairs
        if (idx != (*currentNode)->keys.size() - 1) {
            for (auto j = (*currentNode)->keys.size() - 1; j > idx; j--) {
                (*currentNode)->keys[j] = (*currentNode)->keys[j - 1];
            }

            for (auto j = (*currentNode)->pointer.pNode.size() - 1; j > (idx + 1); j--) {
                (*currentNode)->pointer.pNode[j] = (*currentNode)->pointer.pNode[j - 1];
            }

            // finally, insert the key-pointer pair into the node
            (*currentNode)->keys[idx] = x;
            (*currentNode)->pointer.pNode[idx + 1] = *child;
        }
    } else {  //splitting
        // the currentNode node is full, we have to split the node
        vector<int> virtualKeyNode((*currentNode)->keys);
        vector<Node *> virtualTreePNode((*currentNode)->pointer.pNode);

        // find the correct position to insert it
        int idx = upper_bound((*currentNode)->keys.begin(), (*currentNode)->keys.end(), x) -
                  (*currentNode)->keys.begin();

        // temporarily append the key-pointer pair to the vectors to expand its size
        virtualKeyNode.push_back(x);
        virtualTreePNode.push_back(*child);

        // shift the existing key-pointer pairs
        if (idx != virtualKeyNode.size() - 1) {
            for (auto j = virtualKeyNode.size() - 1; j > idx; j--) {
                virtualKeyNode[j] = virtualKeyNode[j - 1];
            }

            for (auto j = virtualTreePNode.size() - 1; j > (idx + 1); j--) {
                virtualTreePNode[j] = virtualTreePNode[j - 1];
            }

            // finally, insert the key-pointer pair into the node
            virtualKeyNode[idx] = x;
            virtualTreePNode[idx + 1] = *child;
        }

        int partitionKey;  // middle element excluded
        partitionKey = virtualKeyNode[(virtualKeyNode.size() / 2)];  // split is right-biased
        auto partitionIdx = (virtualKeyNode.size() / 2);

        // resize and copy key-pointer pairs into the old node
        (*currentNode)->keys.resize(partitionIdx);
        (*currentNode)->pointer.pNode.resize(partitionIdx + 1);
        (*currentNode)->pointer.pNode.resize(partitionIdx + 1);
        for (int i = 0; i < partitionIdx; i++) {
            (*currentNode)->keys[i] = virtualKeyNode[i];
        }

        for (int i = 0; i < partitionIdx + 1; i++) {
            (*currentNode)->pointer.pNode[i] = virtualTreePNode[i];
        }

        Node *newInternalNode = new Node;
        new(&newInternalNode->pointer.pNode) std::vector<Node *>;

        // copy key-pointer pairs into the newly created node
        for (auto i = partitionIdx + 1; i < virtualKeyNode.size(); i++) {
            newInternalNode->keys.push_back(virtualKeyNode[i]);
        }

        for (auto i = partitionIdx + 1;
             i < virtualTreePNode.size(); i++) {  // because only key is excluded not the pointer
            newInternalNode->pointer.pNode.push_back(virtualTreePNode[i]);
        }

        // if currentNode points to rootNode, create a new node
        if ((*currentNode) == rootNode) {
            Node *newRootNode = new Node;
            newRootNode->keys.push_back(partitionKey);
            new(&newRootNode->pointer.pNode) std::vector<Node *>;
            newRootNode->pointer.pNode.push_back(*currentNode);
            newRootNode->pointer.pNode.push_back(newInternalNode);
            rootNode = newRootNode;
        } else {
            insertInternal(partitionKey, findParentNode(rootNode, *currentNode), &newInternalNode);
        }
    }
}

