#include "process.h"
#include <string.h>
#include "xml++.h"
#include "cvs_db_file.h"

static const string sfilename = "_ChangeLog.xml";
static const string sentry    = "entry";
static const string sfile     = "file";
static const string smsg      = "msg";
static const string sname     = "name";
static const string srevision = "revision";

static void parse_entry (const CXmlNode & rNode, cvs_db_file & rdb_file);
static void parse_file (const CXmlNode & rNode, const CXmlNode & rMsgNode, unsigned int & entryid, cvs_db_file & rdb_file);

void parse_change_log (const string & module, const string & cvs2cl_path, cvs_db_file & rdb_file) 
{
    
    string command = cvs2cl_path + " --xml -f " + sfilename;
    system(command.c_str());
    
    CXmlTree doc(sfilename);

    CXmlNode *pRootNode = doc.root();
    if (pRootNode) {
        CXmlNodeList children = pRootNode->children(sentry);
        CXmlNodeList::const_iterator nitr;
        for (nitr = children.begin(); nitr != children.end(); ++nitr) {
            parse_entry (**nitr, rdb_file);
        }
    }
}


void parse_entry (const CXmlNode & rNode, cvs_db_file & rdb_file) {
    CXmlNodeList   afiles = rNode.children(sfile);
    CXmlNode * pmsg_child = rNode.get_child(smsg);
    if (pmsg_child) {
        unsigned int entryid = 0;
        CXmlNodeList::const_iterator itr;
        for (itr = afiles.begin(); itr!= afiles.end(); ++itr) {
            parse_file (**itr, *pmsg_child, entryid, rdb_file);
        }
    }
}


void parse_file (const CXmlNode & rNode, const CXmlNode & rMsgNode, unsigned int & entryid, cvs_db_file & rdb_file) {
    CXmlNode * pfile_name = rNode.get_child(sname);
    CXmlNode * prevision  = rNode.get_child(srevision);
    if (pfile_name && prevision) {
        unsigned int fileid = 0;
        if (rdb_file.get_fileid(fileid, pfile_name->content()) == 0) {
            if (entryid != 0 || rdb_file.put_comment(entryid, rMsgNode.content()) == 0) {
                rdb_file.put_commit(fileid, prevision->content(), entryid);
            }
        }
    }
}
