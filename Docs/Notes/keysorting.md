# Key Sorting

Many Gedcom files use person keys like I### and family keys like F###. When these keys are sorted alphabetically the results are not satisfying. For example I100 sorts before I2 and so on.

One can imagine Gedcom files in which these pseudo-numeric keys co-exist with other keys. When sorting sets of keys that contain both pseudo-numeric keys purely stirng keys, does it make sense to sort using a custom-defined sort function?

Definition of a key (called a *cross reference identifier* using *Xref* as the symbol in the Gedcom grammar):

|||
|:---|:---|
|Xref|atsign 1*tagchar atsign|
|tagchar | ucletter \| digit \| underscore|

Note that the non-@ part of a key can be single character; the only letters allowed are uppercase ASCII; and the underscore character (U005F) and the ten digits are the other allowed characters.

DeadEnds has the Sequence data structure that implements lists of persons and several operations on those lists. In order to perform some operations the sequences must be sorted. Sorting can be done on anything really, but in general the sorting is done on the keys.
