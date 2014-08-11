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

### Requirements

#### OS X

You will need OS X 10.9 and up, with XCode installed.

For a better user experience with `magecli`,
we recommend that you install [GNU readline](http://cnswww.cns.cwru.edu/php/chet/readline/rltop.html).

With brew:
```bash
brew install readline
```

#### CentOS

```
sudo yum install cmake automake autoconf libtool libcurl-devel readline-devel
```

#### Ubuntu/Debian

```
sudo apt-get install libcurl4-openssl-dev cmake libreadline-dev
```

### Setup

```bash
git clone https://github.com/mage/mage-sdk-cpp.git
cd mage-sdk-cpp
git submodule update --init
```

### OS X/Linux

```
make
```

### Building for iOS, Android, WP8, etc

**Note**: when building libraries under platforms,
you should never need to provide your own cURL implementation.
We are using the one coming from a bootstrapped cocos2d-x project;
this happens to work very well on all platforms tested so far. Please
[post an issue](./issues/new) if you encounter any problems.

#### Make all platforms

```
make platforms
```

#### Make a specific platform

```
make [ios|android|wp8]
```

### magecli

Thw default make will produce a binary called
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

This application can be useful for manually interacting
with a remote MAGE instance without having to write code
yourself; great for testing and debugging.

**Note**: In the future, we would love to add some manual
commands to do the following:

* List available commands
* Destroy current session (if it is set)
* Store, manipulate and use JSON blobs in your user commands
  (instead of having to paste long strings all the time)

Please let us know if any of those feature would be really
useful/critical to you.

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

We haven't tried to integrate with these platforms yet. We
will add some integration notes for each of those projects
as soon as we have experimented with them.

Events polling
--------------

You can pull events from the `msgStream` of your MAGE server by using
the `void PullEvents(Transport transport);` function.

Two transports are available:
* `LONGPOLLING`: the request will be kept open by the server,
  until an event becomes available, or the server send a heartbeat.
* `SHORTPOLLING`: the request will return immediately with the response
  from the server.

When you use `void StartPolling(transport transport);`,
a loop is started in another thread to call `PullEvents()`.

If you use `LONGPOLLING`, after each request a new one will be sent.
If you use `SHORTPOLLING`, the loop will wait `SHORTPOLLING_INTERVAL_SECS`
seconds before sending a new request.

By default `SHORTPOLLING_INTERVAL_SECS` is set to 5 seconds.
You can change it, by adding a new flag in the `Makefile`.
You need to add `-DSHORTPOLLING_INTERVAL_SECS=5` with your value,
at the end of the `CFGLAGS` line.

Concurrency
-----------

```c++
virtual std::future<void> Call(const std::string& name,
				  const Json::Value& params,
				  const std::function<void(mage::MageError, Json::Value)>& callback,
				  bool doAsync) const;
```

When you use `mage::RPC::Call()` with a callback and in an asynchronous way,
with `doAsync` set to true, the callback will be called in a different thread.

```c++
void StartPolling(Transport transport = LONGPOLLING);
```

When you use `mage::RPC::StartPolling()`, all the `mage::RPC::ReceiveEvent()`
calls will be done in a different thread.
In consequence, your implementation of `mage::EventObserver::ReceiveEvent()`
will be called in a different thread too.

In these cases, you need to use `std::mutex` or other locking strategy to
ensure your data are not accessed at the same time by two different
threads.

Todo
-----

- [ ] Test/fix build on CentOS, Debian and Ubuntu
- [ ] Test integration against popular game development SDKs
- [ ] Session handling: save the session when we receive it, and offer an API to interact with it
- [ ] Make install/clean for the binaries (maybe)
- [ ] CLI: Have the option to list and describe the remote calls

Conding Style
-------------

We try to follow the [Google C++ Style Guide](http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml).
There are some exceptions:
* We use default arguments in functions;
* We use C++ exceptions;
* We use lambda expressions;
* We use tabs for indentation and spaces for alignment.

We use the [Hungarian Notation](http://en.wikipedia.org/wiki/Hungarian_notation) to name class attributes.
More references on Hungarian Notation:
* [http://web.mst.edu/~cpp/common/hungarian.html](http://web.mst.edu/~cpp/common/hungarian.html)
* [http://www.cse.iitk.ac.in/users/dsrkg/cs245/html/Guide.htm](http://www.cse.iitk.ac.in/users/dsrkg/cs245/html/Guide.htm)

You can check if you follow the rules by running:
```bash
make lint
```

See also
---------

- [JSONCPP, the library we use for our JSON operations](http://jsoncpp.sourceforge.net/)
- [libjson-rpc-cpp](https://github.com/cinemast/libjson-rpc-cpp)

