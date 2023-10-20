//
//  DeadEnds Project
//
//  nameindex.c -- Implements a name index that is built with a hash table. Gedcom names are
//    mapped to name keys. Name keys are the keys in the name index. The index maps the name
//    keys to the list of keys of the persons who have names that map to the name key.
//
//  Created by Thomas Wetmore on 26 November 2022.
//  Last changed on 22 April 2023.
//

#include <ansidecl.h>		/* ATTRIBUTE_UNUSED */

#include "nameindex.h"
#include "name.h"
#include "sort.h"
#include "set.h"
#include "gedcom.h"

// NameKeys -- Compare two name keys.
//--------------------------------------------------------------------------------------------------
static int compareNameKeys(Word leftEl, Word rightEl)
{
    String a = ((NameElement*) leftEl)->nameKey;
    String b = ((NameElement*) rightEl)->nameKey;
    return strcmp(a, b);
}

// getNameKey -- Get the name key of an element.
//--------------------------------------------------------------------------------------------------
static String getNameKey(Word element) { return ((NameElement*) element)->nameKey; }

//  deleteNameElement
//--------------------------------------------------------------------------------------------------
static void deleteNameElement(Word element)
{
    NameElement *nameEl = (NameElement*) element;
    stdfree(nameEl->nameKey);
    deleteSet(nameEl->recordKeys);
    stdfree(nameEl);
}

//  createNameIndex -- Create a name index from a hash table.
//--------------------------------------------------------------------------------------------------
NameIndex *createNameIndex(void)
{
    return createHashTable(compareNameKeys, deleteNameElement, getNameKey);
}

//  deleteNameIndex -- Delete a name index.
//--------------------------------------------------------------------------------------------------
void deleteNameIndex(NameIndex *nameIndex)
{
    deleteHashTable(nameIndex);
}

//  insertNameIndex -- Add a (name key, person key) pair to a name index.
//    MNOTE: nameKey is newly allocated; personKey is in static memory.
//--------------------------------------------------------------------------------------------------
void insertInNameIndex(NameIndex *index, String nameKey, String personKey)
//  index -- Name index to update.
//  nameKey -- Name key to insert.
//  personKey -- Person key to insert.
{
    //  Hash the name key to get a bucket index; create a bucket if it does not exist.
    int hash = getHash(nameKey);
    Bucket *bucket = index->buckets[hash];
    if (!bucket) {
        bucket = createBucket();
        index->buckets[hash] = bucket;
    }

    //  See if there is an element for the name key; create if not.
    NameElement *element = searchBucket(bucket, nameKey, index->compare, index->getKey, null);
    if (!element) {
        element = (NameElement*) stdalloc(sizeof(NameElement));
        element->nameKey = strsave(nameKey);  // MNOTE: nameKey is in data space.
        element->recordKeys = createSet(compareRecordKeys, null);
        appendToBucket(bucket, element);
    }
    //  Add the person key to element's set of person keys.
    //    MNOTE: personKey is in data space (in the rmvat function), so it must be saved.
    if (!isInSet(element->recordKeys, personKey))
        addToSet(element->recordKeys, strsave(personKey));  //  MNOTE: personKey is in data space.
}

//  searchNameIndex -- Search a name index for a name.
//--------------------------------------------------------------------------------------------------
Set *searchNameIndex(NameIndex *index, String name)
//  index -- Name index, a specialized hash table.
//  name -- Name being search for.
{
    ASSERT(index && name);
    String nameKey = nameToNameKey(name);
    NameElement* element = searchHashTable(index, nameKey);
    return element == null ? null : element->recordKeys;
}

//  showNameIndex -- Show the contents of a name index; for debugging.
//--------------------------------------------------------------------------------------------------
void showNameIndex(NameIndex *index)
//  index -- Name index, a specialized hash table.
{
    ASSERT(index);
    //  Iterate through the buckets.
    for (int i = 0; i < MAX_HASH; i++) {
        if (!index->buckets[i]) continue;  // Don't show anything for empty Buckets.
        //  The ith Bucket has something in it.
        Bucket *bucket = index->buckets[i];
        //  A bucket contains a List of elements sorted by name key; get that list.
        Word *elements = bucket->elements;
        printf("Bucket %d:\n", i);
        // Iterate through the elements of the ith Bucket.
        for (int j = 0; j < bucket->length; j++) {
            // An element is tuple of a name key and a set of person keys.
            NameElement* element = elements[j];
            printf("    Name key %s:\n", element->nameKey);
            // Get the set of record keys in this element and show them.
            Set *recordKeys = element->recordKeys;
            for (int k = 0; k < lengthSet(recordKeys); k++) {
                printf("        %s\n", (String) recordKeys->list->data[k]);
            }
        }
    }
}
