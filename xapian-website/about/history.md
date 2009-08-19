% History

Xapian is a modern class library, but has evolved out of more than 25 years of
commercial and academic experience.

Xapian is partly derived from the Open Muscat engine, developed by BrightStation
PLC and released under the
<a href="http://www.opensource.org/licenses/gpl-license.php">GPL</a>.
Open Muscat was built to be a replacement for
the proprietary Muscat 3.6 information retrieval system, which was written
almost entirely in <a href="http://en.wikipedia.org/wiki/BCPL">BCPL</a>,
and becoming hard to extend in the ways they wanted.

Muscat was originally written by
<a href="http://en.wikipedia.org/wiki/Martin_Porter">Dr. Martin Porter</a>
at Cambridge University.  In 1984, Martin and John Snyder founded Cambridge CD
Publishing
to commercially exploit the technology; the company was soon renamed
Muscat Ltd when the focus shifted from CDs to the web.  Muscat Ltd was
bought by Maid PLC, who renamed themselves first to Dialog Corporation, and
then to BrightStation PLC (when they sold the Dialog brand and content
to Thomson).

In early 2001, BrightStation's management renamed Open Muscat to Omsee, and
shortly afterwards they sadly announced they were taking development
closed-source.

A number of developers (both former BrightStation employees and others from the
fledgling Open Muscat community) took the last GPLed version and have been
continuing development under the GPL.  This project was initially known as
OmSeek, but BrightStation complained this was too close to their (untrademarked
and unpublicised) name Omsee.  It was simpler to change it than to argue, and
the name Xapian was chosen.

The last numbered BrightStation release was Open Muscat 0.4.1, although there
was extensive development in CVS after that.  The first official Xapian release
was 0.5.0.

Back in 2000, Open Muscat (using the muscat36 backend) was the retrieval
engine which powered BrightStation's Webtop search engine
(<a href="http://web.archive.org/web/20000708023501/http://www.webtop.com/index.html">archive.org snapshot</a>),
which offered a sub-second search over around
500 million web pages (around 1.5 terabytes of database `files).
The whole search engine, including the web crawlers
and index building, ran on 30 Intel boxes.  Webtop also incorporated
technology from Muscat Ltd's EuroFerret search engine.
