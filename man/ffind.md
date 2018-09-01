ffind(1) -- Quickly searches for files.
==========================================================================

## SYNOPSIS

`find [-eHij4lLP] [directories...] [pattern]`

## DESCRIPTION

**ffind** is a multithreaded replacement for POSIX find. It recursively searches directories for files matching a certain pattern. If a directory is not specified, **'.'** is assumed. If a pattern is not given, **'*'** is used.

## OPTIONS

* `-e` :
	Support escaping **'\*'** with **'\\\*'** in the **-name** parameter. Escape characters are automatically supported in the **-regex** parameter, so this option is not needed in that case.


* `-H` :
	Do not follow symbolic links. This is the default option.


* `-I` :
	Ignore case in the **-name** or **-regex** parameters.


* `-jN` :
	Use *N* threads when searching. For example, use **-j8** to use 8 threads.


* `-l` :
	Match all characters in the **-name** parameter literally. In this case, **ffind** matches if the **-name** parameter is a substring of the full path.


* `-L` :
	Follow symbolic links.


* `-P` :
	Identical to **-H**.


If more than one of **-H**, **-L**, or **-P** is specified, the last one takes effect.


* `--help` :
	Prints the help menu and exits.


* `-maxdepth N` :
	Limit the maximum recursion depth to *N*. For example, **-maxdepth 2** will limit **ffind** to 2 subfolders.


* `-name PATTERN` :
	Print only the files matching this *PATTERN*. The **'\*'** character can be used to match 0 or more of any character. The **fnmatch(3)** function call is used to perform this match.


* `-print0` :
	Seperate entries with **'\\0'** instead of **'\\n'**. Useful for piping to **xargs -0**.


* `-regex REGEXP` :
	Print only the files matching this *REGEXP*. The default regex dialect is 'posix-basic'. Use **-regextype** to use a different one.


* `-regextype TYPE` :
	Use a different regex dialect. Use **-regextype help** to see all available dialects.


* `-type C` :

	Print only listings matching one of the following types:


	`d`	Directory


	`f`	File


* `--version` :
	Displays **ffind**'s version and exits.

## AUTHOR

Jonathan Lemos (jonathanlemos@protonmail.com)

## COPYRIGHT

**ffind** is Copyright (C) 2018 Jonathan Lemos. **ffind** is released under the MIT License. See [LICENSE.txt](LICENSE.txt) for details.

## SEE ALSO

find(1)
