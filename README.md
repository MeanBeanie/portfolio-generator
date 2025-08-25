# Portgen

A quick and dirty static website generator. Uses a `markdown` and `ini` insprired formatting syntax to design and generate static websites with ease.

## Installation

If your device supports:
- [x] C11
- [x] Thats it
- [x] Nothing else is needed

Then all you need to do is take your C compiler of choice, mine is gcc, and run

`gcc *.c -o portgen`

## What it does

Using a simple formatting syntax, shown in the below example, it generates all the html needed to display a decent looking website. Its one downfal is that it does only generate the html, so you'll need to supply the css. An example css file is put below as well as in the `example` directory.

## Example Configuration

Do note that comments do not exist in the `port` syntax, and the one present below is merely to label the file

```
/* example.port */
% Website Title | Ishaan Mehta | TODO! Description

"home" > (index)
"github" > {https://github.com/MeanBeanie}
"TODO! add more links" > (index)

[index]
# "Header"
"this is the main page, it needs to exist"
"it has links to other pages too"
"go to page 1" > (page1)
"go to page 2" > (page2)

[page1]
% Project 1
~ Project 1: Doing this with that and this
# "Header"
"hello world, how are you!"
"This is page one, split up across multiple p elements"
"- this has a list"
"- it is unordered"
"- but it is still good"
## "Then code stuff"
`
// i'm a code block :3
// no syntax highlight though
#include <stdio.h>

int main(int argc, char* argv[]){
	for(int i = 0; i < argc; i++){
		printf("%s ", argv[i]);
	}
	
	return 0;
}
`
$image.png | a sample image of a giraffe

[page2]
% Project 2
~ Project 2: Doing that with this and that
# "Header"
"Neato this is page 2"
## "Subheader"
"this one even has a subheader"
```

```css
/* style.css */
body {
	font-size: 14px;
	background-color: #3b4252;
	color: #eceff4;
	overflow: scroll;
}
a {
	color: #81a1c1;
}
a:hover {
	color: #88c0d0;
}
.page_title {
	text-align: center;
}
.text_element_p {
	margin-left: 4%;
	width: 92%;
	overflow-wrap: break-word;
}
.text_element_h {
	text-align: center;
}
.text_element_box {
	margin-left: 9%;
	width: 82%;
	height: 100%;
}
hr {
	color: rgba(0, 0, 0, 0);
	border: 2px solid #8fbcbb;
	border-radius: 50px;
	width: 92%;
}
.nav_elem {
	color: #a3be8c;
}
.code {
	margin-left: 4%;
	width: 92%;
	font-family: monospace;
	color: #a3be8c;
}
img {
    width: 80%;
    height: 80%;
    margin-left: 9%;
}
```
