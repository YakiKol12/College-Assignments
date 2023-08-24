# HTTP Server

Authord by Yaki Kol

## Files
threadpool.h
threadpool.c - a generic framework of creating given amount of threads and assigining work to them.<br>
server.c - A server side http 1.0 program recieveing requests from any scorce (browser, telnet, etc)
	returns the desired path given the user has read permissions.

## Remarks
- Do note that browsers create "behind the scenes" requests like favicon,
	empty requests to try keep the connection alive results in less requests granted than specified
	in the execute variables given (max requests).

