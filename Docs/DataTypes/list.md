# List

Implements an array-based list. Lists grow automatically when needed. Lists can be sorted or unsorted. New elements can be appended to the either end or inserted at a specific location. Sorted lists require a compare function to be provided when the list is created.

|Component|Description|
|:---|:---|
|List\*createList(int (\*cmp)(Word, Word), void (\*del)(Word), String(\*key)(Word))|Create an array-based List. Provide a compare function, delete function, and get-key function.|
|void deleteList(List *list)|Delete a list. If there is a delete function call it on each element.|
|void emptyList(List\*)|Make a list empty. If there is a delete function call it on each element.|
|bool isEmptyList(List\*)|See if a list is empty.|
|void appendListElement(List\*, Word value)|Add a new element to the end of a List.|
|void prependListElement(List\*, Word element)|Add a new element to the start of a List.|
|void setListElement(List\*, int index, Word element)|Set a specific element in a List.|
|Word getListElement(List\*, int index)|Get an indexed element from a List.|
|bool insertListElement(List\*, int index, Word element)|Insert an element into a List at a specific indexed location.|
|bool insertSortedListElement(List\*, Word element)|Insert an element into its proper location in a sorted List.|
|Word removeListElement(List\*, int index)|Remove an indexed element from a list. This does not affect the sorted state of the list. If the list is empty return the null pointer. The caller takes responsibility to delete the removed element.|
|Word removeFirstListElement(List\*)|Remove and return the first element of a List.|
|Word removeLastListElement(List\*)|Remove and return the last element of a List.|
|void showList(List\*, String (*describe)(Word))|Show the contents of a List. Intended for debugging. If the describe function is not null, call it to get a string form of the element to print. Otherwise assume the elements are strings.|
|void uniqueList(List\*)|Remove duplicates from a List in place. The List is first sorted if it is not already sorted. Uniqueness is defined by the compare function. The list will be sorted even if its keep sorted flag is not set or its length is less than the sort threshold.|
|Word isInList(List\*, Word element)|Check if an element is in the list. The List is sorted if it is not already. The element, if found, is returned, and should be cast to its real type. *Isn't there a need to check for elements in unsorted lists.*|
|void iterateList(List\*, void(*iterate)(Word))|Iterate the elements of a list doing something.|
|int lengthList(List\*)|Return the length of a List.|
|static void growList(List\*)|Grow the length of a List.|
|void sortList(List\*, bool force)|Sort a list. If force is true sort the list regardless of its length. If force is false, sort the list if its length is above the sort threshold.
