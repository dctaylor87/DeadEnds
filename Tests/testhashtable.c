//  testhashtable.c -- Test the hastable data type.
//
//  Created by Thomas Wetmore on 25 September 2023.
//  Last changed on 26 September 2023.

#include "hashtable.h"

String getKey(Word key) {
    return (String) key;
}

int main(void)
{
    HashTable *table = createHashTable(NULL, NULL, getKey);
    showHashTable(table, NULL);
}
