% CLAC(1)
% Michel Martens
% 2017

# NAME

clac – A command line, stack-based calculator with postfix notation

# SYNOPSIS

**clac**

# DESCRIPTION

**clac** is a command line, stack-based calculator with postfix notation that
displays the stack contents at all times.
As you type, the stack changes are reflected immediately.

In a stack-based postfix calculator, entering a number pushes it
on a stack, and arithmetic operations pop their arguments from the
stack and push the result.
As all the operations take a fix number of arguments, there's no room for
ambiguity: parenthesis and operator precedence are not needed. Postfix notation
is also known as reverse Polish notation, or RPN.

# Commands

When a command requires an argument, it pops a value from the stack.
If the stack is empty, a zero is provided instead.
In the descriptions below, the top of the stack (and thus the first value
popped) is represented by the letter `a`, while the second value popped is
represented by the letter `b`.
For example, if the stack is composed of the number `1`, `2` and `3` (with `3`
at the top of the stack), when we describe the sum then `a` will be `3` and `b`
will be `2`.
It is important to note that subtraction and division invert the
order of the arguments before performing the operation: with `1`,
`2` and `3` in the stack, when you type `-` it will pop the values
`3` and `2` and push the result of `2 - 3`. This is in the tradition
of other postfix calculators and programming languages.

A description of the available commands follows.

# Arithmetic operations

`+` 
Pop two values `a` and `b` and push the result of `a + b`.

`-` 
Pop two values `a` and `b` and push the result of `b - a`.

`*` 
Pop two values `a` and `b` and push the result of `a * b`.

`/` 
Pop two values `a` and `b` and push the result of `b / a`.

# Modulo operation

`%`
Pop two values `a` and `b` and push the remainder of the Euclidean
division of `b` by `a`.

# Exponentiation

`^` 
Pop two values `a` and `b` and push the result of `b ^ a`.

# Rounding

`ceil`
Pop the value `a` and push smallest integral value greater than or
equal to `a`.

`floor`
Pop the value `a` and push largest integral value less than or equal
to `a`.

`round`
Pop the value `a` and push integral value nearest to `a`.

# Stack manipulation

`swap` 
Pop two values `a` and `b` and push the values `a`, `b`.

`dup`
Pop the value `a` and push the values `a`, `a`.

`_` 
Push on the stack the result of the last operation.

