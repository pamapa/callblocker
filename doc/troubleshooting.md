# Troubleshooting

## Symptom: It is unspecific not working.
- double check all installed files, with its locations and permissions. See [file layout](/doc/development.md#fileLayout)
- make sure lighttpd and callblockerd are running.<br>
```bash
sudo ps aux | grep -E 'lighttpd|callblockerd' | grep -v 'grep' # shows: 2 lines
```
- check for possible errors/warning.<br>
```bash
sudo journalctl _SYSTEMD_UNIT=callblockerd.service
```
- increase log levels: "log_level" to "debug" and/or "pjsip_log_level" to 2. See documentation of
   [configuration file](/etc/callblocker/README.md) for more info. And restart callblockerd.
```bash
sudo vi /etc/callblocker/settings.json
sudo systemctl restart callblockerd.service
```

## Symptom: Web interface is not working.
The web interface is running within lighttpd, double check the [web configuration](/README.md#webInterface) of this deamon.
- also look into the seperate log file:
```bash
sudo cat /var/log/lighttpd/error.log
sudo journalctl -xn _SYSTEMD_UNIT=lighttpd.service
```
- make sure the python file api.py has correct execution rights
```bash
sudo chmod a+x /usr/var/www/callblocker/python-fcgi/api.py
```

## Symptom: Configuration done within the web interface is not saved persistent.
The web interface is running within lighttpd, this deamon is using "www-data" as user and group. Make
sure that this process has access to the configuration file (see [file layout](/doc/development.md#fileLayout)).
```bash
sudo chown -R www-data.www-data /etc/callblocker/
```

## Symptom: Caller log and diagnostics stay empty within the web interface.
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

