#
# @progname    ahnentafel_tutorial.py
# @version      1.0
# @author       David Taylor, derived from ahnentafel_tutorial.ll by Thomas Wetmore
# @category     sample
# @output       text
# @description
#
# Generate an ahnentafel chart for the selected person (tutorial sample).
#

import llines
import sys

def main ():
    indi = llines.getindi()
    ilist = []
    alist = []
    ilist.append(indi)
    alist.append(1)

    # list of people needing to be displayed
    # ancestor numbers for people on ilist
    #
    # Our basic loop is we take the next person who needs to be displayed,
    # display them, and then record their parents as needing to be displayed.

    for indi in ilist:
        # display person we just pulled off list

        ahnen = alist.pop(0)

        print(ahnen, ". ", indi.name())
        e = indi.birth()
        if e:
            print(" b. ", e.long())
        e = indi.death()
        if e:
            print(" d. ", e.long())
        # add person’s parents to list to display
        par = indi.father()
        if par:
            ilist.append(par)
            alist.append(2*ahnen)

        par = indi.mother()
        if par:
            ilist.append(par)
            alist.append(1 + 2*ahnen)

if __name__ == '__main__':
    main()
    sys.stdout.flush()

