mage-sdk-cpp
============

![MAGE Logo](./img/logo.jpg)

What is this MAGE thing anyway?
-------------------------------

- English
	- [http://www.wizcorp.jp/#portfolio](http://www.wizcorp.jp/#portfolio)
- 日本語
	- [http://www.wizcorp.jp/#portfolio](http://www.wizcorp.jp/ja/#portfolio)
	- [http://www.spiralsense.jp/products/m-a-g-e/](http://www.spiralsense.jp/products/m-a-g-e/)

Description
------------

This is a C++ library that enables you to interact with a MAGE
server. More specifically, it allows you to call any user commands
made available on a given server.

Installation
-------------

### setup

```bash
git clone git@github.com:Wizcorp/mage-sdk-cpp.git
cd mage-sdk-cpp
git submodule update --init
make
```

### magecli

After you `make`, you will find an application called
`magecli` under `./bin`. To use:

```bash
> ./bin/magecli -h
Usage: magecli -a [application name] -d [domain] [-p [protocol]] [-h]

	-a	The name of the MAGE application you wish to access
	-d	The domain name or IP address where the MAGE instance is hosted
	-p	The protocol through which you wish to communicate with MAGE (default: http)
	-h	Show this help screen
```

Some real-life examples:

![Screenshot](./img/screenshot.png)

### Building the example scripts

```
make examples
```

This will build the example programs under ./examples. Feel
free to use them to experiment a bit with the API (you will need
to change the application name and ports).

Integration
-----------

### With Cocos2dx

Coming soon

### With Unity/Unreal/Cry/etc.

We haven't tried to integrate with these technologies yet. We
will add some integration notes for each of those projects
as soon as we have experimented with them.

Todo
-----

- [ ] Session handling: save the session when we receive it, and offer an API to interact with it
- [ ] Make install/clean for the binaries (maybe)
- [ ] Message stream event handling
- [ ] CLI: Have the option to list and describe the remote calls

See also
---------

- [JSONCPP, the library we use for our JSON operations](http://jsoncpp.sourceforge.net/)
- [libjson-rpc-cpp](https://github.com/cinemast/libjson-rpc-cpp)
