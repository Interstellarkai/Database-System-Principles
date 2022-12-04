#ifndef TREE_H
#define TREE_H

#include <vector>
#include "dtypes.h"

class Node {
public:
    bool isLeafNode;
    std::vector<int> keys;
    Node *pNextLeaf;

    union ptr {
        std::vector<Node *> pNode;
        std::vector<std::vector<Record *>> pData;
        ptr();
        ~ptr();
    } pointer;

    friend class Tree;

public:
    Node();
};

class Tree {
private:
    int maxInternalChild;
    int n;
    int nodesAccessedNum;
    Node *rootNode;

    void insertInternal(int x, Node **currentNode, Node **child);

    Node **findParentNode(Node *currentNode, Node *child);

public:
    explicit Tree(int blockSize);

    Node *getRoot();

    int countNodes();

    int countHeight();

    int getMaxInternalChild();

    int getN();

    int getNodesAccessedNum();

    void setNodesAccessedNum(int setNumber);

    void setRoot(Node *);

    void displayCurrentNode(Node *currentNode);

    std::vector<Record *> *search(int key, bool printNode);

    Node *searchNode(int key, bool printNode);

    void insert(int key, Record *pRecord);

    void removeKey(int key);

    void removeInternal(int x, Node *currentNode, Node *child);

};


#endif
