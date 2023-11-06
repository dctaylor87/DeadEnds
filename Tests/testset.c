//  testset.c -- Test program for the Set data type.
//
//  Created by Thomas Wetmore on 3 November 2023.
//  Last changed on 4 November 2023.

#include <stdio.h>
#include <set.h>
#include "gedcom.h"

int compare(Word element1, Word element2)
{
    return compareRecordKeys((String) element1, (String) element2);
}

String getKey(Word element)
{
    return (String) element;
}

int main (void)
{
    // Create a Set.
    Set *set = createSet(compare, null, getKey);
    String keys[] = { "I1", "I1", "I2", "I4", "I10", "I21", "I4", "I5", "I300", "I299" };
    int n = sizeof(keys)/sizeof(String);
    for (int i = 0; i < n; i++) {
        printf("add %s\n", keys[i]);
        addToSet(set, keys[i]);
        showSet(set, null);
        printf("\n");
    }
}