% Write Access

We provide write access in one of two ways, either using WebDAV over https or
over ssh (generally you'll be given one sort of access /or/ the other, not
both).  There's a section describing how to set up for each below.

## WebDAV over https

If your username is `fred` then execute the following command:

    svn --username fred ls https://svn-dav.xapian.org:8443/xapian/

The certificate we use for HTTPS is issued by <a href="http://www.cacert.org/"
>CAcert</a> so unless your subversion client is already configured to trust
such certificates you will get the following warning:

    Error validating server certificate for 'https://svn.xapian.org:8443':
     - The certificate hostname does not match.
    Certificate information:
     - Hostname: svn-dav.xapian.org
     - Valid: from Sun, 19 Apr 2009 03:48:43 GMT until Tue, 19 Apr 2011 03:48:43 GMT
     - Issuer: http://www.CAcert.org, CAcert Inc.
     - Fingerprint: 2a:72:69:71:fd:da:dc:97:0c:f5:de:2a:73:8b:f5:78:51:26:ba:72
    (R)eject, accept (t)emporarily or accept (p)ermanently?

If you do, verify the fingerprint is as above.  If the fingerprint
doesn't match, or you have other concerns, talk to the other developers
and don't just accept the certificate.  If you're happy with it, you
can either accept it temporarily (and be asked to confirm it for every
"session", though not every remote svn operation), or permanently.  We
recommend you accept it permanently as otherwise you need to recheck
the fingerprint repeatedly which makes it more likely you might accidentally
accept a spoofed certificate (so press `p`).

You'll then be asked for your password:

    Authentication realm: <https://svn.xapian.org:8443> Xapian Subversion Repository
    Password for 'fred':

Assuming you type your password correctly, you should get a list of the top
level directories in the SVN tree:

    branches/
    tags/
    trunk/

Your password /may/ be cached in plaintext in your home directory -
if your machine isn't well secured, you should ensure that it is being
encrypted, or disable this caching.  See the relevant <a
href="http://subversion.tigris.org/faq.html#plaintext-passwords">Subversion
FAQ entry</a> for more information.

Now you can check out a tree with commit access like so:

    svn co https://svn-dav.xapian.org:8443/xapian/trunk xapian

## ssh

You will need to do a small amount of configuration as we use userv to provide
additional security.

For access on atreus itself, simply create a `[tunnels]` section in your
`~/.subversion/config` file, and add to it the line:

    userv = userv xapian-svn svnserve

Then you can check out a tree with commit access like so:
       
    svn co svn+userv:///xapian/trunk xapian

For remote access via ssh, again create a `[tunnels]` section in your
/local/ `~/.subversion/config` file, but instead add this line:

    ssh+userv=bash -c 'user=${0/@*};host=${0/$user@};ssh $host userv $user svnserve'

Then you can check out a tree with commit access like so:
       
    svn co svn+ssh+userv://xapian-svn@svn.xapian.org/xapian/trunk xapian
