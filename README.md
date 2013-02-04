# A simple C http client

This is just a simple implementation of an HTTP client written in C.  It uses the included library to initialize and build a dynamically growable buffer.

## Dependencies

* [uriparser](http://uriparser.sourceforge.net/) -- TODO: build instructions

## Installation

Once all dependencies are installed, run these commands to build included dependencies and the client itself:

    $ git submodule init && git submodule update
    $ cd vendor/buffer && make && cd -
    $ make

## Usage

The client takes a single parameter -- the URL to fetch:

    $ ./bin/http http://example.com/foobar.html
