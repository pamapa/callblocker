# callblocker

![](https://github.com/pamapa/callblocker/workflows/Test/badge.svg)
![](https://github.com/pamapa/callblocker/workflows/Frontend/badge.svg)
![](https://github.com/pamapa/callblocker/workflows/Debian/badge.svg)
![](https://github.com/pamapa/callblocker/workflows/Raspbian/badge.svg)
![](https://github.com/pamapa/callblocker/workflows/Ubuntu/badge.svg)

Blocks unwanted calls from your home phone, such as telemarketing, junk, spam, scam, etc.

The call blocker acts like a normal phone. No additional telephone switchboard (like FreePBX or Asterisk) is needed. When a telemarketer is calling, the call blocker picks up the phone and automatically hangs up after a few seconds. Like this the call is blocked. It's designed to run on devices with small footprints, such as a Raspberry Pi. When the call blocker is not running (e.g. hardware broke) your home phone is still working by design. Also it has the ability to check against online and offline phone spam lists.


## Features
- supporting VoIP systems and analog modems
- blocking automatically unwanted incoming calls
- logging of all incoming calls, including caller name lookups
- import your addressbook as allow or block list
- different kind of modes
  - logging only
  - block number via lists
  - block number via lists but avoid blocking, iff number is in allow lists
  - block number if not in allow lists
- different kind of ways to verify incoming calls
  - support of online spam check sites, for spam verification
  - user maintained offline blocklist
  - extern maintained offline blocklists (downloaded from the Internet and stored offline)
  - detect anonymous and invalid numbers
  - support range of numbers matching whole areas/regions
- different kind of ways to get caller name of incoming calls
  - support of online lookup sites, to find out who is calling
  - support CID information coming from analog modem, iff supported from the landline provider
  - using allow and block lists as offline lookup for caller name
- online request caching
- nice web interface


## Hardware
Supported (tested) server host systems
- Raspberry Pi
  - raspbian/bullseye (use master branch)
  - raspbian/buster (use master branch)
- Debian GNU/Linux
  - bullseye (use master branch)
  - buster (use master branch)

Supported (tested) VoIP systems
- Fritzbox 7390, 5490

Supported (tested) analog modems
- USRobotics: USR5637 (56K USB Faxmodem)
- Zoom: Model 3095 (V.92 56K USB Mini External Modem)


## Install

### via Debian package
Download provided Debian package matching your OS.

Work in progress: not finished yet! Use 'via manual build' for now.

```bash
apt install ./callblocker_*.deb
```

### via manual build
Follow [this instructions](/docs/development.md).


## <a name="webInterface"></a> Setup web interface on Linux
The configuration of the web interface is optional, the callblocker daemon works perfectly without it.
The web interface allows to view the caller log, change settings and diagnose problems. This instructions
explain howto make it work with lighttpd, its also possible to configure it via apache.

Additional Debian packages are required:
```bash
sudo apt-get install lighttpd python3-legacy-cgi python3-systemd python3-setuptools python3-wheel
```

Prepare lighttpd, for additional information
see [here](http://redmine.lighttpd.net/projects/lighttpd/wiki/Docs_ModFastCGI):
```bash
sudo pip3 install flipflop --break-system-packages
sudo lighty-enable-mod fastcgi
sudo vi /etc/lighttpd/lighttpd.conf
```
- make server.document-root point to "/usr/var/www/callblocker"
- at the end of this file add this code:
```section
   fastcgi.server = (
     ".py" => (
       "callblocker-fcgi" => (
         "bin-path" => "/usr/var/www/callblocker/python-fcgi/api.py",
         "socket" => "/var/run/lighttpd/fastcgi.python.socket"
       )
     )
   )
```
- make sure the lighttpd daemon has write access to the callblocker settings
```bash
sudo usermod -a -G systemd-journal www-data
sudo chown -R www-data.www-data /etc/callblocker/
```
- make sure the python file api.py has correct execution rights and restart lighttpd daemon.
```bash
sudo systemctl restart lighttpd.service
```


## Setup
There are two ways to connect the call blocker application with your phone system, depending if it is VoIP or analog. 

### Prepare your settings file
```bash
cd /etc/callblocker
sudo mv tpl_settings.json settings.json
sudo vi settings.json
sudo systemctl start callblockerd.service
```

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


## Configuration file
The documentation of the configuration file "settings.json" is located [here](/etc/callblocker/README.md).


## Offline blocklists
Through the web interface you have the possibility to maintain your own blocklist. Additionally there is the
possibility to download automatically an extern maintained blocklist for later offline usage. You will need to
setup a cronjob for this task to download the blocklist periodically.

Currently the following blocklists are supported:

Name                         | Site                       | Description
----                         | ----                       | -----------
blocklist_toastedspam_com.py | http://www.toastedspam.com | Mostly USA and Canada (+1)
blocklist_ktipp_ch.py        | https://www.ktipp.ch       | Switzerland (+41)

The following cronjob will download each day the blocklist provided by ktipp_ch:
```bash
0 0 * * * /usr/share/callblocker/blocklist_ktipp_ch.py --output /etc/callblocker/blocklists/ >/dev/null 2>&1
```


## Troubleshooting
See [this instructions](/docs/troubleshooting.md).

