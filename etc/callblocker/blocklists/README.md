# Documentation of main.json
```json
{ 
  "name": "main",
  "entries": [ 
      { 
        "name": "Telemarketing Company X",
        "number": "+123567890123",
        "date_created": "2015-05-15 12:00:00 +0000",
        "date_modified": "2015-05-15 12:00:00 +0000",
      }
    ]
}
```
Fields    | Values                  | Description
--------- | ----------------------- | ------------
"name"    | `<string>`              | Name of the list
"entries" | [`List<entry>`](#entry) | List of entries

## <a name="entry"></a> Entry - block by number
Supported in allow- and blocklists.

Fields          | Values       | Description
--------------- | ------------ | ------------
"name"          | `<string>`   | Name describing the number.
"number"        | `<\+[0-9]+>` | Number must be in international format (E.164). Use first part of number to match a whole area/region (e.g. +234).
"date_created"  | `<date>`     | Date in UTC formatted like "%Y-%m-%d %H:%M:%S +0000"
"date_modified" | `<date>`     | Date in UTC formatted like "%Y-%m-%d %H:%M:%S +0000"

## <a name="entry"></a> Entry - block by name
Supported in blocklists only.

Fields          | Values       | Description
--------------- | ------------ | ------------
"name"          | `<string>`   | Wildcard matching name (? matches any single character and * matches any string including empty string). E.g. SPAM*.
"number"        | ``           | Number must be empty.
"date_created"  | `<date>`     | Date in UTC formatted like "%Y-%m-%d %H:%M:%S +0000"
"date_modified" | `<date>`     | Date in UTC formatted like "%Y-%m-%d %H:%M:%S +0000"
