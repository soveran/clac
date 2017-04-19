# Clac

A command line, stack-based calculator with postfix notation that
displays the stack contents at all times. As you type, the stack
changes are reflected immediately.

![clac in action](http://files.soveran.com/misc/clac.gif)

In a stack-based postfix calculator, entering a number pushes it
on a stack, and arithmetic operations pop their arguments from the
stack and push the result. As all the operations take a fix number
of arguments, there's no room for ambiguity: parenthesis and operator
precedence are not needed. Postfix notation is also known as reverse
Polish notation, or RPN.

Why?
----

I like Forth and other stack based, concatenative languages, and
I've used the `dc` calculator a lot. Once I got used to `dc`, other
calculators seemed to be less powerful. While it has been extremely
useful for me, I've always found the UI a bit lacking. That's why
I decided to try this idea of an always-visible stack with realtime
updates.

Usage
-----

If invoked without arguments, clac starts in interactive mode. If
an expression is provided, clac will process it and print each
element in the stack starting from the top. It will then exit
immediately.

Here are some examples of non-interactive invocations:

```terminal
$ clac "3 4 +"
7
```

The following example illustrates what happens when there's more
than one element left in the stack:

```terminal
$ clac "2 3 4 +"
7
2
```

When clac finishes evaluating the expression "2 3 4 +", there are
two elements in the stack: the number 7 at the top of the stack and
the number 2 at the bottom of the stack. The elements are printed
in order, one per line, starting from the top of the stack.

Installation
------------

Download the [latest release][releases] or check the available
[packages][packages]. You can also compile the source code in the
master branch.

[releases]: https://github.com/soveran/clac/releases
[packages]: https://github.com/soveran/clac/wiki/Distribution-Packages

Install clac into `/usr/local/bin` with the following command:

    $ make install

You can use `make PREFIX=/some/other/directory install` if you wish
to use a different destination. If you want to remove clac from
your system, use `make uninstall`.

Commands
--------

When a command requires an argument, it pops a value from the stack.
If the stack is empty, a zero is provided instead. In the descriptions
below, the top of the stack (and thus the first value popped) is
represented by the letter `a`, while the second value popped is
represented by the letter `b`. For example, if the stack is composed
of the number `1`, `2` and `3` (with `3` at the top of the stack),
when we describe the sum then `a` will be `3` and `b` will be `2`.
It is important to note that substraction and division invert the
order of the arguments before peforming the operation: with `1`,
`2` and `3` in the stack, when you type `-` it will pop the values
`3` and `2` and push the result of `2 - 3`. This is in the tradition
of other postfix calculators and programming languages.

Here's a description of the available commands:

### Arithmetic operations

`+` 
Pop two values `a` and `b` and push the result of `a + b`.

`-` 
Pop two values `a` and `b` and push the result of `b - a`.

`*` 
Pop two values `a` and `b` and push the result of `a * b`.

`/` 
Pop two values `a` and `b` and push the result of `b / a`.

### Modulo operation

`%`
Pop two values `a` and `b` and push the remainder of the Euclidean
division of `b` by `a`.

### Exponentiation

`^` 
Pop two values `a` and `b` and push the result of `b ^ a`.

### Rounding

`ceil`
Pop the value `a` and push smallest integral value greater than or
equal to `a`.

`floor`
Pop the value `a` and push largest integral value less than or equal
to `a`.

`round`
Pop the value `a` and push integral value nearest to `a`.

### Stack manipulation

`swap` 
Pop two values `a` and `b` and push the values `a`, `b`.

`dup`
Pop the value `a` and push the values `a`, `a`.

`_` 
Push on the stack the result of the last operation.

### What is missing

It is likely that more operations will be added in the future. I
didn't add every possible stack manipulation command because I'm
trying to let usage define the API, and the fact that the calculator
has line editing capabilities makes some commands less useful. For
example, `drop` is trivial to implement (two lines of code), yet I
didn't include it in the API because I haven't had the need to use
it so far. I like to keep the code short, that's why I didn't go
ahead implementing commands just in case. That said, let me know
if you find use cases for other commands (or even for `drop`!) and
we will either find a better solution or we'll end up implementing
them.

User defined operations
-----------------------

It is possible to define operations (or _words_, as they are usually
called in stack based programming languages) by editing the `words`
configuration file. It is not created by default, but clac will use
some environment variables in order to search for word definitions.

`$CLAC_WORDS`
If set, it should point to a file containing word definitions.

`$XDG_CONFIG_HOME`
If set, clac will search for `$XDG_CONFIG_HOME/clac/words`.

`$HOME`
If set, clac will search for `$HOME/.config/clac/words`.

### How to define words

Words are defined as aliases, with one alias on each line. Empty
lines are ignored. Here are some examples:

```
pi 3.141592
tau "pi 2 *"
sqrt "0.5 ^"
```

Then you can use them as if they were built-in commands:

```
$ clac "42 dup * pi *"
5541.76
```


Contributing
------------

If you find a bug, please create an issue detailing the ways to
reproduce it. If you have a suggestion, create an issue detailing
the use case.
