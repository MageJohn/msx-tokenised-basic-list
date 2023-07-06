# msx-tokenised-basic-list

This is a program to read a [tokenised MSX-BASIC](https://www.msx.org/wiki/MSX-BASIC_file_formats#Tokenized_BASIC) file. It was originally written by Vincent van Dam, back in 1997 (the year I was born, incidentally).

I have since substantially rewritten it to fix some bugs I found, modernising the code along the way (to C99, if that even counts as modern). What's amazing to me is that this work was not necessary to get this to compile and run, it did that just fine already.

Vincent's original code is in the first commit to this repository, and is available on his website, http://home.kabelfoon.nl/~vincentd/.

## Known issues

- May not correctly handle graphic symbols
- May not handle a few commands from versions of MSX-BASIC greater than 2

I might add these as I come across a need for them. If I've not done so and you need them, I'm happy to accept PRs!

