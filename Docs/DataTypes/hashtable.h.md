# hashtable.h

|Component|Description|
|:---|:---|
|HashTable *createHashTable(int(\*comp)(Word, Word), void(\*del)(Word), String(\*getKey)(Word));|Create a hash table. Functions to compare, delete, and retrieve keys from elements are passed in.|
|void deleteHashTable(HashTable*)|Delete a HashTable; use the element delete function if it exists.|
|bool isInHashTable(HashTable\*, String key)|Return whether an element with the given key is in the table.|
|Word searchHashTable(HashTable\*, String key)|Return the element that matches the given key if it is in the table; otherwise return null.|
|void insertInHashTable(HashTable*, Word element)|Insert (add) an element into the table. They getKey function is called to extract the key from the element.|
|Word firstInHashTable(HashTable\*, int\*, int\*)|Return the first element in the table. Provide two integers to track the iteration state.|
|Word nextInHashTable(HashTable\*, int\*, int\*)|Return the next element in the table in the iteration defined by the two state variables.|
|int sizeHashTable(HashTable\*)|Return the number of elements in the table.|
|void showHashTable(HashTable\*, void (\*show)(Word))|Show the contents of the hash table &mdash; for debugging.||
|/\*static\*/ int getHash(String)|Return the hashed value of a String. *This function should be  static, but there is a dependency to clean up.*|
|void removeFromHashTable(HashTable\*, String key)|Remove the element with the given key from the hash table. Call the delete function on the element if present.|
|Bucket \*createBucket(void)|Create a bucket. *Should this be static?*|
|void deleteBucket(Bucket\*, void(\*)(Word))|Delete a bucket.|
|Word searchBucket(Bucket\*, String key, int(\*compare)(Word, Word), String (\*getKey)(Word), int \*index)|Search a bucket.|
|void appendToBucket(Bucket\*, Word element)|Append an element to a bucket.|
|void removeElement(HashTable\*, Word element)|Remove an element from the hash table. *How does this relate to the removeFromHashTable function?*|
|int iterateHashTableWithPredicate (HashTable\*, bool(*)(Word element))|Iterate through a hash table performing a function. *Needs a better description.*|