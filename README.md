# utf8test

A console (terminal) application to print all unicode characters in
UTF-8 encoding. It is useful to test the completeness of fonts and
generally to look up some unicode characters.

## Syntax

```
utf8text
```

At the moment the program has only one mode of operation and will print
one character per line and show

* the `U+XXXX` notation in first column
* the raw UTF-8 character in second column (this is the character itself,
	which is shown if the current font supports it)
* following are the bytes in hexadecimal

## Sample output (shortened)

```
U+0000  ^@      00
U+0001  ^A      01
U+0002  ^B      02
U+0003  ^C      03
[...]
U+0040  @       40
U+0041  A       41
U+0042  B       42
U+0043  C       43
U+0044  D       44
U+0045  E       45
[...]
U+0080  <U+0080>        c2 80
U+0081  <U+0081>        c2 81
U+0082  <U+0082>        c2 82
U+0083  <U+0083>        c2 83
[...]
U+0391  Α       ce 91
U+0392  Β       ce 92
U+0393  Γ       ce 93
U+0394  Δ       ce 94
U+0395  Ε       ce 95
[...]
```

## Remarks

* Please make sure that your console/terminal is set to UTF-8, otherwise
the output probably won't make any sense for you.
* It is advisable to pipe the output to a pager to slow down the output.
