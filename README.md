# callblocker
Blocks unwanted calls from your home phone, such as telemarketing, junk, spam, scam, etc.

The call blocker acts like a normal phone. No additional telephone switchboard (like FreePBX or Asterisk) is needed. When a telemarketer is calling, the call blocker picks up the phone and automatically hangs up after a few seconds. Like this the call is blocked. It's designed to run on small footprints such as a Raspberry Pi. When the call blocker is not running (e.g. hardware broke) your home phone is still working by design. Also it has the ability to check against online and offline phone spam lists.


## Features
- supports VoIP systems and analog modems
- automatically blocks unwanted incoming calls
- logging of all incoming calls, including caller name lookups
- import your addressbook as whitelist or blacklist
- different kind of blocking modes
  - logging only
  - number blocking using blacklists only
  - number blocking using blacklists and avoid blocking, if number is in whitelist
  - number has to be in whitelist, all other numbers are blocked
- different kind of ways to verify incoming calls
  - user maintained offline blacklist
  - extern maintained offline blacklists (downloaded from the Internet and stored offline)
  - support of online spam check sites, for spam verification
  - detect anonymous and invalid numbers
  - support range of numbers matching whole areas/regions
- different kind of ways to get caller name of incoming calls
  - using whitelists and blacklists as offline lookup for caller name
  - support of online lookup sites, to find out who is calling
- online request caching
- nice web interface


## Hardware
Supported (tested) server host systems
- Raspberry Pi
  - raspbian/jessie (use v0.10.x release)
  - raspbian/stretch (use master branch)
- Debian GNU/Linux
  - jessie (use v0.10.x release)
  - stretch (use master branch)

Supported (tested) VoIP systems
- Fritzbox 7390

Supported (tested) analog modems
- USRobotics 5637


## Install daemon on Linux
```bash
sudo apt-get install git make automake g++ libpjproject-dev libjson-c-dev libphonenumber-dev
sudo apt-get install python3 python3-bs4 python3-ldif3 python3-vobject
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


## <a name="webInterface"></a> Install web interface on Linux
The installation of the web interface is optional, the callblock daemon works perfectly without it.
The web interface allows to view the caller log, change settings and diagnose problems.

Debian packages are required:
```bash
sudo apt-get install lighttpd python3-pip
```

Yarn is required, install like described [here](https://yarnpkg.com/en/docs/install):
```bash
wget -qO- https://dl.yarnpkg.com/debian/pubkey.gpg | sudo apt-key add -OK
echo "deb https://dl.yarnpkg.com/debian/ stable main" | sudo tee /etc/apt/sources.list.d/yarn.list
sudo apt-get update && sudo apt-get install yarn
```

Fetch (within checkout directory) and install vendor files:
```bash
cd usr/var/www/callblocker/js
yarn install --modules-folder vendor
sudo cp -r vendor /usr/var/www/callblocker/js/
```

Prepare lighttpd, for additional information
see [here](http://redmine.lighttpd.net/projects/lighttpd/wiki/Docs_ModFastCGI):
```bash
sudo pip3 install flipflop
sudo usermod -a -G systemd-journal www-data
sudo vi /etc/lighttpd/lighttpd.conf
```
- in the upper part you find the section 'server.modules='. Please add the module "mod_fastcgi" to it.
- make server.document-root point to "/usr/var/www/callblocker"
- at the end of this file add this code:
```section
   fastcgi.server              = (
        ".py" => (
                "callblocker-fcgi" => (
                        "bin-path" => "/usr/var/www/callblocker/python-fcgi/api.py",
                        "socket" => "/var/run/lighttpd/fastcgi.python.socket")
        )
   )
```
- make sure the python file api.py has correct execution rights and restart lighttpd daemon.
```bash
sudo systemctl restart lighttpd.service
```


## Updating daemon and web interface on Linux
You have prevoiusly installed the callblock application and would like to update it to the lastest version. Make a backup
of your configuration (/etc/callblocker), the installation will not overwrite it, but you never know.

In case you have still the installation git checkout:
```bash
cd callblocker
git pull
make clean all
```

Else you do not have the installation git checkout anymore:
```bash
git clone https://github.com/pamapa/callblocker.git
cd callblocker
aclocal
automake --add-missing --foreign
autoconf
./configure --prefix=/usr --sysconfdir=/etc
make all
```

Double check your [settings.json](/etc/callblocker/README.md). Hints:
- v0.0.7: switched from php to python web backend
- v0.9.0: prefix "from_" has been removed from "from_domain", "from_username" and "from password"

```bash
sudo systemctl stop callblockerd
sudo make install
sudo systemctl daemon-reload
sudo systemctl start callblockerd
```

Double check your [web configuration](#webInterface) and do:
```bash
sudo systemctl restart lighttpd.service
```


## <a name="fileLayout"></a> File Layout
When installed on Linux, the following file layout is used
```
drwxr-xr-x  www-data www-data  /etc/callblocker                            # configuration
-rw-r--r--  www-data www-data  /etc/callblocker/settings.json              # configuration file
drwxr-xr-x  www-data www-data  /etc/callblocker/blacklists                 # put your blacklists here
drwxr-xr-x  www-data www-data  /etc/callblocker/whitelists                 # put your whitelists here
drwxr-xr-x  www-data www-data  /etc/callblocker/cache                      # used for caching online request
-rwxr-xr-x  root     root      /usr/bin/callblockerd                       # daemon
drwxr-xr-x  root     root      /usr/share/callblocker                      # python helper scripts
drwxr-xr-x  root     root      /usr/var/www/callblocker                    # web interface
-rwxr-xr-x  root     root      /usr/var/www/callblocker/python-fcgi/api.py # web api
```


## Configuration file
The documentation of the configuration file "settings.json" is located [here](/etc/callblocker/README.md).


## Offline blacklists
Through the web interface you have the possibility to maintain your own blacklist. Additionally there is the
possibility to download automatically an extern maintained blacklist for later offline usage. You will need to
setup a cronjob for this task to download the blacklist periodically.

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
There are two ways to connect the call blocker application with your phone system, depending if it is VoIP or analog. 


### Setup using Fritzbox with an IP-phone
- create in the Fritzbox a new IP-phone
  - open your web browser and navigate to the URL http://fritz.box
  - in the menu "Telefonie -> Telefoniegeraete" click on "Neues Geraet einrichten"
  - choose "Telefon (mit und ohne Anrufbeantworter)" and click "Weiter"
  - choose "LAN/WLAN (IP-Telefon)", for name use for example "callblocker" and click "Weiter"
  - choose a password, remember it and click "Weiter"
  - choose "alle Anrufe annehmen" and click "Weiter"
- setup the IP-phone in the call blocker configuration ([/etc/callblocker/setting.json](/etc/callblocker/README.md)):
  - edit the section sip -> accounts
  - "domain":   "fritz.box"
  - "username": "your username"
  - "password": "your password"
  - make sure the account is enabled and the other fields are ok for you


### Setup using an analog phone
- attach the USB modem to the Raspberry Pi
- use `dmesg` to find the device name `/dev/<name>`
- setup the Analog phone in the call blocker configuration ([/etc/callblocker/setting.json](/etc/callblocker/README.md)):
  - edit the section analog -> phones
  - "device": "your device name"
  - make sure the account is enabled and the other fields are ok for you


## Troubleshooting

### Symptom: It is unspecific not working.
- double check all installed files, with its locations and permissions. See [file layout](#fileLayout)
- make sure lighttpd and callblockerd are running.<br>
```bash
sudo ps aux | grep -E 'lighttpd|callblockerd' | grep -v 'grep' # shows: 2 lines
```
- check for possible errors/warning.<br>
```bash
sudo journalctl _SYSTEMD_UNIT=callblockerd.service
```
- increase log levels: "log_level" to "debug" and/or "pjsip_log_level" to 2. See documentation of
   [configuration file](/etc/callblocker/README.md) for more info.
```bash
sudo vi settings.json
```

### Symptom: Web interface is not working.
The web interface is running within lighttpd, double check the [web configuration](#webInterface) of this deamon.
- also look into the seperate log file:
```bash
sudo cat /var/log/lighttpd/error.log
sudo journalctl -xn _SYSTEMD_UNIT=lighttpd.service
```
- make sure the python file api.py has correct execution rights
```bash
sudo chmod a+x /usr/var/www/callblocker/python-fcgi/api.py
```

### Symptom: Configuration done within the web interface is not saved persistent.
The web interface is running within lighttpd, this deamon is using "www-data" as user and group. Make
sure that this process has access to the configuration file (see [file layout](#fileLayout)).
```bash
sudo chmod -R www-data.www-data /etc/callblocker/
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

# allow web interface access the journal
sudo usermod -a -G systemd-journal www-data
sudo systemctl restart lighttpd.service

# manual verify that journal is working
sudo journalctl _SYSTEMD_UNIT=callblockerd.service
```
