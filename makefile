all:
	echo "HELLO, MAKE"
	cd Database; make
	cd Gedcom; make
	cd Interpreter; make
	cd Parser; make
	cd DataTypes; make
	cd Utilities; make
