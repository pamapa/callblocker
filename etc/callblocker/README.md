# Documentation of settings.json
Start with the provided template settings file.
```bash
sudo mv tpl_settings.json settings.json
```

The settings file looks like this.
```json
{ 
  "log_level": "info",
  "pjsip_log_level": 0,
  "phones": [
    {
      "enabled":             false,
      "name":                "Analog Home Phone",
      "country_code":        "+41",
      "block_mode":          "logging_only",
      "block_anonymous_cid": false,
      "block_invalid_cid":   true,
      "online_check":        "tellows_de",
      "online_lookup":       "tel_search_ch",
      "device":              "/dev/ttyACM0"
    },
    { 
      "enabled":             false,
      "name":                "SIP Home Phone",
      "country_code":        "+41",
      "block_mode":          "allowlists_and_blocklists",
      "block_anonymous_cid": false,
      "block_invalid_cid":   true,
      "online_check":        "tellows_de",
      "online_lookup":       "tel_search_ch",
      "domain":              "<your sip domain>",
      "username":            "<your sip username>",
      "password":            "<your sip password>"
    }
  ],
  "online_credentials": [
    {
      "name":     "tellows_de",
      "username": "<your partner name>",
      "password": "<your api key>"
    },
    {
      "name":     "youmail_com",
      "username": "<your api key>",
      "password": "<your api sid>"
    }
  ]
}
```


## "phones" section
Fields                | Values       | Description
------                | ------       | -------
"log_level"           | "error", "warn", "info" or "debug" | Logging level. Default is "info".
"pjsip_log_level"     | 0-5          | Logging level of the pjsip library, for debugging proposes. Default is 0.
"country_code"        | `"+<X[Y][Z]>"` | Your international country code (e.g. +33 for France). This is used to map local numbers to international ones, which then are used for blocking.
"block_mode"          | "logging_only", "allowlists_only", "allowlists_and_blocklists" or "blocklists_only" | "logging_only": number is never blocked, only logged what it would do. "allowlists_only": number has to be in a allowlists (blocklists not used). "allowlists_and_blocklists": number is blocked, when in a blocklists and NOT in a allowlists (default). "blocklists_only": number is blocked, when in a blocklists. (allowlists not used)
"block_anonymous_cid" | true, false  | optional: block all calls that have an anonymous/unknown caller ID. Default is false.
"block_invalid_cid"   | true, false  | optional: block all calls that have an invalid caller ID. Default is true.
"online_check"        | [`<string>`](#onlineCheck)  | optional: online check site to verify if number is spam
"online_lookup"       | [`<string>`](#onlineLookup) | optional: online lookup site, to see who is calling
"online_cache"        | true, false  | optional: allows to cache online checks and lookups. Default is true.
"device"              | `<string>`   | If present marks phone as analog one. You can get the device name with "dmesg". Usually its "/dev/ttyACM0".
"domain"              | `<string>`   | Your SIP host (domain) name. Example: "fritz.box"
"username"            | `<string>`   | Your SIP username
"password"            | `<string>`   | Your SIP password
"realm"               | `<string>`   | optional: allows to change the SIP realm. Default is same value as specified in "domain".
"outbound_proxy"      | `<string>`   | optional: specify the URL of outbound proxies to visit for all outgoing requests. Example: "sip:192.168.0.1\;lr". Default is "" (no proxy).
"secure"              | true, false  | optional: allows to use SIPS instead of SIP. Default is false.
"forceIPv4"           | true, false  | optional: force IPv4 transport. Default is true.


## "online_credentials" section
In this section you can define credentials, which are needed by some [online check](#onlineCheck) and [online lookup](#onlineLookup) scripts.


## Privacy
Try to avoid as many online checks and lookups as possible. Each time the phone number gets through the internet and the
used sites may this phone number. Good strategies to do so are:
- use allowlist of your known good phone numbers (importing your address book)
- use offline blocklist


## <a name="onlineCheck"></a> Online check option
This option selects the online check site to verify the number from the incoming call. If the number is listed as spam, the callblocker will block it.

Name                  | Site                            | Description
----                  | ----                            | -----------
""                    | N/A                             | No online check is done
"tellows_de"          | https://tellows.de              | Registration needed, not free. Support for many countries all over the world.
"nomorobo_com"        | https://www.nomorobo.com        | Free protection for all of your VoIP landlines. Support for USA and Canada (+1).
"phonespamfilter_com" | https://www.phonespamfilter.com | Free for non commercial use. Support for Astralia, France, New Zealand, United Kingdom and United States.
"youmail_com"         | https://data.youmail.com        | Registration needed, not free. Support for USA and Canada (+1). Online caching not allowed.

Developer hint: The online check name e.g. "tellows_de" leads to the python script name `onlinecheck_tellows_de.py`.


## <a name="onlineLookup"></a> Online lookup option
This option selects the online lookup site to find out who is calling. The all options decides depending on the region code
which site to use, as all sites only support a certain region.

Name                  | Site                           | Description
----                  | ----                           | -----------
""                    | N/A                            | No online lookup is done
"all"                 | Automatic select               | Depending on the country code, it will automatically select one of the below
"dasoertliche_de"     | https://www.dasoertliche.de    | Germany (+49). Free for non commercial use
"dasschnelle_at"      | https://www.dasschnelle.at     | Austria (+43). Free for non commercial use
"tel_search_ch"       | https://tel.search.ch          | Switzerland (+41). Free for non commercial use
"paginebianche_it"    | https://www.paginebianche.it   | Italy (+39). Free for non commercial use

Developer hint: The online lookup name e.g. "tel_search_ch" leads to the python script name `onlinelookup_tel_search_ch.py`.
