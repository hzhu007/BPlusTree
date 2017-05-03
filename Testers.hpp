#ifndef Testers_hpp
#define Testers_hpp

void sequentialTestForInsertion() {
    SeqBPlusTree tree = SeqBPlusTree();
    // try to repeat the process from http://www.cburch.com/cs/340/reading/btree/

    /* Insertion */
    /* phase 1 */
    tree.insert(1, 1);
    //    cout << "insert 1 finish";
    tree.insert(4, 4);
    tree.insert(16, 16);
    tree.insert(25, 25);
//    tree.print();
//    cout << endl;
//    exit(0);
    /* phase 2 */
    tree.insert(9, 9);
    tree.insert(20, 20);
    tree.insert(13, 13);
//    tree.print();
//    cout << endl;
    /* phase 3 */
    tree.insert(15, 15);
    tree.insert(10, 10);
    tree.insert(11, 11);
//    tree.print();
//    cout << endl;
    /* phase 4 */
    tree.insert(12, 12);
//    tree.print();
//    cout << endl;
/* Expect
          [13,             M]
         /                 \
     [9, 11, M]         [16,  M]
    /    \    \         /     \
   1,4  9,10  11,12  13,15  16,20,25
 */
    exit(0);
}

void sequentialTestForDeletion() {
    /* Deletion */
    /* phase 1: initialization */
    tree = SeqBPlusTree();
    tree.insert(1, 1);
    tree.insert(40, 40);
    tree.insert(60, 60);
    tree.insert(90, 90);

    tree.insert(15, 15);
    tree.insert(30, 30);

    tree.insert(10, 10);
    tree.insert(20, 20);

    tree.insert(50, 50);
    tree.insert(55, 55);
    tree.insert(16, 16);
    tree.insert(58, 58);
    // tree.print();
    // cout << endl;
    // exit(0);
/* Expect
             [50,               M]
              /                  \
       [15,   30,   M]       [60,     M]
      /       |      \        /       \
    1,10  15,16,20  30,40  50,55,58  60,90
*/

    /* phase 2: test borrow from left leaf under the same parent  */
    tree.remove(60);
    // tree.print();
    // cout << endl;
    // exit(0);
/* Expect
             [50,               M]
              /                  \
       [15,   30,   M]       [58,   M]
      /       |      \        /     \
    1,10  15,16,20  30,40  50,55  58,90
*/

    /* phase 3: test borrow from left leaf not in the same subtree */
    tree.insert(45, 45);
    tree.remove(55);
    // tree.print();
    // cout << endl;
    // exit(0);
/* Expect
             [45,               M]
              /                  \
       [15,   30,   M]       [58,   M]
      /       |      \        /     \
    1,10  15,16,20  30,40  45,50  58,90
*/

    /* phase 4: test borrow from right leaf */
    tree.remove(1);
    // tree.print();
    // cout << endl;
    // exit(0);
/* Expect
            [45,               M]
             /                 \
       [16,   30,   M]      [58,   M]
       /       |      \      /     \
    10,15  16,20  30,40   45,50  58,90
*/

    /* phase 5: test merge to left leaf then borrow from left internal */
    tree.remove(45);
    // tree.print();
    // cout << endl;
    // exit(0);
/* Expect
            [30,          M]
             /            \
       [16,    M]     [58,   M]
       /       |       /     \
    10,15  16,20   30,40,50  58,90
*/
    /* phase 6: test merge to right leaf then borrow from right internal */
    tree.insert(35, 35);
    // tree.print();
    // cout << endl;
    // exit(0);
    /* Expect
                [30,            M]
                 /              \
           [16,   M]     [40,  58,   M]
           /      |       /     |    \
        10,15  16,20   30,35  40,50  58,90
    */
    tree.remove(10);
    // tree.print();
    // cout << endl;
    // exit(0);
/* Expect
            [40,          M]
             /            \
       [30,    M]     [58,   M]
       /       |       /     \
    15,16,20  30,35  40,50  58,90
*/

    /* phase 7: test merge to left leaf then merge to left internal */
    tree.insert(60, 60);
    tree.insert(70, 70);

    tree.insert(80, 80);
    tree.insert(85, 85);

    tree.insert(94, 94);
    tree.insert(98, 98);
    // tree.print();
    // cout << endl;
    // exit(0);
    /* Expect
                [40,             85                M]
                 /                |                |
           [30,    M]     [58,   70,     M]    [94,   M]
           /       |       /      |      \      /     \
        15,16,20  30,35  40,50  58,60  70,80  85,90  94,98
    */
    tree.insert(84, 84);
    tree.insert(73, 73);

    tree.insert(42, 42);
    tree.insert(48, 48);

    // tree.print();
    // cout << endl;
    // exit(0);
    /* Expect
                [40,             70,               85           M]
                 /                |                |            \
           [30,    M]     [48,    58,     M]    [80,   M]    [94,   M]
           /       |       /       |      \      /     \      /     \
        15,16,20  30,35  40,42  48,50  58,60  70,73  80,84  85,90  94,98
    */
    tree.remove(98);
    tree.print();
    cout << endl;
    exit(0);
/* Expect
            [40,             70,                   M
             /                |                    |
       [30,    M]      [48,   58,     M]    [80,   85,    M]
       /       |        /      |      \      /     |      \
    15,16,20  30,35  40,42  48,50  58,60  70,73  80,84  85,90,94
*/

    /* phase 8: test merge to right leaf, then borrow from left internl,
        then merge to left leaf, then merge to right internal */
    tree.remove(20);
    tree.remove(16);
    // tree.print();
    // cout << endl;
    // exit(0);
    /* Expect
                [48,           70,             M]
                 /              |               \
           [40,     M]      [58,  M]    [80,   85,   M]
            /       |        /    |       /     |      \
        15,30,35  40,42  48,50  58,60  70,73  80,84  85,90,94
    */
    tree.remove(40);
    tree.remove(42);
    // tree.print();
    // cout << endl;
    // exit(0);
/* Expect
               [70,                   M]
                /                     \
       [48,    58,    M]       [80,   85,   M]
        /       |      \       /      |      \
    15,30,35  48,50  58,60  70,73  80,84  85,90,94
*/

    /* phase 9: test reset root */
    tree.remove(58);
    // tree.print();
    tree.remove(84);
    // tree.print();
    tree.remove(35);
    // tree.print();
    tree.remove(48);
    // tree.print();
    // cout << endl;
    // exit(0);
    /* Expect
               [70,           M]
                /             \
           [48,  M]       [85,     M]
            /    |         /        \
        15,30  50,60   70,73,80  85,90,94
    */
    tree.remove(15);
    // tree.print();
    // cout << endl;
    // exit(0);
/* Expect
           [70,     85,      M]
            /        |        \
       30,50,60  70,73,80  85,90,94
*/
}

#endif /* Testers_hpp */
