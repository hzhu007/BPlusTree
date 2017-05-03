#ifndef Sequential_hpp
#define Sequential_hpp

#include <algorithm>
#include <climits>
#include <iostream>
#include <cstdio>
#include <vector>

using namespace std;

// tree brachning factor
const int ORDER = 4;

// two types of nodes:
// internal node for search path guidance (seperator-reference pairs)
// leaf for key-value pair storage
enum NodeType {
    INTERNAL = 0,
    LEAF
};

/*
 * self-defined data structures used in this class
 */
struct Node {
    NodeType type;
    // int height;
    int size; // # of seperators in internal nodes or values in leaves
    Node* parent;
    Node* left_sibling;
    Node* right_sibling;
    int id;

    bool isRoot() {
        return parent == NULL;
    }
    // the maximum num of seperators/values is ORDER - 1
    bool isFull() {
        return size >= ORDER - 1;
    }
    virtual bool isDeficient() = 0; // size is less than necessary, need to borrow or merge
    virtual bool isNearDeficient() = 0; // will be deficient if a key gets removed
    virtual void print() = 0; // for debug
};

struct KeyValuePair {
    int key;
    int value;
};
struct Leaf : Node {
    // key-value pair
    // should be ORDER - 1, but reserve one for inserting into a full leaf then split
    KeyValuePair key_value[ORDER];

    Leaf() {
        type = LEAF;
        size = 0;
        parent = left_sibling = right_sibling = NULL;
    }

    bool isDeficient() {
        // the critical value is actually floor(OEDER/2)
        return size < ORDER / 2;
    }

    bool isNearDeficient() {
        return size == ORDER / 2;
    }

    void print() {
        if (parent) {
            printf("|ID: %2d, size: %d, parent: %2d, " , id, size, parent->id);
        } else {
            printf("|ID: %2d, size: %d, parent: NULL, ", id, size);
        }
        if (left_sibling) {
            printf("LSib: %4d ", left_sibling->id);
        } else {
            printf("LSib: NULL ");
        }
        if (right_sibling) {
            printf("RSib: %4d", right_sibling->id);
        } else {
            printf("RSib: NULL");
        }
        for (int i = 0; i < size; ++i) {
            printf(" (%d)key=%3d,value=%3d", i, key_value[i].key, key_value[i].value);
        }
        cout << "|";
    }
};

struct KeyReferencePair {
    int key;
    Node* reference;
};
struct InternalNode : Node {
    // seperators and references to children
    // references[i]: the child node containing all elements *less* than seperators[i]
    // Dummy seperator: seperators[size] = INT_MAX (not count in size)
    // Should be ORDER (including the dummy), but reserve one for inserting into a full node then split
    // Note an internal node is full if the size hits ORDER-1
    KeyReferencePair key_ref[ORDER+1];

    InternalNode() {
        type = INTERNAL;
        size = 0;
        parent = left_sibling = right_sibling = NULL;
        key_ref[0].key = INT_MAX;
    }

    bool isDeficient() {
        if (isRoot()) {
            return size < 1;
        } else {
            // count the numebr of references, which is the number of seperators+1
            return size + 1 < ORDER / 2;
        }
    }

    bool isNearDeficient() {
        if (isRoot()) {
            return size == 1;
        } else {
            // count the numebr of references, which is the number of seperators+1
            return size + 1 == ORDER / 2;
        }
    }

    void print() {
        if (parent) {
            printf("|ID: %2d, size: %d, parent: %2d, ", id, size, parent->id);
        } else {
            printf("|ID: %2d, size: %d, parent: NULL, ", id, size);
        }
        if (left_sibling) {
            printf("LSib: %4d ", left_sibling->id);
        } else {
            printf("LSib: NULL ");
        }
        if (right_sibling) {
            printf("RSib: %4d", right_sibling->id);
        } else {
            printf("RSib: NULL");
        }
        for (int i = 0; i <= size; ++i) {
            printf(" (%d)key=%3d,childID=%3d", i, key_ref[i].key, key_ref[i].reference->id);
        }
        cout << "|";
    }
};

/*
 * Sequential B+ Tree class
 */
class SeqBPlusTree {
private:
    Node* root;
    int depth;
    int node_count; // # of nodes
    // a monotonicly increasing accumulator for node id assignment
    // may overflow if the numebr nodes ever created by the tree is more than INT_MAX
    // But this is hardly happending because we don't expect the tree size to be
    // that huge (unfortunately the memory will overflow first) or the tree structures
    // to exists for that long time.
    // Possible improvement: maintain a set of ids of deleted nodes in addition
    // to id_accumulator. Only increase id_accumulator and use it to assign a new
    // node id when the set is empty. Otherwise, extract an id from the set for
    // a newly created node.
    int id_accumulator;

public:
    SeqBPlusTree();
    // print the node information by level for debug
    void print();
    // search for the value relative to the given key, return -1 if not exists
    int search(int key);
    // return true: successfully insert a new key-value pair
    // return false: key already exists, replace the previous with the new value
    bool insert(int key, int value);
    // return true if the key-value pair is successfully removed
    // otherwise return false if the key doesn't exist
    bool remove(int key);

// private helper functions
private:
    // return the leaf where the key possibly exists
    Leaf* leaf_search(int key, Node* curr_node);
    // sort the entries (key-value pairs or key-reference pairs) in the node by key
    void sort_entry_by_key(Node* curr_node);
    // return the min key stored in this subtree
    int min_key_in_subtree(Node* curr_node);

    // split the current full leaf and insert a value into its parent
    void split_leaf(Leaf* curr_leaf);
    // Used in split: insert a key into a node's parent and link to the newly split nodes (right_half)
    void parent_insert(Node* curr_node, int key, Node* right_half);
    // split the current full internal node and insert a value into its parent
    void split_internal(InternalNode* curr_node);
    // recusively print the nodes by level
    void print_recusive(vector<Node*> nodeVec);
    // get the KeyReferencePair pointed to the current node from its parent
    KeyReferencePair* get_key_ref_pair_from_parent(Node* curr_node);

    // borrow from or merge to the left(right) sibling leaf
    void borrow_merge_leaf(Leaf* curr_leaf);
    // the current leaf borrows a key-value pair from its sibling
    void borrow_leaf(Leaf* curr_leaf, Leaf* sibling, bool fromLeft);
    // the current leaf merges to its sibling
    void merge_leaf(Leaf* curr_leaf, Leaf* sibling, bool toLeft);

    // borrow from or merge to the left(right) sibling internal node
    void borrow_merge_internal(InternalNode* curr_node);
    // the current node borrows a key-reference pair from its sibling
    void borrow_internal(InternalNode* curr_leaf, InternalNode* sibling, bool fromLeft);
    // the current node merges to its sibling
    void merge_internal(InternalNode* curr_leaf, InternalNode* sibling, bool toLeft);

};

// at the beginning the root should be only a leaf
SeqBPlusTree::SeqBPlusTree() {
    // cout << "constructing SeqBPlusTree" << endl;
    root = new Leaf();
    depth = 0;
    node_count = 1;
    id_accumulator = 1;
    root->id = 1;
    // cout << "construction end" << endl;
}

int SeqBPlusTree::search(int key) {
    Leaf* leaf = leaf_search(key, root);
    for (int i = 0; i < leaf->size; ++i) {
        if (key == leaf->key_value[i].key) {
            return leaf->key_value[i].value;
        }
    }
    return -1;
}

// return true: insert a new key-value pair
// return false: key already exists, replace the previous with the new value
bool SeqBPlusTree::insert(int key, int value) {
    Leaf* leaf = leaf_search(key, root);
    for (int i = 0; i < leaf->size; ++i) {
        if (key == leaf->key_value[i].key) {
            leaf->key_value[i].value = value;
            return false;
        }
    }
    // if the node is full, need to split after insertion
    bool needSplit = leaf->isFull();
    leaf->key_value[leaf->size].key = key;
    leaf->key_value[leaf->size++].value = value;
    sort_entry_by_key(leaf);

    if (needSplit) {
        split_leaf(leaf);
    }

    return true;
}

// return true if the key-value pair is successfully removed
// otherwise return false if the key doesn't exist
bool SeqBPlusTree::remove(int key) {
    Leaf* leaf = leaf_search(key, root);
    if (leaf->size == 0) {
        cerr << "Error: Trying to remove from an empty tree." << endl;
        return false;
    }

    bool keyNotExist = true;
    for (int i = 0; i < leaf->size; ++i) {
        if (key == leaf->key_value[i].key) {
            keyNotExist = false;
            // move the successive key-value forward
            for (int j = i; j < leaf->size - 1; ++j) {
                leaf->key_value[j] = leaf->key_value[j+1];
            }
            leaf->size--;
            // cout << "Leaf ID: " << leaf->id << endl;
            break;
        }
    }

    if (keyNotExist) return false;

    if (leaf->isDeficient()) {
        borrow_merge_leaf(leaf);
    }
    return true;
}

void SeqBPlusTree::print() {
    vector<Node*> rootVec;
    rootVec.push_back(root);
    print_recusive(rootVec);
}

/*
 * Private helper functions
 */
// return the leaf where the key possibly exists
Leaf* SeqBPlusTree::leaf_search(int key, Node* curr_node) {
    if (LEAF == curr_node->type) {
        return (Leaf*) curr_node;
    }
    InternalNode* curr_internal = (InternalNode*) curr_node;
    for (int i = 0; i < curr_internal->size; ++i) {
        if (key < curr_internal->key_ref[i].key) {
            return leaf_search(key, curr_internal->key_ref[i].reference);
        }
    }
    // if the key is lager than every seperator, the only possible location
    // is in the dummy reference
    return leaf_search(key, curr_internal->key_ref[curr_internal->size].reference);
}

// sort the entries (key-value pairs or key-reference pairs) in the node by key
void SeqBPlusTree::sort_entry_by_key(Node* curr_node) {
    if (LEAF == curr_node->type) {
        Leaf* curr_leaf = (Leaf*)curr_node;
        sort(curr_leaf->key_value, curr_leaf->key_value + curr_leaf->size,
            [](KeyValuePair a, KeyValuePair b) {
                return a.key < b.key;
            });
    } else {
        InternalNode* curr_internal = (InternalNode*)curr_node;
        // need to +1 because there is a dummy key INT_MAX at key_ref[size]
        sort(curr_internal->key_ref, curr_internal->key_ref + curr_internal->size+1,
            [](KeyReferencePair a, KeyReferencePair b) {
                return a.key < b.key;
            });
    }
    return;
}

// return the min key stored in this subtree
int SeqBPlusTree::min_key_in_subtree(Node* curr_node) {
    while (LEAF != curr_node->type) {
        curr_node = ((InternalNode*)curr_node)->key_ref[0].reference;
    }
    return ((Leaf*)curr_node)->key_value[0].key;
}

// split the current full leaf and insert a value to its parrent
void SeqBPlusTree::split_leaf(Leaf* curr_node) {
    if (curr_node == NULL || LEAF != curr_node->type || !curr_node->isFull()) {
        cerr << "Not a valid leaf or the leaf is not full." << endl;
        return;
    }

    Leaf* right_half = new Leaf();
    for (int i = curr_node->size/2, j = 0; i < curr_node->size; ++i, ++j) {
        right_half->key_value[j] = curr_node->key_value[i];
        right_half->size++;
    }
    right_half->id = ++id_accumulator;
    ++node_count;

    int medianKey = curr_node->key_value[curr_node->size/2].key;
    curr_node->size = curr_node->size/2;

    // update siblings, from right to left
    if (NULL != curr_node->right_sibling) {
        curr_node->right_sibling->left_sibling = right_half;
    }
    right_half->right_sibling = curr_node->right_sibling;
    right_half->left_sibling  = curr_node;
    curr_node->right_sibling  = right_half;

    parent_insert(curr_node, medianKey, right_half);

    return;
}

// Used in split: insert a key into a node's parent and link to the newly split nodes (right_half)
void SeqBPlusTree::parent_insert(Node* curr_node, int key, Node* right_half) {
    InternalNode* parent = (InternalNode*) curr_node->parent;
    // if the split node is root, we need to add a new root
    if (parent == NULL) {
        parent = new InternalNode();
        depth++;
        parent->id = ++id_accumulator;
        node_count++;
        root = parent;
    }
    // if parent is full, we need to split the parent afterwards
    bool parent_split = parent->isFull();

    // ++ first because there is a dummy key INT_MAX at key_ref[size]
    parent->key_ref[++(parent->size)].key 	= key;
    parent->key_ref[parent->size].reference = curr_node;
    sort_entry_by_key(parent);
    // Search for the first key-reference pair whose key is greater than the
    // inserted key, this pair is also pointed to the current node
    // Now redirect it to the right.
    // Need to use <= because also need to check the dummy key INT_MAX at key_ref[size]
    for (int i = 0; i <= parent->size; ++i) {
        if (key < parent->key_ref[i].key) {
            parent->key_ref[i].reference = right_half;
            break;
        }
    }
    curr_node->parent  = parent;
    right_half->parent = parent;

    // if parent is full, we need to split the parent
    if (parent_split) {
        split_internal(parent);
    }

    return;
}

// split the current full internal node and insert a value into its parent
void SeqBPlusTree::split_internal(InternalNode* curr_node) {
    if (curr_node == NULL || !curr_node->isFull()) {
        cerr << "Not a valid node or the node is not full." << endl;
        return;
    }

    InternalNode* right_half = new InternalNode();
    // Need to use <= because we also want to copy the dummy key INT_MAX at key_ref[size]
    for (int i = curr_node->size/2 + 1, j = 0; i <= curr_node->size; ++i, ++j) {
        right_half->key_ref[j] = curr_node->key_ref[i];
        right_half->size++;
        Node* child = curr_node->key_ref[i].reference;
        child->parent = right_half;
    }
    right_half->size--; // -1 because there is a dummy key INT_MAX at key_ref[size]
    right_half->id = ++id_accumulator;
    ++node_count;

    int medianKey = curr_node->key_ref[curr_node->size/2].key;
    curr_node->size = curr_node->size / 2;
    curr_node->key_ref[curr_node->size].key = INT_MAX;

    // update siblings, from right to left
    if (NULL != curr_node->right_sibling) {
        curr_node->right_sibling->left_sibling = right_half;
    }
    right_half->right_sibling = curr_node->right_sibling;
    right_half->left_sibling  = curr_node;
    curr_node->right_sibling  = right_half;

    parent_insert(curr_node, medianKey, right_half);

    return;
}

// recusively print the nodes by level
void SeqBPlusTree::print_recusive(vector<Node*> nodeVec) {
    vector<Node*> nextLevel;
    bool hit_leaves = LEAF == nodeVec.front()->type;
    for (int i = 0; i < nodeVec.size(); ++i) {
        nodeVec.at(i)->print();
        // cout << " ";
        cout << endl;
        if (hit_leaves) continue;
        for(int j = 0; j <= nodeVec.at(i)->size; ++j) {
            nextLevel.push_back( ((InternalNode*)nodeVec.at(i))->key_ref[j].reference);
        }
    }
    cout << endl;
    if (hit_leaves) return;
    print_recusive(nextLevel);
}

KeyReferencePair* SeqBPlusTree::get_key_ref_pair_from_parent(Node* curr_node) {
    if (curr_node == NULL) {
        cerr << "Not a valid node." << endl;
        return NULL;
    }
    InternalNode* parent = (InternalNode*) curr_node->parent;
    if (parent == NULL) {
        cerr << "Parent is NULL." << endl;
        return NULL;

    }

    for (int i = 0; i <= parent->size; ++i) {
        if (curr_node == parent->key_ref[i].reference) {
            return &parent->key_ref[i];
        }
    }
    return NULL;
}

// Borrow from or merge to the left(right) sibling
// The rule is as follows:
// If the left sibling exists, first try to borrow the largest key-value pair from it.
// If the sibling is also close to deficient, merge the two leaves by appending the
// key-values from the current leaf to that sibling. Then delete the current leaf
// and shift other leaves with the same parent left and update the parent accordingly.
// Note the two leaves may not be in the same subtree, borrowing or merging may
// affect the seperators in there first common root so we need to update that node.
// If the left sibling doesn't exist, i.e. the current leaf is the leftmost one,
// do the same process to the right sibling, except that we borrow the smallest
// key-value pair from the right sibling.
void SeqBPlusTree::borrow_merge_leaf(Leaf* curr_leaf) {
    Leaf* left_sib = (Leaf*)curr_leaf->left_sibling;
    if (left_sib) { // left sibling exists
        // if the left sibling is not close to deficient, we can borrow one
        if ( ! (left_sib->isDeficient() || left_sib->isNearDeficient() ) ) {
            borrow_leaf(curr_leaf, left_sib, true);
        }
        else {
            merge_leaf(curr_leaf, left_sib, true);
        }
    } else { // left sibling doesn't exist, turn to right
        Leaf* right_sib = (Leaf*)curr_leaf->right_sibling;
        if ( ! (right_sib->isDeficient() || right_sib->isNearDeficient() ) ) {
            borrow_leaf(curr_leaf, right_sib, false);
        }
        else {
            merge_leaf(curr_leaf, right_sib, false);
        }
    }
    return;
}

// The current leaf borrows a key-value pair from its sibling.
// If borrowing from left sibling, borrow the largest. If borrowing from right
// sibling, borrow the smallest. Then update the reference to the current leaf
// but no need to update the reference to the sibling as the remaining keys are
// smaller than the key in the key-reference pair from the parent.
void SeqBPlusTree::borrow_leaf(Leaf* curr_leaf, Leaf* sibling, bool fromLeft) {
    int borrowed_key = -1;
    if (fromLeft) { // borrow from left sibling
        curr_leaf->key_value[curr_leaf->size++] = sibling->key_value[--(sibling->size)];
        borrowed_key = curr_leaf->key_value[curr_leaf->size-1].key;
        sort_entry_by_key(curr_leaf);
    }
    else { // borrow from right sibling
        curr_leaf->key_value[curr_leaf->size++] = sibling->key_value[0];
        borrowed_key = curr_leaf->key_value[curr_leaf->size-1].key;
        // move sibling's successive key-value forward
        for (int i = 0; i < sibling->size - 1; ++i) {
            sibling->key_value[i] = sibling->key_value[i+1];
        }
        sibling->size--;
    }
    // Also need to update the reference in the firsr common ancestor because
    // borrowing may affect branching at that node. Note the borrowed key will
    // become the minimum key (from left) or the maximum key (from right) in the
    // subtree where curr_leaf lies.
    if (fromLeft) {
        Node *last_leaf_iter = curr_leaf, *last_sib_iter = sibling;
        Node *leaf_iter = curr_leaf->parent, *sib_iter = sibling->parent;
        while (leaf_iter != sib_iter) {
            last_leaf_iter = leaf_iter;
            leaf_iter = leaf_iter->parent;
            last_sib_iter = sib_iter;
            sib_iter = sib_iter->parent;
        }
        KeyReferencePair* key_ref_to_sib_in_ancestor =
            get_key_ref_pair_from_parent(last_sib_iter);
        key_ref_to_sib_in_ancestor->key = borrowed_key;
    }
    else {
        // Borrowing from right only happens if curr_leaf is the leftmost one,
        // and the branching factor is at least two, so it must share the same
        // parent with its right sibling.
        KeyReferencePair* key_ref_to_curr_in_parent =
            get_key_ref_pair_from_parent(curr_leaf);
        key_ref_to_curr_in_parent->key = sibling->key_value[0].key;
    }

    return;
}

// the current leaf merges with its sibling
void SeqBPlusTree::merge_leaf(Leaf* curr_leaf, Leaf* sibling, bool toLeft) {
    InternalNode* parent = (InternalNode*)curr_leaf->parent;
    if (toLeft) { // merge to left sibling
        Leaf* left_sib = sibling;
        KeyReferencePair* key_ref_to_curr_in_parent =
            get_key_ref_pair_from_parent(curr_leaf);
        bool curr_parent_is_dummy = INT_MAX == key_ref_to_curr_in_parent->key;

        for (int i = 0; i < curr_leaf->size; ++i) {
            left_sib->key_value[left_sib->size + i] = curr_leaf->key_value[i];
        }
        left_sib->size += curr_leaf->size;

        // find the key_ref pair in the parent of curr_leaf and remove it by
        // moving its successive key-ref pairs forward.
        // Use <= because also need to check the dummy key INT_MAX at key_ref[size]
        int idx;
        for (idx = 0; idx <= parent->size; ++idx) {
            if (parent->key_ref[idx].reference == curr_leaf) break;
        }
        for (int i = idx; i < parent->size; ++i) {
            parent->key_ref[i] = parent->key_ref[i+1];
        }
        parent->size--;

        // Also redirect siblings.
        left_sib->right_sibling = curr_leaf->right_sibling;
        if (NULL != curr_leaf->right_sibling)
            curr_leaf->right_sibling->left_sibling = left_sib;

        // The effect of merging to left is the same as borrowing from left so
        // we need to update the reference in the first common ancestor.
        Node *last_leaf_iter = curr_leaf, *last_sib_iter = left_sib;
        Node *leaf_iter = curr_leaf->parent, *sib_iter = left_sib->parent;
        while (leaf_iter != sib_iter) {
            last_leaf_iter = leaf_iter;
            leaf_iter = leaf_iter->parent;
            last_sib_iter = sib_iter;
            sib_iter = sib_iter->parent;
        }
        KeyReferencePair* key_ref_to_sib_in_ancestor =
            get_key_ref_pair_from_parent(last_sib_iter);
        // curr_leaf may be the rightmost one under its parent so its left sibling
        // must share the same parent with it and after merging the left sibling
        // will become the rightmost one.
        if (curr_parent_is_dummy) {
            key_ref_to_sib_in_ancestor->key = INT_MAX;
        }
        else {
            // Merging to the left sibling is the same as the left sibling borrowing
            // from curr_leaf. Note the smallest key in the right after merging is
            // the first key in the right sibling of curr_leaf.
            // Also note if the left sibling and curr_leaf don't share the same parent,
            // the left sibling must be the rightmost one in its subtree whereas the
            // curr_node is the leftmost one. So the reference to the left sibling is
            // the dummy one with key INT_MAX.
            Leaf* right_sib = (Leaf*) curr_leaf->right_sibling;
            int min_key_right = right_sib->key_value[0].key;
            key_ref_to_sib_in_ancestor->key = min_key_right;
        }
    }
    else { // merge to right sibling
        Leaf* right_sib = sibling;
        for (int i = 0; i < curr_leaf->size; ++i) {
            right_sib->key_value[right_sib->size + i] = curr_leaf->key_value[i];
        }
        right_sib->size += curr_leaf->size;
        sort_entry_by_key(right_sib);

        // As merge to right only happens if the curr_leaf is the leftmost one,
        // and the branching factor is at least two, so it must share the same
        // parent with its right sibling. So merging to right doesn't need to
        // modify any reference above, only needs to modify the references in
        // the parent.
        for (int i = 0; i < parent->size; ++i) {
            parent->key_ref[i] = parent->key_ref[i+1];
        }
        parent->size--;

        // Also redirect siblings.
        right_sib->left_sibling = curr_leaf->left_sibling;
    }

    node_count--;
    delete curr_leaf;

    if (parent->isDeficient()) {
        borrow_merge_internal(parent);
    }
}

// Borrow from or merge to the left(right) sibling
// The rule is as follows:
// If the left sibling exists, first try to borrow the largest key-reference pair
// from it. This borrowing will cause inserting a new key-reference to the current node.
// If the sibling is also close to deficient, merge the two nodes by appending the
// key-references from the current node to that sibling. Then delete the current node
// and update the references in the parent accordingly.
// Note the two internal nodes may not be in the same subtree, borrowing or merging
// may affect the seperators in there first common root so we need to update that node.
// If the left sibling doesn't exist, i.e. the current node is the leftmost, do the
// same process to the right sibling, except that we try borrowing the smallest
// key-reference pair.
void SeqBPlusTree::borrow_merge_internal(InternalNode* curr_node) {
    InternalNode* left_sib = (InternalNode*) curr_node->left_sibling;
    if (left_sib) { // left sibling exists
        // if the left sibling is not close to deficient, we can borrow one
        if ( ! (left_sib->isDeficient() || left_sib->isNearDeficient() ) ) {
            borrow_internal(curr_node, left_sib, true);
        }
        else {
            merge_internal(curr_node, left_sib, true);
        }
    } else { // left sibling doesn't exist, turn to right
        InternalNode* right_sib = (InternalNode*) curr_node->right_sibling;
        if ( ! (right_sib->isDeficient() || right_sib->isNearDeficient() ) ) {
            borrow_internal(curr_node, right_sib, false);
        }
        else {
            merge_internal(curr_node, right_sib, false);
        }
    }
    return;
}

// the current node borrows a key-reference pair from its sibling
// If borrowing from left sibling, borrow the largest. If borrowing from right
// sibling, borrow the smallest. Then update the reference to the current leaf
// but no need to update the reference to the sibling as the remaining keys are
// smaller than the key in the key-reference pair from the parent.
void SeqBPlusTree::borrow_internal(InternalNode* curr_node, InternalNode* sibling, bool fromLeft) {
    Node* borrowed_node = NULL;
    if (fromLeft) { // borrow from left sibling
        InternalNode* left_sibling = sibling;
        borrowed_node = left_sibling->key_ref[left_sibling->size].reference;
        // ++ first because there is a dummy key INT_MAX at key_ref[size]
        curr_node->key_ref[++curr_node->size] = left_sibling->key_ref[left_sibling->size--];
        // borrowed one is a dummy reference with key = INT_MAX, so need to modify
        // its key to the smallest key in the first reference
        curr_node->key_ref[curr_node->size].key = min_key_in_subtree(curr_node);
        sort_entry_by_key(curr_node);
        // set the key of the last key-reference pair to INT_MAX
        left_sibling->key_ref[left_sibling->size].key = INT_MAX;

        // Also need to update the reference in the first common ancestor because
        // borrowing may affect branching at that node. Note the borrowed key will
        // become the minimum key in the subtree where curr_node lies.
        Node *last_node_iter = curr_node, *last_sib_iter = left_sibling;
        Node *node_iter = curr_node->parent, *sib_iter = left_sibling->parent;
        while (node_iter != sib_iter) {
            last_node_iter = node_iter;
            node_iter = node_iter->parent;
            last_sib_iter = sib_iter;
            sib_iter = sib_iter->parent;
        }
        KeyReferencePair* key_ref_to_sib_in_ancestor = get_key_ref_pair_from_parent(last_sib_iter);
        key_ref_to_sib_in_ancestor->key = min_key_in_subtree(curr_node);
    }
    else { // borrow from right sibling
        InternalNode* right_sibling = sibling;
        borrowed_node = right_sibling->key_ref[0].reference;
        // ++ first because there is a dummy key INT_MAX at key_ref[size]
        curr_node->key_ref[++curr_node->size] = right_sibling->key_ref[0];
        // delete the borrowed key-reference pair by moving sibling's successive
        // key-reference pairs forward
        for (int i = 0; i < right_sibling->size; ++i) {
            right_sibling->key_ref[i] = right_sibling->key_ref[i+1];
        }
        right_sibling->size--;
        // the remaining key-reference pairs in the current node contains a dummy
        // reference with key = INT_MAX, so need to modify its key to the smallest
        // key in the borrowed reference
        int min_key_in_borrowed = min_key_in_subtree(curr_node->key_ref[curr_node->size].reference);
        curr_node->key_ref[curr_node->size-1].key = min_key_in_borrowed;
        curr_node->key_ref[curr_node->size].key   = INT_MAX;

        // Borrowing from right only happens if curr_node is the leftmost one, and
        // the branching factor is at least two, so it must share the same parent
        // with its right sibling. So we only need to update the reference in parent.
        KeyReferencePair* key_ref_in_parent = get_key_ref_pair_from_parent(curr_node);
        key_ref_in_parent->key = min_key_in_subtree(curr_node->right_sibling);
    }
    borrowed_node->parent = curr_node;
    return;
}

// the current node merges with its sibling
void SeqBPlusTree::merge_internal(InternalNode* curr_node, InternalNode* sibling, bool toLeft) {
    InternalNode* parent = (InternalNode*)curr_node->parent;
    KeyReferencePair* key_ref_to_curr_in_parent = get_key_ref_pair_from_parent(curr_node);
    bool curr_parent_is_dummy = INT_MAX == key_ref_to_curr_in_parent->key;
    if (toLeft) { // merge to left sibling
        InternalNode* left_sib = sibling;
        // +1 because there is a dummy key INT_MAX at key_ref[size]
        for (int i = 0; i <= curr_node->size; ++i) {
            left_sib->key_ref[left_sib->size + 1 + i] = curr_node->key_ref[i];
            left_sib->key_ref[left_sib->size + 1 + i].reference->parent = left_sib;
        }
        // There may be two dummy keys equal INT_MAX after merging.
        // As the left side is always smaller, edit the dummy key in the left sibling
        left_sib->key_ref[left_sib->size].key =
            min_key_in_subtree(curr_node->key_ref[0].reference);
        left_sib->size += curr_node->size + 1;

        // find the key_ref pair in the parent of curr_node and remove it by
        // moving its successive key-ref pairs forward.
        // Use <= because also need to check the dummy key INT_MAX at key_ref[size]
        int idx;
        for (idx = 0; idx <= parent->size; ++idx) {
            if (parent->key_ref[idx].reference == curr_node) break;
        }
        for (int i = idx; i < parent->size; ++i) {
            parent->key_ref[i] = parent->key_ref[i+1];
        }
        parent->size--;

        // Also redirect siblings.
        left_sib->right_sibling = curr_node->right_sibling;
        if (NULL != curr_node->right_sibling)
            curr_node->right_sibling->left_sibling = left_sib;

        // The effect of merging is the same as borrowing so we need to update the
        // reference in the first common ancestor.
        Node *last_node_iter = curr_node, *last_sib_iter = left_sib;
        Node *node_iter = curr_node->parent, *sib_iter = left_sib->parent;
        while (node_iter != sib_iter) {
            last_node_iter = node_iter;
            node_iter = node_iter->parent;
            last_sib_iter = sib_iter;
            sib_iter = sib_iter->parent;
        }
        KeyReferencePair* key_ref_to_sib_in_ancestor =
            get_key_ref_pair_from_parent(last_sib_iter);
        if (curr_parent_is_dummy) {
            key_ref_to_sib_in_ancestor->key = INT_MAX;
        }
        else {
            // Merging to the left sibling is the same as the left sibling borrowing
            // from curr_node. Note the smallest key in the right after merging is
            // the first key in the right sibling of curr_node.
            // Also note if the left sibling and curr_node don't share the same parent,
            // the left sibling must be the rightmost one in its subtree whereas the
            // curr_node is the leftmost one. So the reference to the left sibling is
            // the dummy one with key INT_MAX.
            InternalNode* right_sib = (InternalNode*) curr_node->right_sibling;
            int min_key_right = min_key_in_subtree(right_sib->key_ref[0].reference);
            key_ref_to_sib_in_ancestor->key = min_key_right;
        }
    }
    else { // merge to right sibling
        InternalNode* right_sib = sibling;
        // +1 because there is a dummy key INT_MAX at key_ref[size]
        for (int i = 0; i <= curr_node->size; ++i) {
            right_sib->key_ref[right_sib->size + 1 + i] = curr_node->key_ref[i];
            right_sib->key_ref[right_sib->size + 1 + i].reference->parent = right_sib;
        }
        right_sib->size += curr_node->size + 1;
        // There may be two dummy keys equal INT_MAX after merging.
        // As the right side is always larger, edit the dummy key from the curr_node
        right_sib->key_ref[right_sib->size].key =
            min_key_in_subtree(right_sib->key_ref[0].reference);
        sort_entry_by_key(right_sib);

        // As merge to right only happens if the curr_node is the leftmost one,
        // and the branching factor is at least two, so it must share the same
        // parent with its right sibling.
        // So only need to modify the references in the parent.
        for (int i = 0; i < parent->size; ++i) {
            parent->key_ref[i] = parent->key_ref[i+1];
        }
        parent->size--;

        // Also redirect siblings.
        right_sib->left_sibling = curr_node->left_sibling;
    }

    node_count--;
    delete curr_node;

    if (parent->isDeficient()) {
        if (parent->isRoot()) {
            Node* oldRoot = root;
            root = sibling;
            sibling->parent = NULL;
            node_count--;
            depth--;
            delete oldRoot;
        } else {
            borrow_merge_internal(parent);
        }
    }

    return;
}

#endif /* Sequential_hpp */
