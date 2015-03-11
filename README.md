# callblocker
blocking unwanted calls from your home phone

## TODO
1. finish Analog phone support
1. add webinterface
1. support calls without caller ID
1. night silence
1. shield support?
1. polish

## Features
- automatically blocks unwanted incoming calls
- logging of all incoming calls
- different kind of block modes:
  - logging only
  - number blocking using blacklists
  - avoid blocking, if number is in whitelist
  - number has to be in whitelist
- up to date blacklists available through the Internet
- callblocker has been designed to run on a Raspberry Pi

## Installing on a Raspberry Pi (running raspbian/jessie)
1. sudo apt-get install git automake g++ libpjproject-dev libjson-c-dev
1. git clone https://github.com/pamapa/callblocker.git
1. cd callblocker
1. aclocal
1. automake --add-missing --foreign
1. autoconf
1. ./configure --prefix=
1. make all
1. sudo make install

## Setup
There are two ways to connect the callblock to your phone system, depending if your system is VoIP or analog. 
The callblocker acts as an additional phone, when someone is calling the phone detects the
number and checks it offline via white-and blacklists. There is also the ability to check the number online.

- Supported VoIP systems (tested):
  - Fritzbox 7390
- Supported analog modems (tested):
  - USRobotics 5637

### Setup via Fritzbox
- Create in the Fritzbox a new IP-phone
  - http://fritz.box
  - In the menu "Telefonie -> Telefoniegeräte" click on "Neues Gerät einrichten"
  - Choose "Telefon (mit und ohne Anrufbeantworter)" and click "Weiter"
  - Choose "LAN/WLAN (IP-Telefon)", for name use for example "callblocker" and click "Weiter"
  - Choose a password, remember the username and click "Weiter"
  - Choose "alle Anrufe annehmen" and click "Weiter"
- Setup the Ip-phone in the callblocker configuration (/etc/callblocker/setting.json):
  - sip -> accounts
  - "from_domain"   : "fritz.box"
  - "from_username" : "your username"
  - "from_password" : "your password"
  - make sure the account is enabled and the other fields ok ok for you

### Setup via Analog phone
- Implementation is not yet finished...

