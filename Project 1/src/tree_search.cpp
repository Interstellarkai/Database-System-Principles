#include <iostream>
#include <algorithm>
#include <vector>
#include "dtypes.h"
#include "tree.h"

using namespace std;

vector<Record *> *Tree::search(int key, bool printNode) {
    /*
     * Searches the B+ tree for a key and returns the corresponding pointer to a vector of Record pointers.
     * If the key is not found in the tree, a nullptr is returned.
     */

    // check if the B+ tree is empty
    if (rootNode == nullptr) {
        return nullptr;
    } else {
        // start the currentNode at the rootNode
        Node *currentNode = rootNode;

        // traverse to the leaf node
        while (!currentNode->isLeafNode) {
            int idx = upper_bound(currentNode->keys.begin(), currentNode->keys.end(), key) - currentNode->keys.begin();

            // count accesses for intermediate internal nodes
            nodesAccessedNum++;

            // print intermediate internal nodes
            if (printNode) {
                displayCurrentNode(currentNode);
            }

            currentNode = currentNode->pointer.pNode[idx];
        }

        // print the leaf node
        if (printNode) {
            displayCurrentNode(currentNode);
        }

        // count the access for the leaf node
        nodesAccessedNum++;

        // binary search of the keys in the leaf node
        int idx = lower_bound(currentNode->keys.begin(), currentNode->keys.end(), key) - currentNode->keys.begin();

        // key not found
        if (idx == currentNode->keys.size() || currentNode->keys[idx] != key) {
            return nullptr;
        }

        // return the corresponding pointer of the key
        return &currentNode->pointer.pData[idx];
    }
}

Node *Tree::searchNode(int key, bool printNode) {
    /*
     * Searches the B+ tree for a key and returns the corresponding leaf node that the key resides in.
     * If the key is not found in the tree, a nullptr is returned.
     */

    // check if the B+ tree is empty
    if (rootNode == nullptr) {
        return nullptr;
    } else {
        // start the currentNode at the rootNode
        Node *currentNode = rootNode;

        // traverse to the leaf node
        while (!currentNode->isLeafNode) {
            int idx = upper_bound(currentNode->keys.begin(), currentNode->keys.end(), key) - currentNode->keys.begin();

            // count accesses for intermediate internal nodes
            nodesAccessedNum++;

            // print intermediate internal nodes
            if (printNode) {
                displayCurrentNode(currentNode);
            }

            currentNode = currentNode->pointer.pNode[idx];
        }

        // count the access for the leaf node
        nodesAccessedNum++;

        // return the leaf node
        return currentNode;
    }
}
