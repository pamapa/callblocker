# callblocker
blocking unwanted calls from your home phone, such as telemarketing, junk, spam, scam, etc. Its
designed to run on small footprint such as a Raspberry Pi. For all those where jcblock or ncid
does not fit or is not enough and Asterisk or FreePBX would be an overkill, as you already have
running VoIP server. callblocker acts as an additional phone, when someone is calling the phone detects the
number and checks it offline via white-and blacklists. There is also the ability to check the number online.


## TODO
1. finish webinterface
1. polish, bugfixing


## Features
- automatically blocks unwanted incoming calls
- logging of all incoming calls
- different kind of block modes.
  - logging only
  - number blocking using blacklists
  - avoid blocking, if number is in whitelist
  - number has to be in whitelist
- daily refresh blacklists through the Internet
- support of online spam check sites, for spam verification
- support of online lookup sites, to find out who is calling

Supported VoIP systems (tested):
- Fritzbox 7390

Supported analog modems (tested):
- USRobotics 5637


## Install daemon on a Raspberry Pi (running raspbian/jessie)
```bash
sudo apt-get install git automake g++ libpjproject-dev libjson-c-dev libboost-dev libboost-regex-dev python python-beautifulsoup python-demjson
git clone https://github.com/pamapa/callblocker.git
cd callblocker
aclocal
automake --add-missing --foreign
autoconf
./configure --prefix=/usr --sysconfdir=/etc
make all
sudo make install
cd /etc/callblocker
sudo mv tpl_settings.json settings.json
sudo joe settings.json
sudo systemctl start callblockerd.service
```


## Install WebUI on a Raspberry Pi (running raspbian/jessie)
```bash
sudo apt-get install lighttpd php5-common php5-cgi php5 libjs-dojo-core libjs-dojo-dijit libjs-dojo-dojox
sudo chgrp -R www-data /etc/callblocker/
sudo usermod -a -G systemd-journal www-data
sudo joe /etc/lighttpd/lighttpd.conf
```
1. In the upper section of this file you can find den section 'server.modules='. Please add this line: '"mod_fastcgi",'.
2. At the end of file, add: 'fastcgi.server = (".php"=>(("bin-path"=>"/usr/bin/php-cgi", "socket"=>"/tmp/php.sock")))'.
3. Make server.document-root point to "/usr/var/www/callblocker"
```bash
sudo reboot
```


## File Layout
When installed on Linux, the following file layout is used
```
/etc/callblocker
  settings.json (configuration)
  blacklists (place your own blacklists here)
  whitelists (place your own whitelists here)
/usr/bin/callblockerd (daemon)
/usr/share/callblocker (scripts)
/usr/var/www/callblocker (web interface)
```


## <a name="settingsJson"></a> Documentation of settings.json
Start with the provided template settings file (sudo mv tpl_settings.json settings.json)
```json
{ 
  "log_level" : "info",
  "pjsip_log_level" : 0,
  "phones" : [
    {
      "enabled"           : false,
      "name"              : "Analog Home Phone",
      "country_code"      : "+41",
      "block_mode"        : "logging_only",
      "block_unknown_cid" : false,
      "online_check"      : "tellows_de",
      "online_lookup"     : "tel_search_ch",
      "device"            : "/dev/ttyACM0"
    },
    { 
      "enabled"           : false,
      "name"              : "SIP Home Phone",
      "country_code"      : "+41",
      "block_mode"        : "whitelists_and_blacklists",
      "block_unknown_cid" : false,
      "online_check"      : "tellows_de",
      "online_lookup"     : "tel_search_ch",
      "from_domain"       : "<your domain>",
      "from_username"     : "<your username>",
      "from_password"     : "<your password>"
    }
  ],
  "online_credentials" : [
    {
      "name"     : "tellows_de",
      "username" : "<your partner name>",
      "password" : "<your api key>"
    },
    {
      "name"     : "whocalled_us",
      "username" : "<your name>",
      "password" : "<your pass>"
    }
  ]
}
```
Fields               | Values | Description
------               | ------ | -------
"log_level"          | "error", "warn", "info" or "debug" | Logging level. Default is "info".
"pjsip_log_level"    | 0-5 | Logging level of the pjsip library, for debugging proposes. Default is 0.
"country_code"       | `+<X[Y][Z]>` | Your international country code (e.g. +33 for France)
"block_mode"         | "logging_only", "whitelists_only", "whitelists_and_blacklists" or "blacklists_only" | "logging_only": number is never blocked, only logged what it would do. "whitelists_only": number has to be in a whitelists (blacklists not used). "whitelists_and_blacklists": number is blocked, when in a blacklists and NOT in a whitelists (default). "blacklists_only": number is blocked, when in a blacklists. (whitelists not used)
"block_unknown_cid"  | true, false | optional: You can decide to block all calls that come to your system with a blocked/unknown caller ID. Default is false.
"online_check"       | [values](#onlineCheck)  | optional: online check site to verify if number is spam
"online_lookup"      | [values](#onlineLookup)  | optional: online lookup site, to see who is calling
"device"             | | Your device name (get it with dmesg). Usually its "/dev/ttyACM0".
"from_domain"        | | Your SIP domain name
"from_username"      | | Your SIP username
"from_password"      | | Your SIP password
"online_credentials" | | In this section you can define credentials, which are needed by some scripts to get the online information.


## <a name="onlineCheck"></a> Online check option
Name                  | Site                           | Description
----                  | ----                           | -----------
""                    | No online check is done        |
"phonespamfilter_com" | http://www.phonespamfilter.com | Free for non comercial use
"whocalled_us"        | http://whocalled.us            | Mostly USA and Canada (+1). Free, but needs login
"tellows_de"          | http://tellows.de              | Not free

The online check script base name e.g. "tellows_de" leds to onlinecheck_tellows_de.py.


## <a name="onlineLookup"></a> Online lookup option
Name                  | Site                           | Description
----                  | ----                           | -----------
""                    | No online lookup is done       |
"tel_search_ch"       | http://tel.search.ch           | Switzerland (+41). Free for non comercial use

Developer hint: The online lookup script base name e.g. "tel_search_ch" leds to onlinelookup_tel_search_ch.py.


## Automatically download blacklists
Currently the following blacklists are supported:

Name                         | Site                       | Description
----                         | ----                       | -----------
blacklist_toastedspam_com.py | http://www.toastedspam.com | Mostly USA and Canada (+1)
blacklist_ktipp_ch.py        | https://www.ktipp.ch       | Switzerland (+41)

There is a possibility to daily download a whole blacklist. You will need to setup a cronjob for this task. The following cronjob will download each day the blacklist provided by ktipp_ch:
```
0 0 * * * /usr/share/callblocker/blacklist_ktipp_ch.py --output /etc/callblocker/blacklists/ >/dev/null
```


## Setup
There are two ways to connect the callblock to your phone system, depending if your system is VoIP or analog. 


### Setup using Fritzbox with a IP-phone
- Create in the Fritzbox a new IP-phone
  - Open your webbrowser and go to the URL http://fritz.box
  - In the menu "Telefonie -> Telefoniegeraete" click on "Neues Geraet einrichten"
  - Choose "Telefon (mit und ohne Anrufbeantworter)" and click "Weiter"
  - Choose "LAN/WLAN (IP-Telefon)", for name use for example "callblocker" and click "Weiter"
  - Choose a password, remember the username and click "Weiter"
  - Choose "alle Anrufe annehmen" and click "Weiter"
- Setup the IP-phone in the callblocker configuration (/etc/callblocker/setting.json):
  - Edit the section sip -> accounts
  - "from_domain"   : "fritz.box"
  - "from_username" : "your username"
  - "from_password" : "your password"
  - Make sure the account is enabled and the other fields ok ok for you


### Setup using an Analog phone
- Attach the USB modem to the Raspberry Pi
- Use `dmesg` to find the device name `/dev/<name>`
- Setup the Analog phone in the callblocker configuration (/etc/callblocker/setting.json):
  - Edit the section analog -> phones
  - "device" : "your device name"
  - Make sure the account is enabled and the other fields ok ok for you

