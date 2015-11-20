# Documentation of settings.json
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
This option selects the online lookup site to find out who is calling. The all options decides depending on the region code
which site to use, as all sites only support a certain region.

Name                  | Site                           | Description
----                  | ----                           | -----------
""                    | No online lookup is done       |
"all"                 | Automatic select               | Depending on the country code, it will automatically select one of the below
"tel_search_ch"       | http://tel.search.ch           | Switzerland (+41). Free for non commercial use
"dasschnelle_at"      | http://www.dasschnelle.at      | Austria (+43). Free for non commercial use
"dasoertliche_de"     | http://www.dasoertliche.de     | Germany (+49). Free for non commercial use

Developer hint: The online lookup name e.g. "tel_search_ch" leds to the python script name onlinelookup_tel_search_ch.py. This allows easily to add additional scripts.

