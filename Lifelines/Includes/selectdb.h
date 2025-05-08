extern Database *
selectAndOpenDatabase(CString *dbFilename,
		      CString searchPath,
		      Database *oldDatabase,
		      ErrorLog *errorLog);

/* checkForRefnOverlap  --  return  true   if  okay  to  continue  (no
   overlap), false if need to abort the database merge */
extern bool
checkForRefnOverlap (Database *database, Database *oldDatabase,
		     ErrorLog *errorLog);

/* returns false on failure (out of memory or similar), true on success */
extern bool
rekeyDatabase (Database *database, Database *oldDatabase, ErrorLog *errorLog);

/* mergeDatabases -- merge database into oldDatabase.
   Returns true if merge is successful; false if there were conflicts. */

extern bool
mergeDatabases (Database *database, Database *oldDatabase, ErrorLog *errorLog);

/* databaseHasStandardKeys -- return true if database has LL standard
   keys; false otherwise */
extern bool databaseHasStandardKeys (Database *database);
