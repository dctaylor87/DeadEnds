#
# @progname    ahnentafel_tutorial.py
# @version      1.0
# @author       David Taylor, derived from tutorial.py
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
    anclist = []
    anclist.append((indi, 1))

    # list of people needing to be displayed
    # ancestor numbers for people on ilist
    #
    # Our basic loop is we take the next person who needs to be displayed,
    # display them, and then record their parents as needing to be displayed.

    for indi, ahnen in anclist:
        # display person we just pulled off list

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
            anclist.append((par, 2*ahnen))

        par = indi.mother()
        if par:
            anclist.append((par, 1 + 2*ahnen))

if __name__ == '__main__':
    main()

