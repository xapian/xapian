/* queryserver.cpp: A simple, UNIX socket based, queryserver.
 *
 * ----START-LICENCE----
 * Copyright 2004 Studio 24 Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include "config.h"
#include <xapian.h>
#include <xapian/queryparser.h>

#include <string>
#include <vector>
#include <map>
#include <utility>

#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>

#include "base64.h"

#ifndef UNIX_PATH_MAX
/* Correct value on linux 2.4*/
#define UNIX_PATH_MAX 108
#endif

/** Path to socket */
const char * socket_path;

/** Directory holding databases */
std::string dbdir;

/** Ensure that all processes in this process group are terminated on SIGTERM.
 */
extern "C" void
on_SIGTERM(int)
{
    unlink(socket_path);
    signal(SIGTERM, SIG_DFL);
    kill(0, SIGTERM);
    exit(0);
}

/** Ensure that all processes in this process group are terminated on SIGINT.
 */
extern "C" void
on_SIGINT(int)
{
    unlink(socket_path);
    signal(SIGINT, SIG_DFL);
    kill(0, SIGTERM);
    exit(0);
}

/** Wait for all child process which have exited.
 */
extern "C" void
on_SIGCHLD(int)
{
    int status;
    /* There may be one, several or no children which have exited.  The
     * while loop ensures that all of them are waited for. */
    while (waitpid(-1, &status, WNOHANG) > 0);
}

// Construct a new string using printf format and arg list
static std::string
string_printf(const char* format, ... )
{
    int bufsz = 1024;
    char* buf = NULL;
    va_list arg_list;

    for (;;)
    {
        buf = new char [bufsz];
        if (buf == NULL)
            throw "string_printf: error allocating memory";

        va_start (arg_list, format);
        int sz = vsnprintf (buf, bufsz, format, arg_list);
        va_end (arg_list);

        // if the buffer was big enough, finish and return
        if (sz > -1 && sz < bufsz)
        {
            std::string ret (buf, sz);
            delete[] buf;
            return ret;
        }

        // double the buffer size
        delete[] buf;
        buf = NULL;
        bufsz *= 2;
    }
}

/** Write a string to a socket.
 */
static void
write_to_socket(int sock, const std::string & value)
{
    ssize_t written;
    ssize_t pos = 0;
    do {
        // The socket may be full - wait for it to be able to accept a write.
        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(sock, &fdset);

        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        int retval = select(sock + 1, 0, &fdset, &fdset, &tv);


        written = write(sock, value.c_str() + pos, value.size() - pos);
        if (written < 0) {
            fprintf(stderr, "Error: unable to write result: `%s'\n", strerror(errno));
            return;
        }
        if (written == 0) {
            fprintf(stderr, "Error: unable to write result - no bytes transferred\n");
            return;
        }
    } while(pos + written < value.size());
}

static int
process_line(std::vector<std::pair<std::string, std::string> > & keyvals,
             const std::string curline)
{
    std::string::size_type eq = curline.find(':');
    if (eq == std::string::npos) {
        if (keyvals.empty()) {
            fprintf(stderr, "Error: Invalid first line of request `%s'- no ':'\n",
                    curline.c_str());
            return -1;
        } else {
            keyvals.back().second += curline;
        }
    } else {
        std::string key = curline.substr(0,eq);
        std::string value = curline.substr(eq + 1);
        keyvals.push_back(std::pair<std::string, std::string>(key, value));
    }
    return 0;
}

#define INBUF_SIZE 4096
static int
read_request(int sock,
             std::vector<std::pair<std::string, std::string> > & keyvals)
{
    char inbuf[INBUF_SIZE + 1];
    std::string curline;
    bool request_received = false;

    while(!request_received) {
        ssize_t received = read(sock, inbuf, INBUF_SIZE);
        if (received < 0) {
            if (errno == EAGAIN) {
                continue;
            }
            fprintf(stderr, "Error: unable to read from client: `%s'\n", strerror(errno));
            return -1;
        } else if (received == 0) {
            // EOF
            break;
        } else {
            ssize_t pos = 0;
            ssize_t linestart = 0;
            for (pos = 0; pos != received; pos++)
            {
                if(inbuf[pos] == '\n') {
                    if (pos != 0)
                        curline += std::string(inbuf + linestart,
                                               pos - linestart);
                    if (curline.length() == 0) {
                        // A blank line is used to end the request.
                        request_received = true;
                        if (pos + 1 != received) {
                            inbuf[received] = '\0';
                            fprintf(stderr, "Warning: unexpected characters received after end of request: `%s'\n", inbuf + pos + 1);
                        }
                        break;
                    }
                    if (process_line(keyvals, curline)) return -1;
                    curline.erase();
                    linestart = pos + 1;
                }
            }
            if (!request_received && received - linestart > 0)
                curline += std::string(inbuf + linestart, received - linestart);
        }
    }
    if (!curline.empty())
        return process_line(keyvals, curline);
    return 0;
}

static void
serve_client(int sock)
{
    std::vector<std::pair<std::string, std::string> > keyvals;

    if (read_request(sock, keyvals)) {
        return;
    }

    // Parse request
    std::string dbname = "default";
    Xapian::Query probquery;
    Xapian::QueryParser qp;
    std::string prefix;
    std::map<std::string, Xapian::Query> filters;
    Xapian::doccount firstdoc = 0;
    Xapian::doccount maxitems = 10;
    Xapian::valueno collapsekey = 0;
    bool have_collapsekey = false;

    std::vector<std::pair<std::string, std::string> >::const_iterator i;
    for (i = keyvals.begin(); i != keyvals.end(); i++) {
        //fprintf(stderr, "key:`%s',value:`%s'\n", i->first.c_str(), i->second.c_str());

        if (i->first == "firstdoc") {
            firstdoc = atoi(i->second.c_str());
        } else if (i->first == "maxitems") {
            maxitems = atoi(i->second.c_str());
        } else if (i->first == "collapsekey") {
            collapsekey = atoi(i->second.c_str());
            have_collapsekey = true;
        } else {
            std::string decoded_value;
            try {
                decoded_value = Base64::decode(i->second.c_str(), i->second.size());
            } catch(...) {
                fprintf(stderr, "Error: Invalid request value for key `%s': `%s'- bad base64\n",
                        i->first.c_str(), i->second.c_str());
                return;
            }
            if (i->first == "db") {
                // Check for possibly insecure database names.
                if (decoded_value.find("..") != std::string::npos ||
                    decoded_value.find("/") != std::string::npos) 
                {
                    fprintf(stderr, "Error: potentially insecure database name passed: `%s'\n",
                            decoded_value.c_str());
                    return;
                }
                dbname = decoded_value;
            } else if (i->first == "query") {
                try {
                    probquery = qp.parse_query(decoded_value);
                } catch(...) {
                    fprintf(stderr, "Error: cannot parse query: `%s'\n",
                            decoded_value.c_str());
                    return;
                }
            } else if (i->first == "prefix") {
                prefix = decoded_value;
            } else if (i->first == "filter") {
                Xapian::Query q(prefix + decoded_value);
                std::map<std::string, Xapian::Query>::iterator i;
                i = filters.find(prefix);
                if (i == filters.end()) {
                    // New category.
                    filters[prefix] = q;
                } else {
                    // OR this filter with other filters in the same category.
                    Xapian::Query combined(Xapian::Query::OP_OR, i->second, q);
                    filters[prefix] = combined;
                }
            }
        }
    }

    Xapian::Query filterquery;
    std::map<std::string, Xapian::Query>::const_iterator filteriter;
    for (filteriter = filters.begin();
         filteriter != filters.end();
         filteriter++)
    {
        if (filterquery.is_empty()) {
            filterquery = filteriter->second;
        } else {
            filterquery = Xapian::Query(Xapian::Query::OP_AND, filterquery,
                                        filteriter->second);
        }
    }

    Xapian::Query query;
    if (filterquery.is_empty()) {
        query = probquery;
    } else {
        if (probquery.is_empty()) {
            query = filterquery;
        } else {
            query = Xapian::Query(Xapian::Query::OP_FILTER,
                                  probquery, filterquery);
        }
    }

    Xapian::Database db = Xapian::Auto::open(dbdir + "/" + dbname);
    Xapian::Enquire enquire(db);
    enquire.set_query(query);
    if (have_collapsekey) {
        enquire.set_collapse_key(collapsekey);
    }
    Xapian::MSet results = enquire.get_mset(firstdoc, maxitems);
    results.fetch();
    
    std::string resultstr;

    try {
        resultstr += string_printf("firstitem:%d\n", results.get_firstitem());
        resultstr += string_printf("matches_lower_bound:%d\n",
                                   results.get_matches_lower_bound());
        resultstr += string_printf("matches_estimated:%d\n",
                                   results.get_matches_estimated());
        resultstr += string_printf("matches_upper_bound:%d\n",
                                   results.get_matches_upper_bound());
        resultstr += string_printf("items:%d\n", results.size());

        Xapian::MSetIterator iter;
        for (iter = results.begin(); iter != results.end(); iter++)
        {
            resultstr += "#\n";
            resultstr += string_printf("docid:%d\n", *iter);
            resultstr += string_printf("rank:%d\n", iter.get_rank());
            resultstr += string_printf("percent:%d\n", iter.get_percent());
            //Xapian::Document doc = iter.get_document();
            Xapian::Document doc = db.get_document(*iter);
            resultstr += "data:";
            resultstr += Base64::encode(doc.get_data().c_str(), doc.get_data().size());
            resultstr += "\n";
        }
        if (results.begin() != results.end()) 
            resultstr += "#\n";
    } catch(...) {
        fprintf(stderr, "Error: unable to format results.\n");
        return;
    }
    write_to_socket(sock, resultstr);
}
    
/** Start the server.
 */
static int
start_server()
{
    struct sockaddr_un sockaddr;
    sockaddr.sun_family = AF_UNIX;
    strncpy(sockaddr.sun_path, socket_path, UNIX_PATH_MAX);

    struct stat statbuf;
    int ret = lstat(socket_path, &statbuf);
    if (ret != -1 && !S_ISSOCK(statbuf.st_mode)) {
        fprintf(stderr,
                "A file already exists at `%s', but it is not a socket\n",
                socket_path);
        return EXIT_FAILURE;
    }
    if (ret == -1 && errno != ENOENT) {
        fprintf(stderr, "Can't stat `%s': `%s'\n", socket_path, strerror(errno));
        return EXIT_FAILURE;
    }

    int serversock = socket(PF_UNIX, SOCK_STREAM, 0);
    if (serversock == -1) {
        fprintf(stderr, "Couldn't create UNIX socket: `%s'\n", strerror(errno));
        return EXIT_FAILURE;
    }

    ret = bind(serversock, (struct sockaddr *)(&sockaddr), sizeof(sockaddr));
    if (ret == -1) {
        fprintf(stderr, "Couldn't bind socket to `%s': `%s'\n",
                socket_path, strerror(errno));
        return EXIT_FAILURE;
    }

    ret = chmod(socket_path, 0666);
    if (ret == -1) {
        fprintf(stderr,
                "Warning: couldn't set permissions on socket `%s': `%s'\n",
                socket_path, strerror(errno));
    }

    /* Allow 5 queue to grow to at most 5 requests.  Should be plenty since we
     * simply fork for each request.*/
    ret = listen(serversock, 5);
    if (ret == -1) {
        fprintf(stderr, "Couldn't listen on socket `%s': `%s'\n",
                socket_path, strerror(errno));
        return EXIT_FAILURE;
    }

    /* The SIGTERM and SIGINT signal handlers cause the socket to be removed
     * from the filesystem, so they mustn't be set earlier in this file (where
     * we havn't yet checked that the socket filename doesn't point to a file
     * we don't wish to overwrite. */
    signal(SIGCHLD, on_SIGCHLD);
    signal(SIGTERM, on_SIGTERM);
    signal(SIGINT, on_SIGINT);
    while(true) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(serversock, &rfds);
        ret = select(serversock + 1, &rfds, NULL, NULL, NULL);
        if (ret == -1) {
            if (errno == EINTR) continue;
            fprintf(stderr, "Error in select: `%s'\n",
                    strerror(errno));
            break;
        }
        if (ret == 0)
            continue;

        struct sockaddr_un remote_addr;
        socklen_t remote_addr_size = sizeof(remote_addr);
        int con_socket = accept(serversock,
                                (struct sockaddr *)(&remote_addr),
                                &remote_addr_size);
        if (con_socket == -1) {
            fprintf(stderr, "Warning: Unable to accept a connection: `%s'\n",
                    strerror(errno));
            continue;
        }

        int pid = fork();
        if (pid == 0) {
            // child
            close(serversock);
            try {
                serve_client(con_socket);
            } catch(Xapian::Error & e) {
                fprintf(stderr, "Error executing query: `%s'\n", e.get_msg().c_str());
            }
            close(con_socket);
            exit(0);
        } else if (pid > 0) {
            // parent
            close(con_socket);
        } else {
            // fork failed
            fprintf(stderr, "Warning: Unable to fork: `%s'\n", strerror(errno));
            close(con_socket);
        }
    }
    unlink(socket_path);
    return EXIT_FAILURE;
}

/** Main function.  Starts the queryserver.
 */
extern int
main(int argc, char ** argv)
{
    if (argc != 3) {
        fprintf(stderr, "Usage: queryserver socketpath dbdir\n");
        return EXIT_FAILURE;
    }

    socket_path = argv[1];
    if (strlen(socket_path) > UNIX_PATH_MAX) {
        fprintf(stderr, "pathname too long - maximum length is %d characters\n", UNIX_PATH_MAX);
        return EXIT_FAILURE;
    }

    dbdir = argv[2];
    return start_server();
}
