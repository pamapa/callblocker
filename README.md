# callblocker
blocking unwanted calls from your home phone

## TODO
1. finish SIP support
1. add Analog phone support
1. add webinterface
1. polish

## Installing on a Raspberry Pi (running raspbian jessie)
1. sudo apt-get install git automake g++ libpjproject-dev libjson-c-dev
1. git clone https://github.com/pamapa/callblocker.git
1. cd callblocker
1. aclocal
1. automake --add-missing --foreign
1. autoconf
1. ./configure --prefix=/
1. make all
1. sudo make install

