This is a brief description of the protocol used by the ProgClient
and ProgServer classes.  They don't currently work like this, but
will soon.

Message 1:
Direction: ProgClient->ProgServer
Format: HELLO!
Description: Greeting message (is abritrary ATM)

Message 2:
Direction: ProgServer->ProgClient
Format: BOO!
Description: Return of greeting (is abritrary ATM)

Message 3:
Direction: ProgClient->ProgServer
Format: SETQUERY <weighting type> \
		 <first item> \
		 <maxitems> \
                 "<query string>"
Description: The query, results to get, and weighting formula to use.

Message 4:
Direction: ProgServer->ProgClient
Format: MYSTATS <serialised local stats>
Description: The local database statistics

Message 5:
Direction: ProgClient->ProgServer
Format: GLOBSTATS <serialised global stats>
Description: The global statistics for all databases

Message 6:
Direction: ProgServer->ProgClient
Format: MSETITEMS <items in MSET> \n\
        MSETITEM: <weight> <docid> \n\
	OK
Description: The returned MSet.
