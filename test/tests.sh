#!/bin/sh

export CLAC_WORDS=./test/words

assert () {
	test "$1" = "$2" || printf "F: \"%s\" != \"%s\"\n" $1 $2
}

assert "7" `./clac "3 4 +"`
assert "3.14159" `./clac pi`
assert "6.28318" `./clac tau`
assert "3" `./clac 3`
assert "-3" `./clac -3`
assert "" `./clac 1 2 2> /dev/null`
assert "nan" `./clac 3+`
assert "" `./clac foo`
