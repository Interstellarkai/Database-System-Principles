#include "disk.h"
#include "tree.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <assert.h>

using namespace std;

const int blockSize = 500;  // block size in bytes

void experiment12(Tree *tree, Disk *disk) {
    /*
     * Combines experiments 1 and 2:
     *  -> Insert record into disk
     *  -> Build B+ tree with numVotes attribute
     */
    cout << "EXPERIMENT 1 & 2" << endl;

    // declare pointer to hold newly created records
    Record *newRecord = nullptr;

    // read and parse the data file from "../data/data.tsv"
    cout << "Inserting records from the data file into disk and building index..." << endl;
    ifstream data_file("../data/data.tsv");
    string line;

    // skip first line
    getline(data_file, line);

    // insert records into the disk and index sequentially
    int count = 0;
    while (getline(data_file, line)) {
        // parse the line
        istringstream iss(line);
        string tconst, averageRating, numVotes;
        getline(iss, tconst, '\t');
        getline(iss, averageRating, '\t');
        getline(iss, numVotes, '\t');

        // insert into disk
        newRecord = (*disk).insertRecord(tconst,
                                         (unsigned char) (stof(averageRating) * 10),
                                         stoi(numVotes));
        // insert into tree
        (*tree).insert(newRecord->numVotes, newRecord);
        count++;
    }

    cout << " -> No of records processed: " << count << endl;

    // print experiment 1 outputs
    cout << " -> No of blocks used: " << (*disk).getBlocksUsed() << " blocks" << endl;
    cout << " -> Size of the database (blocks used x blockSize): " << (*disk).getBlocksUsed() * blockSize << " bytes"
         << endl;

    // print experiment 2 outputs
    cout << " -> Parameter N of the B+ Tree: " << (*tree).getN() << endl;
    cout << " -> No of nodes in the B+ Tree: " << (*tree).countNodes() << endl;
    cout << " -> Height of the B+ Tree: " << (*tree).countHeight() << endl;
    cout << " -> Content of rootNode: ";
    (*tree).displayCurrentNode((*tree).getRoot());
    cout << " -> Content of rootNode's first child node: ";
    (*tree).displayCurrentNode((*tree).getRoot()->pointer.pNode[0]);

    // reset number of index nodes accessed
    tree->setNodesAccessedNum(0);

    cout << "===========================================" << endl;
}

void experiment3(Tree *tree, Disk *disk) {
    /*
     * Retrieve records with numVotes = 500 and print statistics
     */
    cout << "EXPERIMENT 3" << endl;

    // retrieve records with numVotes = 500
    cout << " -> Index Nodes accessed: " << endl;
    vector<Record *> *result = tree->search(500, true);

    // print number of index nodes accessed during the search
    cout << " -> No of Index nodes accessed: " << tree->getNodesAccessedNum() << endl;

    // store all block numbers of the records into a vector
    vector<size_t> blockIDList;
    for (Record *record: (*result)) {
        blockIDList.push_back(disk->getBlockId(record));
    }

    // remove duplicates by inserting blockIDs into a set
    set<size_t> s;
    for (size_t blkID: blockIDList) {
        s.insert(blkID);
    }

    // print content of first 5 data blocks accessed
    cout << " -> Data Blocks accessed: " << endl;
    for (int i = 0; i < 5 && i < blockIDList.size(); i++) {
        disk->printBlock(blockIDList.at(i));
    }

    // print number of data blocks accessed
    cout << " -> No of Data blocks accessed: " << blockIDList.size() << endl;
    cout << " -> No of unique Data blocks accessed: " << s.size() << endl;

    // compute average of "averageRating"
    unsigned int total = 0;
    for (Record *record: (*result)) {
        total += record->averageRating;
    }
    cout << " -> Average of averageRating: " << ((float) total / 10) / result->size() << endl;

    // reset number of index nodes accessed
    tree->setNodesAccessedNum(0);

    cout << "===========================================" << endl;
}

void experiment4(Tree *tree, Disk *disk) {
    /*
     * Retrieve records with numVotes from 30,000 to 40,000 and print statistics
     */
    cout << "EXPERIMENT 4" << endl;

    int key1 = 30000;
    int key2 = 40000;

    // search for the leaf node that the key1 should reside in (it may not exist)
    cout << " -> Index Nodes accessed (first 5):" << endl;
    Node *currentNode = tree->searchNode(key1, true);

    int indexNodesAccessed = tree->getNodesAccessedNum();
    int total_average_rating = 0;
    int idx = lower_bound(currentNode->keys.begin(), currentNode->keys.end(), key1) - currentNode->keys.begin();

    // store all block numbers of the records into a vector
    vector<size_t> blockIDList;

    bool finished = false;
    while (currentNode != nullptr && !finished) {
        // print the currentNode leaf node (at the currentNode)
        if (indexNodesAccessed <= 5) {
            tree->displayCurrentNode(currentNode);
        }

        indexNodesAccessed++;

        // iterate through pointers in the currentNode leaf node
        for (int i = idx; i < currentNode->keys.size(); i++) {

            // when upper bound of the key is reached
            if (currentNode->keys.at(i) > key2) {
                finished = true;
                break;
            }

            assert(key1 <= currentNode->keys.at(i));
            assert(key2 >= currentNode->keys.at(i));

            // iterate through the records vector to
            for (Record *record: currentNode->pointer.pData.at(i)) {
                // ensure all records here have the same key (tree did not wrongly index a record)
                assert(record->numVotes == currentNode->keys.at(i));

                // accumulate the total averageRating
                total_average_rating += record->averageRating;

                // store the block ID accessed
                blockIDList.push_back(disk->getBlockId(record));
            }
        }

        // reset idx to 0
        idx = 0;

        // move to the next leaf node
        currentNode = currentNode->pNextLeaf;
    }

    cout << " -> No of Index Nodes Accessed: " << indexNodesAccessed << endl;

    // print content of first 5 data blocks accessed
    cout << " -> Data Blocks accessed: " << endl;
    for (int i = 0; i < 5 && i < blockIDList.size(); i++) {
        disk->printBlock(blockIDList.at(i));
    }

    // remove duplicates by inserting blockIDs into a set
    set<size_t> s;
    for (size_t blkID: blockIDList) {
        s.insert(blkID);
    }

    // print number of data blocks accessed
    cout << " -> No of Data blocks accessed: " << blockIDList.size() << endl;
    cout << " -> No of unique Data blocks accessed: " << s.size() << endl;

    // compute and print average of "averageRating"
    cout << " -> Average of averageRating: " << ((float) total_average_rating / 10) / blockIDList.size() << endl;

    tree->setNodesAccessedNum(0);

    cout << "===========================================" << endl;
}

void experiment5(Tree *tree, Disk *disk) {
    /*
     * Remove the records with numVotes = 1,000, update the tree and print statistics
     */
    cout << "EXPERIMENT 5" << endl;

    // original number of nodes in the tree
    int numNodes = tree->countNodes();

    tree->removeKey(1000);

    // currentNode number of nodes after removal of key=100
    int numUpdatedNodes = tree->countNodes();

    cout << " -> No of times that a node is deleted (or two nodes are merged): " << numNodes - numUpdatedNodes << endl;
    cout << " -> No of nodes in the updated B+ tree: " << numUpdatedNodes << endl;
    cout << " -> Height of the updated B+ tree: " << tree->countHeight() << endl;
    cout << " -> Content of rootNode: ";
    tree->displayCurrentNode(tree->getRoot());
    cout << " -> Content of rootNode's first child node: ";
    tree->displayCurrentNode(tree->getRoot()->pointer.pNode[0]);

    // verify that the key has been deleted
    vector<Record *> *result = tree->search(1000, false);
    assert(result == nullptr);

    // reset number of index nodes accessed
    tree->setNodesAccessedNum(0);

    cout << "===========================================" << endl;
}

int main() {
    // instantiate a disk of 100MB
    Disk disk = Disk((100 * 1000 * 1000), blockSize);

    // instantiate an empty b+ tree
    Tree tree = Tree(blockSize);

    // run experiment 1 and 2
    experiment12(&tree, &disk);

    // run experiment 3
    experiment3(&tree, &disk);

    // run experiment 4
    experiment4(&tree, &disk);

    // run experiment 5
    experiment5(&tree, &disk);

    cout << "End of program! " << endl;
}
