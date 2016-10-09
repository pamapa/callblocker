#!/usr/bin/env python

# callblocker - blocking unwanted calls from your home phone
# Copyright (C) 2015-2016 Patrick Ammann <pammann@gmx.net>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 3
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#

from __future__ import print_function
import re
import codecs, csv
from datetime import datetime

from import_base import ImportBase


class UTF8Recoder:
    """
    Iterator that reads an encoded stream and re-encodes the input to UTF-8
    """
    def __init__(self, f, encoding):
        self.reader = codecs.getreader(encoding)(f)

    def __iter__(self):
        return self

    def next(self):
        return self.reader.next().encode("utf-8")

class UnicodeDictReader:
    """
    A CSV reader which will iterate over lines in the CSV file "f",
    which is encoded in the given encoding.
    """
    def __init__(self, f, delimiter=',', dialect=csv.excel, encoding="utf-8"):
        f = UTF8Recoder(f, encoding)
        self.reader = csv.DictReader(f, delimiter=delimiter, dialect=dialect)

    def next(self):
        row = self.reader.next()
        for key in row:
            if row[key] is not None:
              try:
                row[key] = row[key].decode("utf-8")
              # Special case, sometimes the content gets reqd as a list
              except AttributeError:
                  newList = []
                  for item in row[key]:
                      newList.append(item.decode("utf-8"))
                  row[key] = newList
            else:
              row[key] = ''
        return row

    def __iter__(self):
        return self


class ImportCSV(ImportBase):
    def _find_delimiter(self, filname):
        with open(filname, 'r') as f:
            line = f.readline()
            semi_cnt = line.count(";")
            comma_cnt = line.count(",")
            delimiter = ","
            if semi_cnt > comma_cnt: delimiter = ";"
            self.log.debug("Correct delimiter is %s" % delimiter)
            return delimiter

    # Finding the correct encoding for the file
    def _find_encoding(self, filename, delimiter):
        self.log.debug("Detecting encoding of the CSV file...")

        all_encoding = [
            "utf-8", "iso-8859-1", "iso-8859-2", "iso-8859-15",
            "iso-8859-3", "us-ascii", "windows-1250", "windows-1252",
            "windows-1254", "ibm861"
        ]
        next_encoding = "?"
        encoding_index = 0
        csv_reader = None
        while csv_reader is None:
            next_encoding = all_encoding[encoding_index]
            self.log.debug("Trying %s" % next_encoding)
            csv_file = open(filename, "rt")
            csv_reader = UnicodeDictReader(csv_file, delimiter=delimiter, encoding=next_encoding)
            try:
                for line in enumerate(csv_reader):
                    # Do nothing, just reading the whole file
                    encoding_index = encoding_index
            except UnicodeDecodeError:
                csv_reader = None
            csv_file.close()
            encoding_index += 1

        self.log.debug("Correct encoding is %s" % next_encoding)
        return next_encoding

    def _extract_number(self, data):
        n = re.sub(r"[^0-9\+]","", data)
        return n

    def _get_entity_person(self, fields):
        name = ""
        # first name
        for field_name in fields:
            if field_name and field_name.lower().find("first name") != -1:
                name += " " + fields[field_name]
                break
        # middle name
        for field_name in fields:
            if field_name and field_name.lower().find("middle name") != -1:
                name += " " + fields[field_name]
                break
        # last name
        for field_name in fields:
            if field_name and field_name.lower().find("last name") != -1:
                name += " " + fields[field_name]
                break
        # tellows: name
        if "Score" in fields and "Anruftyp" in fields:
            name = "%s / score:%s" % (fields["Anruftyp"], fields["Score"])
        return name.strip()

    def _parse_csv(self, filename, delimiter, encoding):
        csv_file = open(filename, "rt")
        csv_reader = UnicodeDictReader(csv_file, delimiter=delimiter, encoding=encoding)

        date = datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S +0000")
        #self.log.debug(date)

        # "Home Phone" and "Work Phone"
        # "Mobile Number" and "Pager Number"
        # "Home Fax" and "Work Fax"
        number_names = ["phone", "number", "fax"]

        entries = []
        for (line, fields) in enumerate(csv_reader):
            #self.log.debug(fields)
            name = self._get_entity_person(fields)
            self.log.debug(name)
            for field_name in fields:
                if not field_name: continue
                number = ""
                for n in number_names:
                    if field_name.lower().find(n) != -1:
                        number = fields[field_name]
                        break
                # workaround for tellows.de: number = Land Nummer
                if field_name.find("Nummer") != -1 and "Land" in fields:
                    number = "+%s%s" % (fields["Land"], fields[field_name][1:])
                    field_name = "Work Phone"
                number = self._extract_number(number)
                if len(number) != 0:
                    entries.append({"number": number, "name": name+" ("+field_name+")", "date_created": date, "date_modified": date})

        csv_file.close()
        return entries

    def get_entries(self, args):
        delimiter = self._find_delimiter(args.input)
        encoding = self._find_encoding(args.input, delimiter)
        entries = self._parse_csv(args.input, delimiter, encoding)
        return entries


# main
#
if __name__ == "__main__":
    m = ImportCSV()
    parser = m.get_parser("Convert CSV file to json")
    args = parser.parse_args()
    m.run(args)
