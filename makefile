all:
	echo "HELLO, MAKE"
	cd Database; make
	cd Gedcom; make
	cd Interp; make
	cd Parser; make
	cd DataTypes; make
	cd Utils; make
