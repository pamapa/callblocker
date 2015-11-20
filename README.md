# callblocker
Blocks unwanted calls from your home phone, such as telemarketing, junk, spam, scam, etc.

The callblocker acts like a normal phone. No additional telephone switchboard (like FreePBX or Asterisk) is needed. When a telemarketer is calling, the callblocker picks up the phone and automatically hangs up after a few seconds. Like this the call is blocked. It's designed to run on small footprints such as a Raspberry Pi. In addition when the callblocker is not running (e.g. hardware broke) your home phone is still working. It has the ability to check against online and offline spam lists.


## Features
- automatically blocks unwanted incoming calls
- logging of all incoming calls, including caller name lookups
- different kind of block modes.
  - logging only
  - number blocking using blacklists only
  - number blocking using blacklists and avoid blocking, if number is in whitelist
  - number has to be in whitelist, all other numbers are blocked
- different kind of ways to verify incoming calls
  - support of online spam check sites, for spam verification
  - user maintained offline blacklist
  - extern maintained offline blacklists (downloaded from the Internet and stored offline)
- support of online lookup sites, to find out who is calling
- import your addressbook as whitelist or blacklist and caller lookup
- nice web interface


## Hardware
Supported (tested) server host systems
- Raspberry Pi (running raspbian/jessie)

Supported (tested) VoIP systems
- Fritzbox 7390

Supported (tested) analog modems
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
sudo vi settings.json
sudo systemctl start callblockerd.service
```


## Install web interface on a Raspberry Pi (running raspbian/jessie)
```bash
sudo apt-get install lighttpd php5-common php5-cgi php5 libjs-dojo-core libjs-dojo-dijit libjs-dojo-dojox
sudo chgrp -R www-data /etc/callblocker/
sudo usermod -a -G systemd-journal www-data
sudo vi /etc/lighttpd/lighttpd.conf
```
1. In the upper section of this file you can find den section 'server.modules='. Please add this line: '"mod_fastcgi",'.
2. At the end of file, add: 'fastcgi.server = (".php"=>(("bin-path"=>"/usr/bin/php-cgi", "socket"=>"/tmp/php.sock")))'.
3. Make server.document-root point to "/usr/var/www/callblocker"
```bash
sudo reboot
```


## <a name="fileLayout"></a> File Layout
When installed on Linux, the following file layout is used
```
drwxr-xr-x  www-data www-data  /etc/callblocker               # configuration
-rw-r--r--  www-data www-data  /etc/callblocker/settings.json # configuration file
drwxr-xr-x  www-data www-data  /etc/callblocker/blacklists    # put your blacklists here
drwxr-xr-x  www-data www-data  /etc/callblocker/whitelists    # put your whitelists here
-rwxr-xr-x  root     root      /usr/bin/callblockerd          # daemon
drwxr-xr-x  root     root      /usr/share/callblocker         # python helper scripts
drwxr-xr-x  root     root      /usr/var/www/callblocker       # web interface
```


## <a name="settingsJson"></a> Documentation of settings.json
Start with the provided template settings file (sudo mv tpl_settings.json settings.json)
```json
{ 
  "log_level": "info",
  "pjsip_log_level": 0,
  "phones": [
    {
      "enabled":           false,
      "name":              "Analog Home Phone",
      "country_code":      "+41",
      "block_mode":        "logging_only",
      "block_unknown_cid": false,
      "online_check":      "tellows_de",
      "online_lookup":     "tel_search_ch",
      "device":            "/dev/ttyACM0"
    },
    { 
      "enabled":           false,
      "name":              "SIP Home Phone",
      "country_code":      "+41",
      "block_mode":        "whitelists_and_blacklists",
      "block_unknown_cid": false,
      "online_check":      "tellows_de",
      "online_lookup":     "tel_search_ch",
      "from_domain":       "<your domain>",
      "from_username":     "<your username>",
      "from_password":     "<your password>"
    }
  ],
  "online_credentials": [
    {
      "name":     "tellows_de",
      "username": "<your partner name>",
      "password": "<your api key>"
    },
    {
      "name":     "whocalled_us",
      "username": "<your username>",
      "password": "<your password>"
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
"online_credentials" | | In this section you can define credentials, which are needed by some [online check](#onlineCheck) and [online lookup](#onlineLookup) scripts.


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
```bash
0 0 * * * /usr/share/callblocker/blacklist_ktipp_ch.py --output /etc/callblocker/blacklists/ >/dev/null 2>&1
```


## Setup
There are two ways to connect the callblocker application with your phone system, depending if it is VoIP or analog. 


### Setup using Fritzbox with a IP-phone
- Create in the Fritzbox a new IP-phone
  - Open your web browser and navigate to the URL http://fritz.box
  - In the menu "Telefonie -> Telefoniegeraete" click on "Neues Geraet einrichten"
  - Choose "Telefon (mit und ohne Anrufbeantworter)" and click "Weiter"
  - Choose "LAN/WLAN (IP-Telefon)", for name use for example "callblocker" and click "Weiter"
  - Choose a password, remember it and click "Weiter"
  - Choose "alle Anrufe annehmen" and click "Weiter"
- Setup the IP-phone in the callblocker configuration (/etc/callblocker/setting.json):
  - Edit the section sip -> accounts
  - "from_domain":   "fritz.box"
  - "from_username": "your username"
  - "from_password": "your password"
  - Make sure the account is enabled and the other fields ok ok for you


### Setup using an analog phone
- Attach the USB modem to the Raspberry Pi
- Use `dmesg` to find the device name `/dev/<name>`
- Setup the Analog phone in the callblocker configuration (/etc/callblocker/setting.json):
  - Edit the section analog -> phones
  - "device": "your device name"
  - Make sure the account is enabled and the other fields ok ok for you


## Troubleshooting

### Symptom: It is unspecific not working.
1. Double check all installed files, with its locations and permissions. See [File Layout](#fileLayout)
2. Make sure lighttpd and callblockerd are running.<br>
   ```bash
   sudo ps aux | grep -E 'lighttpd|callblockerd' | grep -v 'grep' # shows: 2 lines
   ```
3. Check for possible errors/warning.<br>
   ```bash
   sudo journalctl _SYSTEMD_UNIT=callblockerd.service
   ```

### Symptom: Configuration done via web interface is not saved persistent.
The web interface is running within lighttpd, this deamon is using "www-data" as user and group. Make
sure that this process has access to the configuration file (see [File Layout](#fileLayout)).
```bash
sudo chgrp -R www-data /etc/callblocker/
```

### Symptom: Caller log and diagnostics stay empty within the web interface.
Make sure journal is active and working and the web interface has access to the journal.The web interface
depends on functionality provided by systemd journal. 
```bash
# switch to systemd journal
sudo apt-get purge rsyslog logrotate libestr0 liblogging-stdlog0 liblognorm1
sudo vi /etc/systemd/journald.conf: #Storage=auto -> Storage=auto
sudo rm -rf /var/log/* # optional, you will lose all existing log entries (old format)
sudo mkdir /var/log/journal
sudo reboot # required to finished the switch

# give web interface access
sudo usermod -a -G systemd-journal www-data
sudo systemctl restart lighttpd.service

# manual verify that journal is working
sudo journalctl _SYSTEMD_UNIT=callblockerd.service
```

