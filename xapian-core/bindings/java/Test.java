import com.muscat.om.*;
import java.util.*;
import java.io.*;

public class Test {
    public static void main (String[] args) throws Throwable {
	OmDatabaseGroup db = new OmDatabaseGroup ();
	//File keysf = new File (args[0], "keyfile");
	//byte[] keybuf = new byte [(int) keysf.length()];
	//new FileInputStream (keysf).read (keybuf);
	
	String dapath       = "/netapp/data/muscat.small.2";
	String type         = "da_flimsy";
	String pquery       = "lemur";
	OmMatchOptions mopt = null;

	for (int i = 0; i < args.length; i++) {
	    if (args[i].charAt (0) == '-') {
		switch (args[i].charAt (1)) {
		case 't':
		    type = args[++i];
		    break;
		case 'd':
  		    dapath = args[++i];
		    break;
		case 'q':
		    pquery = args[++i];
		    break;
		case 'c':
		    mopt = new OmMatchOptions ();
		    mopt.set_collapse_key (0);
		    break;
		default:
		    System.err.println ("huh? option: " + args[i]);
		}
	    }
	}

	db.add_database (type, dapath);
	
	OmEnquire enq = new OmEnquire (db);
	StringTokenizer st = new StringTokenizer (pquery);
	String[] terms = new String[st.countTokens()];
	OmStem stemmer = new OmStem ("porter");
	for (int i = 0; st.hasMoreTokens (); i++) {
	    String term = st.nextToken ();
	    String sterm = stemmer.stem_word (term);
	    terms[i] = sterm;
	}

	//File fmtfile = new File ("./fmt");
	//byte[] buf = new byte [(int) fmtfile.length()];
	//new FileInputStream (fmtfile).read (buf);
	//String fmtstring = new String (buf);

	OmQuery query = new OmQuery ("OR", terms);	
	enq.set_query (query);
	OmMSet mset = enq.get_mset (0, 100, null, mopt, null);
	OmVector items = mset.get_items();
	
	//StringBuffer sbuf = new StringBuffer ();
	//FerretFormatter fmt = new FerretFormatter (enq, mset, fmtstring, sbuf);
	
	System.out.println ("found " + items.size() + " docs\n");
    }
}
