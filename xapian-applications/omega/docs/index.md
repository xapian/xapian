% Omega

Omega is out-of-the-box search for your website. It comes with a
simple indexer, `omindex`, which can create a Xapian database of the
files on disk that make up your website, and a CGI program [omega](cgiparams) that allows users to search that database. All the details
of that interface are configurable using a built-in language called
[OmegaScript](omegascript).

If `omindex` isn't powerful enough for your application, Omega also
comes with [scriptindex](scriptindex), a more flexible
indexer. Instead of working on your website files directly,
`scriptindex` works over intermediate *input files*. These don't have
to be based on files on disk at all - you can pull them out of a SQL
database (such as using the supplied `dbi2omega` script), for
instance.

Finally, because Omega is built on top of Xapian, if you outgrow its
feature set, you can replace pieces of it with your own code. The only
thing you need to know is Omega's [conventions for term prefixes](termprefixes), which will enable you to create, update and search
Omega databases yourself.

To get started with Omega, either check out the [overview](overview),
or to get things running really fast, try the [quickstart
instructions](quickstart).
