# ffind
Fast, multithreaded find implementation written in C99.

## Getting Started

### Dependencies
ffind will only build on a system that has all of the following:
* POSIX-compatibible OS (OSX, Cygwin, Linux, BSD, etc.)
* gcc
* make
* PCRE library

### Building the project

To build and run the executable:
```shell
make
./ffind
```

To build and run a debug version of the executable:
```shell
make debug
./ffind
```

To build and view the documentation (requires Doxygen):
```shell
doxygen
firefox ./docs/html/globals.html
```

## Roadmap
* POSIX conformance

## License
This project is licensed under the MIT License. See [LICENSE.txt](LICENSE.txt) for details.
