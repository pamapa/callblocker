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

## <a name="entry"></a> Entry
Fields          | Values       | Description
--------------- | ------------ | ------------
"name"          | `<string>`   | Name describing the number
"number"        | `<\+[0-9]+>` | Number must be in international format (E.164). Use first part of number to match a whole area/region (e.g. +234).
"date_created"  | `<date>`     | Date in UTC formatted like "%Y-%m-%d %H:%M:%S +0000"
"date_modified" | `<date>`     | Date in UTC formatted like "%Y-%m-%d %H:%M:%S +0000"

