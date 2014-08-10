/*
Copyright (c) 2014 NicoHood
See the readme for credit to other people.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/*
commands:
Bier
Vodka
komasaufen

*/



#ifndef IRC_h
#define IRC_h

//================================================================================
// IRC Client
//================================================================================

// include Ethernet library for all functions
#include <Ethernet.h>
#include <Arduino.h>

// default irc port is 6667
#define IRC_PORT 6667
#define IRC_PING 60000
#define IRC_PONG 3
#define IRC_TIMEOUT 300000

//TODO
// flush: add streams too?

class IrcClient : public EthernetClient {
public:
	IrcClient(void);

	// new overload that you dont need to enter the port
	using EthernetClient::connect;
	int connect(IPAddress ip);
	int connect(const char *host);

	// new overload that prints the data to a debug or log stream too
	using EthernetClient::write;
	size_t write(const uint8_t *buf, size_t size);

	// opens/closes an irc connection
	void begin(String user, String nick, String channel); //TODO
	void end(void);
	void end(String message);

	// stream for debug in/output or data logging
	void setStream(Stream &s);
	void setStream(Stream &inS, Stream &outS);

	// holds the irc alive and responds to various commands
	void task(void);
	void taskIrc(void);
	void taskIn(void);

	// interact with the server
	void nick(String n);
	void nick(void);
	void user(String u);
	void user(void);
	void join(void); // leave TODO
	void chat(void);
	void chat(String message);

private:
	bool ircMessage(String message);

	Stream* _inStream;
	Stream* _outStream;

	unsigned long _lastPing;
	uint8_t _ping;

	String _ircNick;
	String _ircUser;
		String _ircChannel;

};

#endif

