#include <iostream>
#include <queue>
#include "tree.h"

using namespace std;

Node *parentNode = nullptr;

Node::ptr::ptr() {
}

Node::ptr::~ptr() {
}

Node::Node() {
    this->isLeafNode = false;
    this->pNextLeaf = nullptr;
}


Tree::Tree(int blockSize) {
    /*
     *  | |       | |       | |       | |       | |
     *  p - Each pointer is 8 bytes (64-bit address)
     *  k - Each key is 4 byte (unsigned int)
     *  n - Number of key-pointer pairs
     *  Total size of one block = p + n(p+k)
     *  n = (block_size - p) / (p+k)
     *
     *  This is just a simulation, in practice, the size of each Node is more complicated.
     */
    n = (blockSize - 8) / (8 + 4);
    maxInternalChild = n + 1;
    rootNode = nullptr;

    cout << "Instantiating B+ Tree" << endl;
    cout << " -> Nodes bounded by block size of = " << blockSize << endl;
    cout << " -> Maximum number of keys in a node: n = " << n << endl;
    cout << " -> Internal node max pointers to other nodes = " << maxInternalChild << endl;
    cout << " -> Leaf node max key-record pointers = " << n << endl;
    cout << "===========================================" << endl;
}

int Tree::getMaxInternalChild() {
    return maxInternalChild;
}

int Tree::getN() {
    return n;
}

int Tree::getNodesAccessedNum() {
    return nodesAccessedNum;
}

void Tree::setNodesAccessedNum(int setNumber) {
    nodesAccessedNum = setNumber;
}

Node *Tree::getRoot() {
    return this->rootNode;
}

void Tree::setRoot(Node *ptr) {
    this->rootNode = ptr;
}

Node **Tree::findParentNode(Node *currentNode, Node *child) {
    /*
     * Finds and return the parent node.
	 */
    if (currentNode->isLeafNode || currentNode->pointer.pNode[0]->isLeafNode)
        return nullptr;

    for (int i = 0; i < currentNode->pointer.pNode.size(); i++) {
        if (currentNode->pointer.pNode[i] == child) {
            parentNode = currentNode;
        } else {
            Node *temCurrent = currentNode->pointer.pNode[i];
            findParentNode(temCurrent, child);
        }
    }

    return &parentNode;
}

int Tree::countNodes() {
    /*
     * Counts the number of nodes in the b+ tree.
     */
    Node *currentNode = rootNode;
    int count = 0;

    if (currentNode == nullptr)
        return count;

    queue<Node *> q;
    q.push(currentNode);
    count++;

    while (!q.empty()) {
        auto qSize = q.size();
        for (int i = 0; i < qSize; i++) {
            Node *u = q.front();
            q.pop();

            if (!u->isLeafNode) {
                for (Node *v: u->pointer.pNode) {
                    q.push(v);
                    count++;
                }
            }
        }
    }
    return count;
}

int Tree::countHeight() {
    /*
     * Counts the height of the B+ tree.
     */
    int heightOfTree = 0;

    // checks if the tree is empty
    if (rootNode == nullptr) {
        return 0;
    } else {
        // start the currentNode at the rootNode
        Node *currentNode = rootNode;

        // traverse to the leaf node
        while (!currentNode->isLeafNode) {
            heightOfTree++;
            currentNode = currentNode->pointer.pNode[0];
        }

        // count leaf nodes level
        heightOfTree++;
    }
    return heightOfTree;
}