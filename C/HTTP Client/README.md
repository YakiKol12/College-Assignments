# TCP HTTP Client

Authord by Yaki Kol

## Files
client.c - A client side http 1.0 program sending requests to desired server.

## Remarks
- in POST requests the content sent are the first <n> chars from the given string where <n> is the
	content length specified in the request (if no <n> chars available, prints error and exit).
- arguments given can be provided with multiple '-r' insertion in the command line.
- please be aware that entering port option other than default '80' (or leaving it blank) might result in 
	program crash or entering an infinite loop.
