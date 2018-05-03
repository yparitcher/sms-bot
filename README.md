# sms-bot
sms-bot is a modular sms query bot for responding to sms commands via email.

This is a work in progress so there will be updates and changes coming.
Source code documentation coming soon.

## About
sms-bot uses a Gmail account to listen for texts and respond based on the query.
It is designed to be run as a daemon that will check the email for new texts, parse them and respond.

## Dependency
sms-bot requires:
- [libcurl](https://curl.haxx.se/libcurl/) (for the network library).
- libsystemd-dev (to interface with systemd).
- To use as a service [systemd](https://www.freedesktop.org/wiki/Software/systemd/) must be your init system.
- [Gmail](https://www.google.com/gmail)
- [Weather underground api](https://www.wunderground.com/weather/api/)

## Setup
- make a [Gmail](https://www.google.com/gmail) account (preferable to use a dedicated one so as not to interfere with regular emails).
- turn on support for [less secure apps](https://myaccount.google.com/lesssecureapps).
- sign up for [Weather underground api](https://www.wunderground.com/weather/api/) and get your api key.
- edit the systemd service file to your liking see `smsbot.service.example`.
- edit the .conf file by adding your:
    - email
    - user:password
    - prefered mms response suffix (for messages over 160 characters)
    - weather underground key
see `smsbot.conf.example`.

## Installation
- run `git clone https://github.com/yparitcher/sms-bot`.
- you may need to run `git submodules init` and `git submodules update` to get the submodule libraries.
- run `make`.
- copy the resulting `smsbot` file to `/usr/bin/`.
- copy your `smsbot.service` to `/etc/systemd/system/`.
- copy your `smsbot.conf` to `/etc/smsbot/` (you may have to make the folder).
- to start the service run `systemctl start smsbot.service`.

## Usage
- for the current weather send a text to the email address with the zipcode `#weather 00501`
- for the forecast send a text to the email address with the zipcode `#forecast 00501`, 
optionally including the amount of days `#forecast 00501 5`. the default is 2 days. 

## Acknowledgements
sms-bot uses the following ibraries:

- [parson](https://github.com/kgabis/parson) library to parse json.
Copyright (c) 2012 - 2017 [Krzysztof Gabis](https://github.com/kgabis)
licensed under the MIT licence
- [ini](https://github.com/rxi/ini) library for the config file.
Copyright (c) 2016 [rxi](https://github.com/rxi)
licensed under the MIT licence

Thank you

## License
sms-bot is Copyright (c) 2018 [Yparitcher](https://github.com/yparitcher)
licensed under [The MIT License (MIT)](http://opensource.org/licenses/mit-license.php) 
