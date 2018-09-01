# ffind
Fast, multithreaded find implementation written in C99.

## Getting Started

### Dependencies
ffind will only build on a system that has all of the following:
* POSIX compatibillity (OSX, Cygwin, Linux, BSD, etc.)
* gcc (To use a different C compiler, run with `$ make CC=your-cc`)
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

To build and run the tests:
```shell
make test
./test
```

## Roadmap
* POSIX conformance

## License
This project is licensed under the MIT License. See [LICENSE.txt](LICENSE.txt) for details.
