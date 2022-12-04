#include <iostream>
#include "tree.h"

using namespace std;

void Tree::displayCurrentNode(Node *currentNode) {
    /*
     * Displays the keys stored in the given node.
     */

    // return if node is null
    if (currentNode == nullptr) return;

    // print keys of the node
    cout << "{";
    for (auto i = 0; i < currentNode->keys.size(); i++) {
        cout << currentNode->keys.at(i);
        if (i != currentNode->keys.size() - 1)
            cout << ", ";
    }
    cout << "}" << endl;
}

