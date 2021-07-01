# mygrep
My attempt at a regex engine / grep utility.

This was a rewrite of a term assignment which accrued a certain amount of technical debt.

## Regex Syntax

Normal characters are matched sequentially.
For instance, the regex `abc`, would expect to match 'a', then 'b', then 'c'.

### Special Characters

| Special Character | Meaning                    |
|-------------------|----------------------------|
| `.`               | Any character              |
| `\s`              | Whitespace                 |
| `\w`              | Alphabhatic                |
| `\W`              | Non-alphabhatic            |
| `\d`              | Digit                      |
| `\D`              | Non-digit                  |


### Pattern Sets

Anything deliminated by `[` and `]` is a matching set.
For instance, `[\s\dx]` matches any whitespace character, digit, or the literal character 'x'.

If the first character in the set is `^`, then the matching is negated.
For instance, `[^\s\dx]` matches anything that ISN'T a white space character, digit, or the literal 'x'.

### Repitions

Patterns can be repeated.
The repition `*` matches the pattern 0 or more times.
For instance, `x*` matches '' (the empty string), 'x', 'xx', etc.

| Repition    | Meaning                               |
|-------------|---------------------------------------|
| `*`         | O or more times                       |
| `+`         | 1 or more times                       |
| `?`         | 0 or 1 times                          |
| `{n}`       | Exactly n times                       |
| `{n,m}`     | At least n times, and at most m times |
| `{n,}`      | At least n times                      |

### Matching Groups

Anything surrounded by `(` and `)` is a matching group.
The text that they match is captured and can be returned.
They are also repeated in a group, so that `(xy)+` matches 'xy', 'xyxy', 'xyxyxy', etc.

