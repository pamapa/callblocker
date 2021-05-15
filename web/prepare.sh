#!/bin/bash -ex

npm install
npm run build

# clean
rm -f ../usr/var/www/callblocker/js/*
rm -f ../usr/var/www/callblocker/index.html

# copy to layout
\cp -r ./dist/* ../usr/var/www/callblocker/
