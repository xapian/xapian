# -*- mode: ruby -*-
#
# Experimental: use Vagrant & VirtualBox to get a dev environment for Xapian,
# with all packages installed and the source bootstrapped and configured.
# Takes a long while on a slow connection.

$packages = <<END
set -e
echo 'Updating package list.'
apt-get -qq update
echo 'Installing required packages (can take a long while the first time).'
apt-get -yqq install git build-essential m4 perl python zlib1g-dev uuid-dev wget bison tcl libpcre3-dev libmagic-dev valgrind ccache eatmydata doxygen graphviz ghostscript texlive-latex-base texlive-extra-utils texlive-binaries texlive-fonts-extra texlive-fonts-recommended texlive-latex-extra texlive-latex-recommended help2man python-docutils pngcrush python-sphinx python3-sphinx mono-devel default-jdk lua5.2 liblua5.2-dev php-dev php-cli python-dev python3-dev ruby-dev tcl-dev texinfo
echo 'Upgrading existing packages as needed.'
apt-get -yqq upgrade
END

$bootstrap = <<END
set -e
echo 'Bootstrapping.'
if [ ! -d /home/vagrant/build ]; then rm -rf /home/vagrant/build && mkdir /home/vagrant/build; fi
if [ ! -d /home/vagrant/install ]; then rm -rf /home/vagrant/install && mkdir /home/vagrant/install; fi
cd /home/vagrant/build
/vagrant/bootstrap
END

$configure = <<END
set -e
echo 'Configuring Xapian for build.'
cd /home/vagrant/build
/vagrant/configure --prefix=/home/vagrant/install
echo
echo 'Use `vagrant ssh` to get onto the machine, and `cd build && make`.'
END

Vagrant.configure("2") do |config|
  config.vm.hostname = "xapian"
  config.vm.box      = "ubuntu/bionic64"

  config.vm.provision "shell", inline: $packages
  config.vm.provision "shell", inline: $bootstrap, privileged: false
  config.vm.provision "shell", inline: $configure, privileged: false
end
