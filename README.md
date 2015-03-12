# callblocker
blocking unwanted calls from your home phone, such as telemarketing, junk, spam, scam, etc. Its
designed to run on small footprint such as a Raspberry Pi. For all those where jcblock or ncid
does not fit or is not enough and Asterisk or FreePBX would be an overkill, as you already have
running VoIP server. callblocker acts as an additional phone, when someone is calling the phone detects the
number and checks it offline via white-and blacklists. There is also the ability to check the number online.

## TODO
1. finish Analog phone support
1. add webinterface
1. support blocking calls without caller ID
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

- Supported VoIP systems (tested):
  - Fritzbox 7390
- Supported analog modems (tested):
  - USRobotics 5637
- Supported online spam checking sites:
  - phonespamfilter.com
  - tellows.de

## Installing on a Raspberry Pi (running raspbian/jessie)
1. sudo apt-get install git automake g++ libpjproject-dev libjson-c-dev libboost-dev libboost-regex-dev python python-beautifulsoup python-demjson
1. git clone https://github.com/pamapa/callblocker.git
1. cd callblocker
1. aclocal
1. automake --add-missing --foreign
1. autoconf
1. ./configure --prefix=/usr --sysconfdir=/etc
1. make all
1. sudo make install
1. cd /etc/callblocker
1. mv tpl_settings.json settings.json
1. adapt settings.json for your needs

## File Layout
When installed on Linux, the following file layout is used
```
/etc/callblocker
  settings.json (configuration)
  blacklists (place your own blacklist here, multiple files are supported)
  whitelists (place your own whitelist here, multiple files are supported)
/usr/bin/callblocker (daemon)
/usr/share/callblocker (scripts)
```

## Documentation settings.json
Start with the provided template settings file (mv tpl_settings.json settings.json)
```json
{ 
  "log_level" : "info",
  "analog" : {
    "phones" : [
      {
        "enabled"      : false,
        "name"         : "Analog Home Phone",
        "country_code" : "+41",
        "block_mode"   : "logging_only",
        "online_check" : "tellows_de",
        "device"       : "/dev/ttyACM0"
      }
    ]
  },
  "sip" : {
    "pjsip_log_level" : 0,
    "accounts" : [
      { 
        "enabled"       : false,
        "name"          : "SIP Home Phone",
        "country_code"  : "+41",
        "block_mode"    : "logging_only",
        "online_check"  : "tellows_de",
        "from_domain"   : "<yourdomain.net>",
        "from_username" : "<your username>",
        "from_password" : "<your password>"
      }
    ]
  },
  "online_credentials" : [
    {
      "name"      : "tellows_de",
      "partner"   : "<your partner name>",
      "apikey"    : "<your api key>"
    }
  ]
}
```
Fields               | Values | Comment
------               | ------ | -------
"log_level"          | "error", "warn", "info" or "debug" |
"country_code"       | +<XYZ> | needed to create international number
"block_mode"         | "logging_only", "whitelists_only", "whitelists_and_blacklists" or "blacklists_only" | "logging_only": number is never blocked, only logged what it would do. "whitelists_only": number has to be in a whitelists (blacklists not used). "whitelists_and_blacklists": number is blocked, when in a blacklists and NOT in a whitelists (default). "blacklists_only": number is blocked, when in a blacklists (whitelists not used)
"online_check"       | | the online check script base name (e.g. "tellows_de" leds to onlinecheck_tellows_de.py
"device"             | | your modem device (get it with dmesg)
"pjsip_log_level"    | 0-5 | pjsip log level, for debugging proposes
"from_domain"        | | your SIP host domain name
"from_username"      | | your SIP username
"from_password"      | | your SIP password
"online_credentials" | | in this section you can define credentials, which are needed by some scripts to get the online information

## Setup
There are two ways to connect the callblock to your phone system, depending if your system is VoIP or analog. 

### Setup via Fritzbox
- Create in the Fritzbox a new IP-phone
  - Open a webbrowser and enter http://fritz.box
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

