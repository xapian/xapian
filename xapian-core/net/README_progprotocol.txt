This is a brief description of the protocol used by the ProgClient
and ProgServer classes.  They don't currently work like this, but
will soon.

Message 1:
Direction: ProgClient->ProgServer
Format: HELLO!
Description: Greeting message (is abritrary ATM)
When: At database opening time.

Message 2:
Direction: ProgServer->ProgClient
Format: BOO!
Description: Return of greeting (is abritrary ATM)
When: At connection setting up time.

Message 3:
Direction: ProgClient->ProgServer
Format: SETQUERY <weighting type> \
                 "<query string>"
Description: The query, results to get, and weighting formula to use.
When: At prepare_match() time

Message 4:
Direction: ProgServer->ProgClient
Format: MYSTATS <serialised local stats>
Description: The local database statistics

Message 5:
Direction: ProgClient->ProgServer
Format: GLOBSTATS <serialised global stats> \n\
        GETMSET <firstitem> <maxitems>
Description: The global statistics for all databases are sent,
	     and the MSet requested.
When: At get_mset() time.

Message 6:
Direction: ProgServer->ProgClient
Format: MSETITEMS <items in MSET> <max weight>\n\
        MSETITEM: <weight> <docid> \n\
	OK
Description: The returned MSet.
