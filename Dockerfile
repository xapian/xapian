FROM ubuntu:18.04

RUN apt-get update -y
RUN apt-get upgrade -y
RUN apt-get install sudo
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get install git -y
RUN git clone https://github.com/srinivasyadav18/xapian.git
RUN apt-get install wget curl -y
RUN apt-get install build-essential m4 perl python zlib1g-dev uuid-dev \
wget bison tcl libpcre3-dev libmagic-dev valgrind ccache eatmydata \
doxygen graphviz help2man python-docutils pngcrush python-sphinx \
python3-sphinx mono-devel default-jdk lua5.3 liblua5.3-dev \
php-dev php-cli python-dev python3-dev ruby-dev tcl-dev texinfo -y

RUN wget https://dl.google.com/go/go1.14.linux-amd64.tar.gz ;
RUN tar -C /usr/local -xzf go1.14.linux-amd64.tar.gz
RUN echo "export PATH=$PATH:/usr/local/go/bin " >> ~/.bashrc

#run these commands once you are in
#source ~/.bashrc
#cd xapian && git pull && ./bootstrap && ./configure && make && make install
