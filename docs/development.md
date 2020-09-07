# Development
In case the provided Debian packages do not work, you can manually
build everything by your own.


## Download project source code
```bash
git clone https://github.com/pamapa/callblocker.git
cd callblocker
```


## Install build dependencies
```bash
sudo apt-get install --no-install-recommends -y \
  debhelper build-essential git \
  dpkg-dev pkg-config \
  ca-certificates wget \
  ninja-build meson \
  g++ gcc \
  libjson-c-dev libphonenumber-dev uuid-dev libssl-dev \
  python3 python3-bs4 python3-ldif3 python3-vobject \
  python3-pip python3-setuptools
sudo pip3 install meson==0.50.1

# for web-interface (not needed in case of -Dweb-interface=false)
wget -qO- https://deb.nodesource.com/setup_12.x | bash -
sudo apt-get update
sudo apt-get install -y nodejs
```


## Build and install

### via 'ninja install'
```bash
# for no web-interface: add below: -Dweb-interface=false
meson --prefix=/usr --sysconfdir=/etc --localstatedir=/usr/var build

cd build
ninja
sudo ninja install
```

### via 'selfmade Debian package'
```bash
dpkg-buildpackage -b -uc
sudo apt install ../callblocker_*.deb
```


## Updating daemon and web interface on Linux
You have prevoiusly installed the callblock application and would like to update it to the lastest version. Make a backup
of your configuration (/etc/callblocker), the installation will not overwrite it, but you never know.

1. In case you have still the installation git checkout:
```bash
cd callblocker
git pull
cd build
sudo ninja install
```

Else you do not have the installation git checkout anymore:
```bash
git clone https://github.com/pamapa/callblocker.git
cd callblocker
# for no web-interface: add below: -Dweb-interface=false
meson --prefix=/usr --sysconfdir=/etc --localstatedir=/usr/var build
cd build
sudo ninja install
```

2. Double check your [settings.json](/etc/callblocker/README.md). Hints:
- v0.0.7: switched from php to python web backend
- v0.9.0: prefix "from_" has been removed from "from_domain", "from_username" and "from password"
- v0.11.0: moved from jessie to stretch, for jessie use the 0.10.x release
- v0.13.0: switched from automake to meson/ninja and make use of static local pjproject 2.8
- v0.14.0: moved from stretch to buster, start providing Debian packages
- v0.15.0: renamed blacklists to blocklists and whitelists to allowlists, you will need to adapt settings.json + rename subfolders in /etc/callblocker/

```bash
sudo systemctl stop callblockerd
sudo ninja install
sudo systemctl daemon-reload
sudo systemctl start callblockerd
```

3. Double check your [web configuration](/README.md#webInterface) and do:
```bash
sudo systemctl restart lighttpd.service
```


## <a name="fileLayout"></a> File Layout
When installed on Linux, the following file layout is used:
```
drwxr-xr-x www-data www-data /etc/callblocker                              # configuration
-rw-r--r-- www-data www-data /etc/callblocker/settings.json                # configuration file
drwxr-xr-x www-data www-data /etc/callblocker/blocklists                   # put your blocklists here
drwxr-xr-x www-data www-data /etc/callblocker/allowlists                   # put your allowlists here
drwxr-xr-x www-data www-data /etc/callblocker/cache                        # used for caching online request
-rwxr-xr-x root     root     /usr/bin/callblockerd                         # daemon
drwxr-xr-x root     root     /usr/share/callblocker                        # python helper scripts
-rw-r--r-- root     root     /etc/systemd/system/callblockerd.service      # systemd service file
```
When using the web interface:
```
drwxr-xr-x root     root     /usr/var/www/callblocker                      # web interface
-rw-r--r-- root     root     /usr/var/www/callblocker/index.html           # start page
drwxr-xr-x root     root     /usr/var/www/callblocker/dist                 # javascript
-rw-r--r-- root     root     /usr/var/www/callblocker/dist/bundle.js       # - webpack
-rw-r--r-- root     root     /usr/var/www/callblocker/dist/1.bundle.js     # - webpack
-rw-r--r-- root     root     /usr/var/www/callblocker/dist/dojo/resources/blank.gif
drwxr-xr-x root     root     /usr/var/www/callblocker/python-fcgi          # web backend
-rwxr-xr-x root     root     /usr/var/www/callblocker/python-fcgi/api.py
-rw-r--r-- root     root     /usr/var/www/callblocker/python-fcgi/config.py
-rw-r--r-- root     root     /usr/var/www/callblocker/python-fcgi/journal2.py
-rw-r--r-- root     root     /usr/var/www/callblocker/python-fcgi/journal.py
-rw-r--r-- root     root     /usr/var/www/callblocker/python-fcgi/settings.py
```

