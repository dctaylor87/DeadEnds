#include "stringtable.h"

int main(void)
{
    StringTable *table = createStringTable();

    addToStringTable(table, "a", "a");
    addToStringTable(table, "c", "c");
    addToStringTable(table, "tom", "Thomas Trask Wetmore IV");

    showStringTable(table);
}
