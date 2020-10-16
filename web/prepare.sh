#!/bin/bash -ex

npm install
npm run build

rm -f ../usr/var/www/callblocker/js/*
cp -r dist/* ../usr/var/www/callblocker/
