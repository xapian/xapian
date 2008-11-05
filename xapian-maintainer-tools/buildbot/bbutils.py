# -*- python -*-
# ex: set syntax=python:

# Utility functions to help setting up the buildbot

import os.path

class ConfigError(RuntimeError):
    def __init__(self, msg):
        RuntimeError.__init__(self, msg)

class BuildBotConfig(object):
    def __init__(self, projectName, projectURL):
        """Initialise the configuration.

        The 'projectName' string will be used to describe the project that this
        buildbot is working on. For example, it is used as the title of the
        waterfall HTML page. The 'projectURL' string will be used to provide a
        link from buildbot HTML pages to your project's home page.

        """
        self.c = {}
        self.c['projectName'] = projectName
        self.c['projectURL'] = projectURL
        self.c['bots'] = []
        self.c['sources'] = []
        self.c['builders'] = []
        self.c['schedulers'] = []
        self.c['status'] = []

        self.scheduler_builders = {}
        self.sched_order = []

    def __getitem__(self, key):
        return self.c[key]

    def __setitem__(self, key, value):
        self.c[key] = value

    def finalise(self):
        """Finalise the configuration.

        This must be called to make the configuration ready-to-use.

        """
        self._create_schedulers()

    def add_bot(self, botname, password=None):
        """Add a bot to the configuration.

        If password is not supplied, the password will be looked for in a
        module called "private_passwords", in a dictionary called "passwords".

        """
        if password is None:
            try:
                import private_passwords
                password = private_passwords.passwords[botname]
            except ImportError:
                raise ConfigError("Missing password file: can't get password for bot %r" % botname)
            except AttributeError:
                raise ConfigError("Missing password dictionary in password file")
            except KeyError:
                raise ConfigError("Missing password for bot %r" % botname)
        self.c['bots'].append((botname, password))

    def set_slave_portnum(self, portnum):
        """Set the TCP port to listen for slaves on.

        This must match the value configured into the buildslaves (with their
        --master option)

        """
        self.c['slavePortnum'] = 9989

    def add_source(self, source):
        """Add a change source.

        """
        self.c['sources'].append(source)

    def addScheduler(self, name, **kwargs):
        """Add a scheduler.

        `name` is the name of the scheduler, used in status displays.

        """
        if name in self.scheduler_builders:
            raise ConfigError("Duplicate scheduler name: %r" % name)
        self.scheduler_builders[name] = [kwargs]
        self.sched_order.append(name)

    def _create_schedulers(self):
        """Create all the schedulers, and add them to the config.

        """
        from buildbot.scheduler import Scheduler, Dependent
        scheds = {}
        for name in self.sched_order:
            item = self.scheduler_builders[name]
            kwargs = item[0]
            builders = item[1:]
            if "depends" in kwargs:
                upstream = kwargs['depends']
                if upstream not in scheds:
                    raise ConfigError("Scheduler name %r used in depends before being defined" % name)
                upstream = scheds[upstream]
                sched = Dependent(name, upstream, builders)
            else:
                sched = Scheduler(name=name,
                                  builderNames=builders,
                                  **kwargs)
            scheds[name] = sched
            self.c['schedulers'].append(sched)

    def addBuilder(self, name, factory, slavenames, scheduler):
        """Add a builder.

        `scheduler` is the name of the scheduler to listen to.  The scheduler
        specified must have previously been added to the list of schedulers
        with addScheduler().

        """
        if isinstance(slavenames, basestring):
            slavenames = [slavenames, ]
        self.c['builders'].append({
            'name': name,
            'builddir': name,
            'factory': factory,
            'slavenames': slavenames
        })
        if scheduler not in self.scheduler_builders:
            raise ConfigError("Unknown scheduler name: %r" % name)
        self.scheduler_builders[scheduler].append(name)

    def add_status(self, status):
        """Add a status target.

        """
        self.c['status'].append(status)

    def add_status_html_waterfall(self,
                                  hostname=None,
                                  http_port=8010,
                                  public_port=80,
                                  **kwargs):
        """Add an HTML Waterfall status target.

        The 'hostname' string should be the host on which the internal web
        server is publically visible.

        'public_port' should be the port number on which the internal web
        server is publically visible (this may differ from http_port if there
        is some proxying in the middle).  If None, http_port will be used.

        """
        if public_port is None:
            public_port = http_port
        kwargs['http_port'] = http_port
        kwargs['favicon'] = \
            os.path.abspath(os.path.join(os.path.dirname(__file__),
                                         "buildbot.png"))

        import html
        self.add_status(html.Waterfall(**kwargs))
        self.c['buildbotURL'] = "http://%s:%d/" % (hostname, public_port)

    def add_status_mail(self, fromaddr=None, extraRecipients=None,
                        sendToInterestedUsers=False, **kwargs):
        """Add an email status target.

        """
        kwargs['fromaddr'] = fromaddr
        kwargs['extraRecipients'] = extraRecipients
        kwargs['sendToInterestedUsers'] = sendToInterestedUsers
        from buildbot.status import mail
        self.c['status'].append(mail.MailNotifier(**kwargs))

    def add_status_irc(self, host=None, nick=None, channels=None,
                       password=None, **kwargs):
        """Add an IRC status target.

        """
        kwargs['host'] = host
        kwargs['nick'] = nick
        kwargs['channels'] = channels

        if password is None:
            try:
                import private_passwords
                password = private_passwords.irc_passwords[nick]
            except ImportError:
                raise ConfigError("Missing password file: can't get password for IRC bot %r" % nick)
            except AttributeError:
                raise ConfigError("Missing irc_password dictionary in password file")
            except KeyError:
                raise ConfigError("Missing password for IRC bot %r" % nick)
        if password is not None:
            kwargs['password'] = password

        from buildbot.status import words
        self.c['status'].append(words.IRC(**kwargs))
