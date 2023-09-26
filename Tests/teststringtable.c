#include "stringtable.h"

int main(void)
{
    StringTable *table = createStringTable();

    insertInStringTable(table, "a", "a");
    insertInStringTable(table, "c", "c");
    insertInStringTable(table, "tom", "Thomas Trask Wetmore IV");

    showStringTable(table);
}