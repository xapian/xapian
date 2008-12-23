Basic instructions
==================

1. Use omindex to build an index of your website::

    $ omindex --db DBPATH --url / WEBPATH

  Where DBPATH is where you want the database, such as
  /var/lib/omega/data/default and WEBPATH is the directory containing
  all your web documents - e.g. /var/www.

  To start off with, it is advisable to have the final directory of the
  database be called 'default'.  This is what Omega expects, although it
  can be changed once you've got things going.

2. Edit omega.conf:

  This contains 3 settings, written one per line as "<SETTING> <VALUE>":

  * database_dir - this should point to the directory containing your
    database(s), for example /var/lib/omega/data (this should contain a
    database called 'default' which is the database you indexed to above).

  * template_dir - the directory where the OmegaScript templates are, for
    example /var/lib/omega/templates (this should contain an OmegaScript
    template called 'query' which is used by default).

  * log_dir - the directory which the OmegaScript $log command writes log files
    to, for example /var/log/omega .

3. Test omega from the command line::

    $ /usr/lib/omega/bin/omega 'P=my search terms' HITSPERPAGE=10

  (The path to omega may be different in your installation.)

  This will output quite a lot of HTML.  Normally you use omega via CGI, but it
  also has this test mode which is useful for checking that everything works
  independent of your webserver configuration.  To actually use omega, you
  should install it to run via CGI by copying or linking the omega executable
  into your cgi-bin directory.

  For more information, see the `overview <overview.html>`_ document.  There
  are other documents covering the `CGI parameters <cgiparams.html>`_ which
  omega accepts, and the `OmegaScript <omegascript.html>`_ language used to
  control the format of omega's output.
