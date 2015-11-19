# callblocker
blocking unwanted calls from your home phone, such as telemarketing, junk, spam, scam, etc.

The callblock acts like a normal phone. No additional telephone switchboard (like FreePBX or Asterisk) is needed. When a telemarketer is calling, the callblock picks up the phone and automatically hangs up after a few seconds. Like this the call is blocked. It's designed to run on small footprints such as a Raspberry Pi. It has the ability to check against online spam check sites and it also supports offline blacklists.


## Features
- automatically blocks unwanted incoming calls
- logging of all incoming calls
- different kind of block modes.
  - logging only
  - number blocking using blacklists only
  - number blocking using blacklists and avoid blocking, if number is in whitelist
  - number has to be in whitelist, all other numbers are blocked
- different kind of ways to verify incoming calls
  - support of online spam check sites, for spam verification
  - self maintained offline blacklist
  - extern maintained offline blacklists (downloaded from the Internet and stored offline)
- support of online lookup sites, to find out who is calling
- import your addressbook as whitelist or blacklist
- nice web interface

Supported VoIP systems (tested):
- Fritzbox 7390

Supported analog modems (tested):
- USRobotics 5637


## Install daemon on a Raspberry Pi (running raspbian/jessie)
```bash
sudo apt-get install git automake g++ libpjproject-dev libjson-c-dev libboost-dev libboost-regex-dev python python-beautifulsoup python-demjson python-ldap
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


## Optional: Install WebUI on a Raspberry Pi (running raspbian/jessie)
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
  blacklists (place additional blacklists here)
  whitelists (place additional whitelists here)
/usr/bin/callblockerd (daemon)
/usr/share/callblocker (scripts)
/usr/var/www/callblocker (web interface)
```


## <a name="settingsJson"></a> Documentation of settings.json
Start with the provided template settings file (sudo mv tpl_settings.json settings.json)
```json
{ 
  "log_level": "info",
  "pjsip_log_level": 0,
  "phones": [
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
  "online_credentials": [
    {
      "name"     : "tellows_de",
      "username" : "<your partner name>",
      "password" : "<your api key>"
    },
    {
      "name"     : "whocalled_us",
      "username" : "<your username>",
      "password" : "<your password>"
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
"block_anonymous_cid"  | true, false | optional: block all calls that come to your system with a anonymous/unknown caller ID. Default is false.
"online_check"       | [`<string>`](#onlineCheck)  | optional: online check site to verify if number is spam
"online_lookup"      | [`<string>`](#onlineLookup)  | optional: online lookup site, to see who is calling
"device"             | `<string>` | Your device name (get it with dmesg). Usually its "/dev/ttyACM0".
"from_domain"        | `<string>` | Your SIP domain name
"from_username"      | `<string>` | Your SIP username
"from_password"      | `<string>` | Your SIP password
"online_credentials" | | In this section you can define credentials, which are needed by some ["online_check"](#onlineCheck) and ["online_lookup"](#onlineLookup) scripts.


## <a name="onlineCheck"></a> Online check option
This option selects the online check site to verify the number from the incoming call. If the number is listed as spam, the callblocker will block it.

Name                  | Site                           | Description
----                  | ----                           | -----------
""                    | No online check is done        |
"phonespamfilter_com" | http://www.phonespamfilter.com | Free for non commercial use
"whocalled_us"        | http://whocalled.us            | Mostly USA and Canada (+1). Free, but needs login
"tellows_de"          | http://tellows.de              | Not free

Developer hint: The online check name e.g. "tellows_de" leds to the python script name onlinecheck_tellows_de.py. This allows
easily to add additional scripts.


## <a name="onlineLookup"></a> Online lookup option
This option selects the online lookup site to find out who is calling. The all options decides depending on the region code which
site to use, as all sites only support a certain region.

Name                  | Site                           | Description
----                  | ----                           | -----------
""                    | No online lookup is done       |
"all"                 | Automatic select               | Depending on the country code, it will automatically select one of the below
"tel_search_ch"       | http://tel.search.ch           | Switzerland (+41). Free for non commercial use
"dasschnelle_at"      | http://www.dasschnelle.at      | Austria (+43). Free for non commercial use
"dasoertliche_de"     | http://www.dasoertliche.de     | Germany (+49). Free for non commercial use

Developer hint: The online lookup name e.g. "tel_search_ch" leds to the python script name onlinelookup_tel_search_ch.py. This allows
easily to add additional scripts.


## Offline blacklists (automatically periodically downloading)
Through the web interface you have the possibility to maintain your own blacklist. Additionally there is the possibility to daily
download an extern maintained blacklist. You will need to setup a cronjob for this task.

Currently the following blacklists are supported:

Name                         | Site                       | Description
----                         | ----                       | -----------
blacklist_toastedspam_com.py | http://www.toastedspam.com | Mostly USA and Canada (+1)
blacklist_ktipp_ch.py        | https://www.ktipp.ch       | Switzerland (+41)

The following cronjob will download each day the blacklist provided by ktipp_ch:
```
0 0 * * * /usr/share/callblocker/blacklist_ktipp_ch.py --output /etc/callblocker/blacklists/ >/dev/null 2>&1
```


## Setup
There are two ways to connect the callblock application with your phone system, depending if it is VoIP or analog. 


### Setup using Fritzbox with a IP-phone
- Create in the Fritzbox a new IP-phone
  - Open your webbrowser and go to the URL http://fritz.box
  - In the menu "Telefonie -> Telefoniegeraete" click on "Neues Geraet einrichten"
  - Choose "Telefon (mit und ohne Anrufbeantworter)" and click "Weiter"
  - Choose "LAN/WLAN (IP-Telefon)", for name use for example "callblocker" and click "Weiter"
  - Choose a password, remember it and click "Weiter"
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

