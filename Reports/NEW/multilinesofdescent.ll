/*
 * @progname    multilinesofdesc.ll
 * @version     1 (3.07.2023)
 * @author      Stanislaw Prinke
 *              in collaboration with Rafal Prinke
 * @category
 * @output      text
 * @description

The program outputs JSON code for GRAPHVIZ diagram showing all
lines of descent of one person from one or more selected ancestors.

The code should be copied and pasted into the edit window at:

https://stanprinke.github.io/genTree/graph.html?gs=1400

where image resolution can be adjusted and a PNG file generated,
or the diagram may be printed out. The data remain on your computer
and are not sent anywhere.

NOTE: the selected persons are highlighted with red font but
when they also appear in another line, the highlighting may not appear
(depending on the order of entering the ancestors).

If a person entered as an ancestor is not an ancestor, the fact
is displayed on screen after the program ends.

With dynastic or noble genealogies the diagrams may get quite complex.
A sample of a relatively simple 1:1 dynastic descent generated
from the legendary ROYALS92.GED file can be seen here:

http://rafalp.home.amu.edu.pl/graph_tudor-windsor.png

*/


global(visited)
global(common_relatives)
global(startingPoints)
global(startingPointsParents)

proc main () {
	list(mancestors)
	getindi(to, "Which descendant?")

	if(to) {
		set(more, "y")
		while(nestr(more, "n")) {
			getindi(from, "Which ancestor?")
			if(from) {
				enqueue(mancestors, from)
				set(from, 0)
			}
			getstr(more, "Add another ancestor? y/n")
		}

		if(eq(length(mancestors), 0)) {
			print("No ancestors selected")
			return()
		}
	} else {
		print("No descendant selected")
		return()
	}

	getstr(addspouses, "Include spouses? y/n")

	"strict digraph mygraph {
	bgcolor=\"#ffeecc\";
	node [shape=box;style=filled;color=\"NavajoWhite\";fontname=\"Helvetica\"; fontsize=\"12\"]; \n\n"

	while(from, dequeue(mancestors)) {

		/* indiset(common_relatives) */
		set(common_relatives, findCommonRelatives(from, to))

		if (eq(length(common_relatives), 0)) {
			print ("No direct line found from ", name(from), "\n")
			continue()
		}

		list(startingPoints)
		list(startingPointsParents)
		table(visited)

		enqueue(startingPoints, from)
		set(lineNumber, 1)

		while(person, dequeue(startingPoints)) {
			set(parent, dequeue(startingPointsParents))
			"\n\n"
			call processOneLine(lineNumber, person, parent, to)
			set(lineNumber, add(lineNumber, 1))
		}

		"\n\n"
		forindiset(common_relatives, person, x, y) {

			key(person) " [label=<"

			if(eqstr(key(person), key(from))) {
				"<font color=\"red\">" name(person) "</font>"
			} else {
				name(person)
			}

			"<br/><font point-size=\"9\">" lifespan(person) "<br/>"

			if(eqstr(addspouses, "y")) {
				families(person, famv, spousev, numberv) {
					"x"
					if(gt(nfamilies(person), 1)) { " (" d(numberv) ")" }
					" " name(spousev) "<br/>"
				}
			}

			"</font>>]"
		}
	}

	"}\n"
}


func findCommonRelatives(from, to) {
	indiset(top)
	indiset(bottom)
	indiset(anc)
	indiset(desc)

	addtoset(top, from, 0)
	set(desc, descendantset(top))
	addtoset(desc, from, 0)

	addtoset(bottom, to, 0)
	set(anc, ancestorset(bottom))
	addtoset(anc, to, 0)

	return(intersect(anc, desc))
}

func lifespan(i) {
	set(bb, birth(i))
	set(dd, death(i))
	set(yy, concat(extractyear(bb), "-", extractyear(dd)))
	return(yy)
}

func extractyear(b) {
	set(mod, trim(date(b), 3))
	if(eqstr(mod, "ABT")) { set(b, concat("c", year(b))) }
	elsif(eqstr(mod, "AFT")) { set(b, concat("p", year(b))) }
	elsif(eqstr(mod, "BEF")) { set(b, concat("a", year(b))) }
	elsif(eqstr(mod, "BET")) {
		set(from, sub(strlen(date(b)), 3))
		set(to, strlen(date(b)))
		set(y2, substring(date(b), from, to))
		if(eqstr(substring(y2, 1, 1), " ")) { set(y2, substring(y2, 2, strlen(date(b)))) }
		set(b, concat(year(b), "/", y2))
	}
	else {
		set(b, year(b))
	}

	return(b)
}

proc processOneLine(lineNumber, person, parent, to) {
	if(parent) {
		key(parent) " -> "
	}

	call processNextPersonInLine(lineNumber, 1, person, to)
}

proc processNextPersonInLine(lineNumber, personNumber, person, to) {
	insert(visited, key(person), concat(d(lineNumber), ":", d(personNumber)))

	if (eq(person, to)) {
		key(person) "\n"
		return()
	} else {
		key(person) " -> "
	}

	set(personNumber, add(personNumber, 1))

	set(lineFinalized, 0)

	families(person, famv, spousev, numberv) {
		children(famv, childv, childnr) {
			if(lookup(visited, key(childv))) {
				/* this child was already visited in a different line, so we have reached the end of the current one */
				key(childv) "\n"
				set(lineFinalized, 1)
			}
			elsif(inset(common_relatives, childv)) {
				if(not(lineFinalized)) {
					/* first matching child (found in the common relatives set) - continue the current line down this path (recursion) */
					call processNextPersonInLine(lineNumber, personNumber, childv, to)
					set(lineFinalized, 1)
				}
				else {
					/* this line is already finalized (printed out), enqueue other matching children as starting points for other lines */
					enqueue(startingPoints, childv)
					enqueue(startingPointsParents, person)
				}
			}
		}
	}
}

