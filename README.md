# lajihttpd

## Description

This is a not very complex HTTP server for learning purpose, use [`posix_mq`](http://man7.org/linux/man-pages/man7/mq_overview.7.html) to do logging, [`epoll`](http://man7.org/linux/man-pages/man7/epoll.7.html) to do event / fd handling, with HTTP basic GET support and keep-alive support, and using an also hand-made library called [libaji](https://github.com/BLumia/libaji) to do config file processing, logging and threadpool.

## Building

To build this program by yourself, you need a unix like system with `posix_mq` support(for now both cygwin and M$'s WSL don't support `posix_mq`, so you may really need to build it on linux). When you are ready(you may want to install [`build-essential`](https://packages.debian.org/search?searchon=names&keywords=build-essential) or something like that from your package manager first), just type the following to build:

``` shell
git clone https://github.com/BLumia/lajihttpd.git --depth=1 # this repo.
git clone https://github.com/BLumia/libaji.git --depth=1 # the only dep, also by me.
cd libaji/src/
make # build libaji, binary located at `libaji/lib/`
cd ../../lajihttpd/
make # build this program, will generate `lajihttpd.elf`
```

After that, just run `lajihttpd.elf` and it's done. It will listening port 8080 by default. Dig into the config file if you like.

## Testing

For benchmark, you can using [ApacheBench](https://packages.debian.org/stretch/apache2-utils) to test, for example, you can do:

``` shell
ab -c4000 -n111010 http://127.0.0.1:8080/
```

This program may works better when you disable logging, or with keep-alive enabled with `ab` with `-k` argument (you should also ensure enable keep-alive support in config file).

## Other stuff

If you think this repo is still a little bit complex for you, checkout my previous [NaiveHTTPD](https://github.com/BLumia/NaiveHTTPD). That is much more simple than this one, and can also build with [cygwin](https://www.cygwin.com/). Whatever which one you want to learn, check the commit history is helpful.

ps. don't take seirous about the README of **libaji**, it's just for fun.

## License

[WTFPL](http://www.wtfpl.net/), I didn't attach a license file in this repo since it's not neccessary(imo).
