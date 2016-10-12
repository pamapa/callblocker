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
      "block_invalid_cid":   false,
      "online_check":        "tellows_de",
      "online_lookup":       "tel_search_ch",
      "device":              "/dev/ttyACM0"
    },
    { 
      "enabled":             false,
      "name":                "SIP Home Phone",
      "country_code":        "+41",
      "block_mode":          "whitelists_and_blacklists",
      "block_anonymous_cid": false,
      "block_invalid_cid":   false,
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
      "name":     "whocalled_us",
      "username": "<your username>",
      "password": "<your password>"
    }
  ]
}
```
Fields                | Values       | Description
------                | ------       | -------
"log_level"           | "error", "warn", "info" or "debug" | Logging level. Default is "info".
"pjsip_log_level"     | 0-5          | Logging level of the pjsip library, for debugging proposes. Default is 0.
"country_code"        | `"+<X[Y][Z]>"` | Your international country code (e.g. +33 for France)
"block_mode"          | "logging_only", "whitelists_only", "whitelists_and_blacklists" or "blacklists_only" | "logging_only": number is never blocked, only logged what it would do. "whitelists_only": number has to be in a whitelists (blacklists not used). "whitelists_and_blacklists": number is blocked, when in a blacklists and NOT in a whitelists (default). "blacklists_only": number is blocked, when in a blacklists. (whitelists not used)
"block_anonymous_cid" | true, false  | optional: block all calls that have an anonymous/unknown caller ID. Default is false.
"block_invalid_cid"   | true, false  | optional: block all calls that have an invalid caller ID. Default is false.
"online_check"        | [`<string>`](#onlineCheck)  | optional: online check site to verify if number is spam
"online_lookup"       | [`<string>`](#onlineLookup)  | optional: online lookup site, to see who is calling
"device"              | `<string>`   | Your device name (get it with dmesg). Usually its "/dev/ttyACM0".
"domain"              | `<string>`   | Your SIP domain name
"username"            | `<string>`   | Your SIP username
"password"            | `<string>`   | Your SIP password
"realm"               | `<string>`   | optional: allows to change the realm. Default is same value as specified in "domain".
"secure"              | `<string>`   | optional: allows to use SIPS instead of SIP. Default is false.
"online_credentials"  | | In this section you can define credentials, which are needed by some [online check](#onlineCheck) and [online lookup](#onlineLookup) scripts.


## Privacy
Try to avoid as many online checks and lookups as possible. Each time the phone number gets through the internet and the
used sites may this phone number. Good strategies to do so are:
- use whitelist of your known good phone numbers (importing your address book)
- use offline blacklist


## <a name="onlineCheck"></a> Online check option
This option selects the online check site to verify the number from the incoming call. If the number is listed as spam, the callblocker will block it.

Name                  | Site                           | Description
----                  | ----                           | -----------
""                    | N/A                            | No online check is done
"phonespamfilter_com" | http://www.phonespamfilter.com | Free for non commercial use. Support for Astralia, France, New Zealand, United Kingdom and United States.
"whocalled_us"        | http://whocalled.us            | Free, support for USA and Canada (+1).
"tellows_de"          | http://tellows.de              | Not free, support for many countries all over the world.

Developer hint: The online check name e.g. "tellows_de" leds to the python script name onlinecheck_tellows_de.py.


## <a name="onlineLookup"></a> Online lookup option
This option selects the online lookup site to find out who is calling. The all options decides depending on the region code
which site to use, as all sites only support a certain region.

Name                  | Site                           | Description
----                  | ----                           | -----------
""                    | N/A                            | No online lookup is done
"all"                 | Automatic select               | Depending on the country code, it will automatically select one of the below
"dasoertliche_de"     | http://www.dasoertliche.de     | Germany (+49). Free for non commercial use
"dasschnelle_at"      | https://www.dasschnelle.at     | Austria (+43). Free for non commercial use
"tel_search_ch"       | https://tel.search.ch          | Switzerland (+41). Free for non commercial use
"pagesjaunes_fr"      | http://www.pagesjaunes.fr      | France (+33). Free for non commercial use
"paginebianche_it"    | http://www.paginebianche.it    | Italy (+39). Free for non commercial use

Developer hint: The online lookup name e.g. "tel_search_ch" leds to the python script name onlinelookup_tel_search_ch.py.
