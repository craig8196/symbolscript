
# Symbol Script (.sym)

Symbol Script is all about symbols.
The language is focused on consistency and symbology.
Every token is a symbol.
The language, as a result, is able to talk about and modify itself.
A fun consequence about this is the ability to export
Abstract Syntax Trees (ASTs) into multiple languages.
Another fun consequence is the ability to perform mutations and 
experiments on this language, within this language.
Typing and annotations make this a powerful tool.
Also, it makes for a great replacement to other scripting languages.

As it stands: this is a stupid language, don't use it.
But at least I'll learn something from it.


## TODO
- [ ] Remove cmake files and only do meson.build?


## Motivation

I don't like BASH, or other C-Shell type command-line languages.
I wanted the basic convenience of BASH, but better syntax for complex logic.
I like Haskell's syntax, but the learning curve was steep.
I also like Elm.
But the simplicity of basic BASH, Python, and Javascript cannot be denied
their success.
I eventually dreamt up this language.
And gave it a stupid name.


## Goals and Non-Goals

Goals:
1. Consistent and simple interpreted language.
1. Rich tool set.
1. Meta capabilities to control, modify, build, manipulate the language.
1. No difference between functions and macros.
   Building a macro should be as simple as building a function.
   This isn't to say there is reduced complexity when building a macro,
   but rather that you build a macro with the same rules of the language.
1. Organized libraries, but tight specs within each grouping of code.
   This goal is adherence to Unix design principles.
   Each tool does one thing, and it does it well.
   Basically, don't try to code everything and the kitchen sink.

Non-Goals:
1. Speed, the interpreter should interpret quickly, but speed is not a goal.
   Don't confuse this with outputing bytecode or having optimizations availalble.
   This is just in reference to the interpreter.
1. Memory efficiency.
   Again we'll do what we can, but optimizations of generated code are
   more critical.


## Rules

Every language has rules.

The first feature of the language is apply.
Apply calls and executes the first function it comes across, with the highest precedent.

The built-in function bind (=) is the core feature of the language.
Bind takes a symbol and a sequence of symbols;
the sequence of symbols is assembled into an AST;
then the AST is bound to the symbol.
Every sequence, separated by white space, is a symbol.
How do we know to stop feeding symbols into the function bind?
We don't, but bind does.
Bind returns the symbol that it stopped at.
The interpreter then looks at the rest and executes the first function it finds.
And so forth.

Compile is the next function.
This generates Symbol Script Byte Code (SSBC).
This can speed up execution at the cost of descriptive code.

Every return value from a function that executes is printed to the console.
Function return values can be suppressed via a flag.

This language can likely implement other languages.
Some limitations may apply.

When parsing there are four tokens.
1.  Whitespace
1.  Symbol
1.  Newline
1.  String/Byte-String

There is a super-token called a block.
A block is denoted by it's indentation compared to the context of previous lines.

Even Comments are made with a stream of symbols.


## Architectural Design

UTF-8 Checker >>> Liner >>> Lexer >>> Parser >>> Executor
Lexer = Tokenizer >>> Grouper >>> Resolver


### UTF-8 Checker

We aren't going to just accept anything under the sun.
UTF-8 is nearly ubiquitous.
Why bloat our code base trying to support everything?
You can code your own text converters if you'd like.


### Liner

Accumulates text into lines.


### Lexer

#### Tokenizer

Chops up the text.
Text MUST be inserted by the line, no partial lines.
The end of the text is assumed to be a newline.


#### Grouper

Groups tokens.
This allows us to associate blocks (aka contexts).
This relies on the spacing before the first whitespace token.


#### Resolver

Resolves what a symbol points to given its context.


### Parser

Assembles the code into a parse tree that can then be executed, inspected,
or further compiled into bytecode.


### Executor

The executor will execute anything that needs to be run at 'compile time'.


## Machine Specific Optimizations

There are modules for determining certain features of your operating system.
Like the number of cores, threads, cache, memory, storage, etc.
If you base values off of these you can create machine specific optimizations.
If you load into memory according to the cache size to perform certain
operations to take advantage of cache coherancy then you can increase your
speed of execution.


## Examples

```
# This is a comment, comments are actually really weird in this language
# A space MUST follow the # sign, everything else after it is an argument
# Yes, comments are functions
# They are functions that do nothing
# They take all tokens as arguments until the end of the line

###
  This is a multi-line comment
  Everything must be indented two spaces in the block following it

  Still part of the previous comment, because of the same indent level
  The blank line gets ignored

a = lambda a
      # still a comment
      return a

# This is a function
function var1 var2
         var3
# Yes var3 must align with var1 because it is on a new line
# Yes this language is whitespace aware

# This is a weird function '='
# Note that it is infix
# Infix can only have one argument to the left and many to the right
# 'int' is a function that takes a symbol
# 'int' will parse the symbol at 'compile time'
# '=' takes a symbol on the left and binds the things on the right to it
value = int 3
# 'value' is now a function returning the value of 'int 3' which is an
# integer with value 3

# we now call the function 'value', it returns '3' into the void
value
# we can update and rebind the value
value = value + int 4
# '+' takes only arguments of equivalent integral types, '+' is a class
# such that a + b must meet type a == type b

# '=' is a convenience function for 'bind', 'bind' requires the context
# that you are binding to, the symbol, and the thing being bound to a symbol
# 'bind' is more useful for creating tools, not for common usage
bind . value int 4

# 'let' is a special form of bind, the symbol becomes a constant
let a = int 4

# let's look at the functions 'func' and '->'
# 'func' takes a symbol, a function specification, and a Block of code
# The block of code is two spaces in from 'func'
func b ( a : int  b : int ) -> void
  print ( a + b )


# Yes, 'func', 'let', 'bind', '=', all return the symbol they are bound to
# the following is valid
let c let a = int b
# a == c should return true

# Let's look at lambda, this takes one symbol and a Block of code
# 'return' explicitly labels the function's return type
# 'return' is needed here to ensure that the return type of the function
# meets the type needed by sort
# 'return' labels the function type and returns the value specified
# In most functions the return type is specified up-front and is redundant;
# the last value in a function becomes the returned value
# 'return' does help when happening in the middle of a function
# Note that the function body is two spaces indented from 'lambda'
# Note that lambda will return a random name that doesn't conflict with 
# anything in the context
sort list lambda pair
            return compare ( left pair ) ( right pair )

# One annoying consequence of single letter functions:
# You need two spaces after them if they take a Block/RawBlock
# Here we have 'post' that takes a URL, added header info, and a payload
# in the form of a function body
# The extra space helps the grouper to differentiate between arguments and a block
let post p
# If we put all arguments on one line, there is no confusion
p  "example.com"
   { headerKey : headerValue }
  "Payload"

# Let's mess up
# Observe that b is not indented properly
# This results in a lexer error
# So what do we do?
# The lexer will observe that you were binding, and if you are in
# an interactive environment the value a will now contain your lexer
# error in detail
# If there is nothing to bind the error to, then you simply get invalid output
# If not in an interactive environment, we crash
a = int
  b
# Correct variation
a = int
        b
# Obviously we would want to just put it on one line

# You CANNOT have a function that takes more than 8 arguments
# Seriously, create a struct or something
# Also, named arguments just make a mess of things, create a map/struct
func a ( b c d e f g h i errorHere) -> void
  "This will not return" # and this shows that the string is ignored as the return value b/c it's not 'void'
```


