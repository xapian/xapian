# RedHat-style .spec file for Xapian
# RPM Informational headers
#
Summary: Xapian a library that enables developers to use advanced information retrieval techniques in their own projects
Name: xapian-core
Version: 0.4.1
Release: 1
License: GPL
Group: Development/Libraries
URL: http://www.xapian.org/


# The one true source and manuals
#
Source0: http://www.xapian.org/download/distributions/xapian-core-%{version}-cvs.tar.gz
# Patchs 

# Where are we going to build the install set to?
BuildRoot: %{_tmppath}/%{name}-root

# Kill off some old history that we no longer wish to see
#Obsoletes: mod_php, php3, phpfi

# Ok, you wanna build it, you gotta have these packages around
#BuildPrereq: apache-devel >=0.9

%description
Xapian is not an application, but a library that enables developers to use
advanced information retrieval techniques in their own projects. This
makes it an attractive alternative to "packaged" search engines that need
to be extensively hacked in order to use it as a specialty search engine.
 
Features of Xapian include: 
 
* Flexible indexing of documents; 
* Full access to the index: from term to document, but also from document to terms; 
* Good query possibilities, including Boolean and phrase search; 
* Advanced ranking techniques; 
* Relevance feedback. 

%package devel
Group: Development/Libraries
Summary: Files needed for building against xapian
Prereq: xapian-core

%description devel
The xapian-devel package contains the files needed for building against xapian.
You will need this if you develop any packages that require xapian.

%prep
%setup -q -n xapian-core-%{version}-cvs

# Weld the patchs into the main source
#%patch0 -p1

%build

test -f ./configure || ./buildall
./configure
make -j4
echo $RPM_BUILD_ROOT

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT

# First, install the CGI tree.
#
make install-strip prefix=$RPM_BUILD_ROOT%{_prefix}
# move the docs to the right place
mkdir -p $RPM_BUILD_ROOT%{_prefix}/share/doc
mv $RPM_BUILD_ROOT%{_prefix}/share/xapian $RPM_BUILD_ROOT%{_prefix}/share/doc/

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/usr/lib/*
/usr/bin/*

#	%doc CODING_STANDARDS CREDITS EXTENSIONS INSTALL LICENSE NEWS README*
#	%doc Zend/ZEND_*
#	%config(noreplace) %{_sysconfdir}/php.ini
#	%{_bindir}/php
#	%{_datadir}/pear
#	%{_libdir}/apache/libphp4.so


%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files devel
%defattr(-,root,root)
%docdir /usr/share/doc/xapian
/usr/share/doc/xapian
%{_includedir}/om/
/usr/share/aclocal/*

%changelog
* Wed May 14 2002  Sam Liddicott <sam@ananova.com>
  Based on a Redhat php .spec file
