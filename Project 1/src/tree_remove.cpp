#include <iostream>
#include <cstring>
#include "tree.h"

using namespace std;

void Tree::removeKey(int x) {
    /*
     * Removes a key from the B+ tree.
     */
    Node *rootNode = getRoot();

    // check if the B+ tree is empty
    if (rootNode == nullptr) {
        cout << "Unable to remove: The B+ tree is empty!" << endl;
        return;
    }

    Node *currentNode = rootNode;
    Node *parentNode;
    int parentLeft, parentRight;

    // traverse until the leaf node
    while (!currentNode->isLeafNode) {
        for (int i = 0; i < currentNode->keys.size(); i++) {
            parentNode = currentNode;
            parentLeft = i - 1;  // left side of parentNode
            parentRight = i + 1;  // right side of parentNode

            if (x < currentNode->keys[i]) {
                currentNode = currentNode->pointer.pNode[i];
                break;
            }
            if (i == currentNode->keys.size() - 1) {
                parentLeft = i;
                parentRight = i + 2;
                currentNode = currentNode->pointer.pNode[i + 1];
                break;
            }
        }
    }

    // check if the key exist in the currentNode leaf node
    int pos = 0;
    for (pos = 0; pos < currentNode->keys.size(); pos++) {
        if (currentNode->keys[pos] == x) {
            break;
        }
    }

    auto itr = lower_bound(currentNode->keys.begin(), currentNode->keys.end(), x);

    // if the iterator points to the last key, the key was not found
    if (itr == currentNode->keys.end()) {
        cout << "Unable to remove: The key '" << x << "' was not found in the B+ Tree." << endl;
        return;
    }

    // erase the vector of Record pointers from the position
    currentNode->pointer.pData.erase(currentNode->pointer.pData.begin() + pos);

    // shift key-pointer pairs to fill up the gap
    for (int i = pos; i < currentNode->keys.size() - 1; i++) {
        currentNode->keys[i] = currentNode->keys[i + 1];
        currentNode->pointer.pData[i] = currentNode->pointer.pData[i + 1];
    }

    // resize the vectors
    auto new_size = currentNode->keys.size() - 1;
    currentNode->keys.resize(new_size);
    currentNode->pointer.pData.resize(new_size);

    // if the currentNode is the rootNode and is empty, the tree is empty
    if (currentNode == rootNode && currentNode->keys.empty()) {
        setRoot(nullptr);
    }

    cout << "Removed '" << x << "' from the B+ Tree successfully!" << endl;

    // return if the B+ tree is still balanced
    if (currentNode->keys.size() >= (getN() + 1) / 2) {
        return;
    }

    // attempt to borrow a key from the left sibling if we have a left sibling
    if (parentLeft >= 0) {
        Node *leftNode = parentNode->pointer.pNode[parentLeft];

        // check if left sibling has extra key to lend
        if (leftNode->keys.size() >= (getN() + 1) / 2 + 1) {

            // transfer the largest key from the left Sibling
            auto maxIdx = leftNode->keys.size() - 1;
            currentNode->keys.insert(currentNode->keys.begin(), leftNode->keys[maxIdx]);
            currentNode->pointer.pData.insert(currentNode->pointer.pData.begin(), leftNode->pointer.pData[maxIdx]);

            // resize the left sibling node
            leftNode->keys.resize(maxIdx);
            leftNode->pointer.pData.resize(maxIdx);

            // update the parentNode
            parentNode->keys[parentLeft] = currentNode->keys[0];
            return;
        }
    }

    // attempt to borrow a key from the right sibling, if we have a right sibling
    if (parentRight < parentNode->pointer.pNode.size()) {
        Node *rightNode = parentNode->pointer.pNode[parentRight];

        // check if right sibling has extra key to lend
        if (rightNode->keys.size() >= (getN() + 1) / 2 + 1) {

            // transfer the smallest key from the right Sibling
            int minIdx = 0;
            currentNode->keys.push_back(rightNode->keys[minIdx]);
            currentNode->pointer.pData.push_back(rightNode->pointer.pData[minIdx]);

            // resize the right sibling node
            rightNode->keys.erase(rightNode->keys.begin());
            rightNode->pointer.pData.erase(rightNode->pointer.pData.begin());

            // update the parentNode
            parentNode->keys[parentRight - 1] = rightNode->keys[0];
            return;
        }
    }

    // merge and delete nodes
    // check if we have a left sibling
    if (parentLeft >= 0) {
        Node *leftNode = parentNode->pointer.pNode[parentLeft];

        // merge the two leaf nodes by transferring the key-pointer pairs
        for (int i = 0; i < currentNode->keys.size(); i++) {
            leftNode->keys.push_back(currentNode->keys[i]);
            leftNode->pointer.pData.push_back(currentNode->pointer.pData[i]);
        }
        // update the pointer to the next leaf node
        leftNode->pNextLeaf = currentNode->pNextLeaf;

        // delete the node
        removeInternal(parentNode->keys[parentLeft], parentNode, currentNode);//delete parentNode Node Key

    } else if (parentRight <= parentNode->keys.size()) {
        Node *rightNode = parentNode->pointer.pNode[parentRight];

        // merge the two leaf nodes by transferring the key-pointer pairs
        for (int i = 0; i < rightNode->keys.size(); i++) {
            currentNode->keys.push_back(rightNode->keys[i]);
            currentNode->pointer.pData.push_back(rightNode->pointer.pData[i]);
        }
        // update the pointer to the next leaf node
        currentNode->pNextLeaf = rightNode->pNextLeaf;

        // delete the node
        removeInternal(parentNode->keys[parentRight - 1], parentNode, rightNode);
        delete rightNode;
    }

}

void Tree::removeInternal(int x, Node *currentNode, Node *child) {
    /*
     * Removes key from an internal node
     */
    Node *rootNode = getRoot();

    // check if the key to be deleted is in the rootNode
    if (currentNode == rootNode) {
        if (currentNode->keys.size() == 1) {
            // if only one key is left in the rootNode and matches the child, set child as the rootNode
            if (currentNode->pointer.pNode[1] == child) {
                setRoot(currentNode->pointer.pNode[0]);
                delete currentNode;
                return;
            } else if (currentNode->pointer.pNode[0] == child) {
                setRoot(currentNode->pointer.pNode[1]);
                delete currentNode;
                return;
            }
        }
    }

    // find position of key
    int pos;
    for (pos = 0; pos < currentNode->keys.size(); pos++) {
        if (currentNode->keys[pos] == x) {
            break;
        }
    }

    // shift the keys
    for (int i = pos; i < currentNode->keys.size() - 1; i++) {
        currentNode->keys[i] = currentNode->keys[i + 1];
    }

    // reduce the size of the keys vector, effectively deleting the last element
    currentNode->keys.resize(currentNode->keys.size() - 1);

    // find position of the node
    for (pos = 0; pos < currentNode->pointer.pNode.size(); pos++) {
        if (currentNode->pointer.pNode[pos] == child) {
            break;
        }
    }

    // shift the node pointers
    for (int i = pos; i < currentNode->pointer.pNode.size() - 1; i++) {
        currentNode->pointer.pNode[i] = currentNode->pointer.pNode[i + 1];
    }

    // reduce the size of the keys vector, effectively deleting the last element
    currentNode->pointer.pNode.resize(currentNode->pointer.pNode.size() - 1);

    // return if the B+ tree is still balanced
    if (currentNode->keys.size() >= (getMaxInternalChild() + 1) / 2 - 1) {
        return;
    }

    if (currentNode == rootNode) {
        return;
    }

    Node **p1 = findParentNode(rootNode, currentNode);
    Node *parentNode = *p1;

    int parentLeft, parentRight;

    // find the left and right siblings
    for (pos = 0; pos < parentNode->pointer.pNode.size(); pos++) {
        if (parentNode->pointer.pNode[pos] == currentNode) {
            parentLeft = pos - 1;
            parentRight = pos + 1;
            break;
        }
    }

    // attempt to borrow a key from the left sibling if we have a left sibling
    if (parentLeft >= 0) {
        Node *leftNode = parentNode->pointer.pNode[parentLeft];

        // check if left sibling has extra key to lend
        if (leftNode->keys.size() >= (getMaxInternalChild() + 1) / 2) {

            // transfer the key from left sibling through parentNode
            auto maxIdxKey = leftNode->keys.size() - 1;
            currentNode->keys.insert(currentNode->keys.begin(), parentNode->keys[parentLeft]);
            parentNode->keys[parentLeft] = leftNode->keys[maxIdxKey];

            auto maxIdxPtr = leftNode->pointer.pNode.size() - 1;
            currentNode->pointer.pNode.insert(currentNode->pointer.pNode.begin(), leftNode->pointer.pNode[maxIdxPtr]);

            // resize the left sibling node
            leftNode->keys.resize(maxIdxKey);
            leftNode->pointer.pData.resize(maxIdxPtr);

            return;
        }
    }

    // attempt to borrow a key from the right sibling if we have a right sibling
    if (parentRight < parentNode->pointer.pNode.size()) {
        Node *rightNode = parentNode->pointer.pNode[parentRight];

        // check if right sibling has extra key to lend
        if (rightNode->keys.size() >= (getMaxInternalChild() + 1) / 2) {

            // transfer the key from right sibling through parentNode
            auto maxIdxKey = rightNode->keys.size() - 1;
            currentNode->keys.push_back(parentNode->keys[pos]);
            parentNode->keys[pos] = rightNode->keys[0];
            rightNode->keys.erase(rightNode->keys.begin());

            // transfer the pointer from parentRight to currentNode
            currentNode->pointer.pNode.push_back(rightNode->pointer.pNode[0]);
            currentNode->pointer.pNode.erase(currentNode->pointer.pNode.begin());

            return;
        }
    }

    // merge nodes
    if (parentLeft >= 0) {
        // leftNode + parentNode key + currentNode
        Node *leftNode = parentNode->pointer.pNode[parentLeft];
        leftNode->keys.push_back(parentNode->keys[parentLeft]);

        for (int val: currentNode->keys) {
            leftNode->keys.push_back(val);
        }

        for (auto & i : currentNode->pointer.pNode) {
            leftNode->pointer.pNode.push_back(i);
            i = nullptr;
        }

        currentNode->pointer.pNode.resize(0);
        currentNode->keys.resize(0);

        removeInternal(parentNode->keys[parentLeft], parentNode, currentNode);
    } else if (parentRight < parentNode->pointer.pNode.size()) {

        //currentNode + parentkey +rightNode
        Node *rightNode = parentNode->pointer.pNode[parentRight];
        currentNode->keys.push_back(parentNode->keys[parentRight - 1]);

        for (int val: rightNode->keys) {
            currentNode->keys.push_back(val);
        }

        for (auto & i : rightNode->pointer.pNode) {
            currentNode->pointer.pNode
                    .push_back(i);
            i = nullptr;
        }

        rightNode->pointer.pNode.resize(0);
        rightNode->keys.resize(0);

        removeInternal(parentNode->keys[parentRight - 1], parentNode, rightNode);
    }
}