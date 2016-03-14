# Emphasis

*italic*   **bold**

_italic_   __bold__

# Headers

# Header 1

## Header 2

### Header 3
# Lists

## Unordered List:

* Item 1
* Item 2
    + Item 2a
    + Item 2b

## Ordered List:

1. Item 1
2. Item 2
3. Item 3
    + Item 3a
    + Item 3b

# R Code Chunks

## R code will be evaluated and printed

```{r}
summary(cars$dist)
summary(cars$speed)
```
## Inline R Code

There were `r nrow(cars)` cars studied
Links

# Use a plain http address or add a link to a phrase:

http://example.com

[linked phrase](http://example.com)

# Images

## Images on the web or local files in the same directory:

![alt text](http://www.cpe.sjtu.edu.cn/themes/webhtm/images/email.jpg)


# Blockquotes

## A friend once said:

> It's always better to give
> than to receive.
Plain Code Blocks

## Plain code blocks are displayed in a fixed-width font but not evaulated

```
This text is displayed verbatim / preformatted
```
## Inline Code

We defined the `add` function to
compute the sum of two numbers.
LaTeX Equations
LaTeX Equations

## Inline equation:

$equation$
Display equation:

$$ equation $$
Horizontal Rule / Page Break

# Three or more asterisks or dashes:

******

------

# Tables

First Header  | Second Header
------------- | -------------
Content Cell  | Content Cell
Content Cell  | Content Cell
Reference Style Links and Images
Links

A [linked phrase][id].
At the bottom of the document:

[id]: http://example.com/ "Title"
Images
![alt text][id]
At the bottom of the document:

[id]: figures/img.png "Title"
Manual Line Breaks

End a line with two or more spaces:

Roses are red,
Violets are blue.
Miscellaneous

superscript^2^

~~strikethrough~~
