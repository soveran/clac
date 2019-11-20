#!/bin/sh

export CLAC_WORDS=./test/words

assert_equal () {
	test "$1" = "$2" || printf "F: \"%s\" != \"%s\"\n" $1 $2
}

# Single argument, positive number
assert_equal "3" `./clac 3`

# Single argument, negative number
assert_equal "-3" `./clac -3`

# Basic arithmetic
assert_equal "7" `./clac "3 4 +"`

# Modulo
assert_equal "2" `./clac "7 5 %"`

# Summation
assert_equal "10" `./clac "1 2 3 4 sum"`

# User defined word
assert_equal "3.141592" `./clac pi`

# Word defined based on user defined word
assert_equal "6.283184" `./clac tau`

# Not found words starting with digits result in nan
assert_equal "nan" `./clac 3+`

# Not found words starting with alpha are ignored
assert_equal "" `./clac foo`

# Argument error (too many arguments)
assert_equal "" `./clac 1 2 2> /dev/null`

# Stashing numbers
assert_equal "21" `./clac "4 3 9 . * , +"`

# Stashing everything
assert_equal "" `./clac "1 2 3 :"`

# Stashing and restoring everything
assert_equal "6" `./clac "1 2 3 : ; sum"`

# Partial add
assert_equal "7" `./clac "1 2 3 4 2 add"`
assert_equal "10" `./clac "1 2 3 4 count add"`
assert_equal "10" `./clac "1 2 3 4 count 1 + add"`

# Roll
assert_equal "3" `./clac "1 2 3 4 3 1 roll"`
assert_equal "2" `./clac "1 2 3 4 3 1 roll ."`
assert_equal "4" `./clac "1 2 3 4 3 1 roll . ."`
assert_equal "1" `./clac "1 2 3 4 3 1 roll . . ."`
assert_equal "3" `./clac "1 2 3 4 4 1 roll"`
assert_equal "1" `./clac "1 2 3 4 4 -1 roll"`
assert_equal "3" `./clac "1 2 3 4 4 5 roll"`
assert_equal "1" `./clac "1 2 3 4 4 -5 roll"`
assert_equal "1" `./clac "1 2 1 roll"`
assert_equal "" `./clac "1 2 roll"`
assert_equal "" `./clac "1 roll"`

# Rounding
assert_equal "2" `./clac "2.1 round"`
assert_equal "2" `./clac "2.1 floor"`
assert_equal "3" `./clac "2.1 ceil"`
