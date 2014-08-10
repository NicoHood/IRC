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

#include "IRC.h"

//================================================================================
// IRC Client
//================================================================================

IrcClient::IrcClient(void){
	_inStream = NULL;
	_outStream = NULL;
	_lastPing = 0;
	_ping = 0;
}

int IrcClient::connect(IPAddress ip){
	return connect(ip, IRC_PORT);
}
int IrcClient::connect(const char *host){
	return connect(host, IRC_PORT);
}

size_t IrcClient::write(const uint8_t *buf, size_t size){
	// this will print to the debug/log stream and the client
	if (_outStream != NULL) _outStream->write(buf, size);
	return EthernetClient::write(buf, size);
}

void IrcClient::begin(String user, String nick, String channel){
	_ircUser = user;
	_ircNick = nick;
	_ircChannel = channel;
	_lastPing = millis();
	_ping = 0;
}

void IrcClient::end(void){
	end("");
}

void IrcClient::end(String message){
	print("QUIT :");
	println(message);
	flush();
	stop();
}

void IrcClient::setStream(Stream &s){
	setStream(s, s);
}

void IrcClient::setStream(Stream &inS, Stream &outS){
	_inStream = &inS;
	_outStream = &outS;
}


void IrcClient::nick(String n){
	_ircNick = n;
	nick();
}

void IrcClient::nick(void){
	taskIrc();
	print("NICK ");
	println(_ircNick + String(random(100))); //TODO random
}

void IrcClient::user(String u){
	_ircUser = u;
	user();
}

void IrcClient::user(void){
	taskIrc();
	print("USER uk 8 * : ");
	println(_ircUser);
}

void IrcClient::join(void){
	taskIrc();
	print("JOIN ");
	println(_ircChannel);
}

void IrcClient::chat(String message){
	chat();
	println(message);
}

void IrcClient::chat(void){
	taskIrc();
	print("PRIVMSG ");
	print(_ircChannel);
	print(" :");
}

void IrcClient::task(void){
	taskIrc();
	taskIn();
}

void IrcClient::taskIrc(void){

	if (_outStream == NULL) return; //TODO

	// reads all bytes from the irc and print them to the stream
	if (available()){
		_outStream->println("=========IN=========");

		String message = "";
		while (available()) {
			char c = read();
			message += c;
			if (c == '\n'){
				// logging
				if (_outStream)
					_outStream->print(message);

				// weve got a full message, analyze now
				if (!ircMessage(message) && _outStream)
					_outStream->println("=======ERROR=======");

				message = "";
			}
		}
		_lastPing = millis();
		_ping = 0;
	}


	unsigned long time = millis();
	if (time - _lastPing > IRC_PING){
		println("PING :irc.NoNSeNSe.IRC-Mania.net");
		_lastPing = time;
		_ping++;
	}
	if (_ping > (IRC_TIMEOUT / IRC_PING + 1)){
		if (_outStream)
			_outStream->print("=======NOPING=======");
		end();
	}

}

bool IrcClient::ircMessage(String message){
	String from;
	String type;
	String to;
	String rest;

	int space3;

	// from
	int space = message.indexOf(' ');
	if (space == -1)
		return false;
	else
		from = message.substring(0, space);

	if (message[0] == ':'){
		// type
		int space2 = message.indexOf(' ', space + 1);
		if (space2 == -1)
			return false;
		else
			type = message.substring(space + 1, space2);

		// to
		space3 = message.indexOf(' ', space2 + 1);
		if (space3 == -1)
			return false;
		else
			to = message.substring(space2 + 1, space3);
	}
	else space3 = space;

	// rest
	rest = message.substring(space3 + 1, message.length() - 2);

	// printing
	if (0){
		if (from.length()){
			Serial.print("<");
			Serial.print(from);
			Serial.println(">");
		}

		if (type.length()){
			Serial.print("<");
			Serial.print(type);
			Serial.println(">");
		}

		if (to.length()){
			Serial.print("<");
			Serial.print(to);
			Serial.println(">");
		}

		if (rest.length()){
			Serial.print("<");
			Serial.print(rest);
			Serial.println(">");
		}
	}

	// PING check
	if (from == "PING"){
		print("PONG ");
		println(rest);
	}

	if (type == "PRIVMSG"){
		if (rest.startsWith(":!Nico"))
			chat("Hood");
		if (rest.startsWith(":!Bier"))
			chat("Bier für alle! *Prost*");
		if (rest.startsWith(":!Ping"))
			chat("Pong!");
		if (rest.startsWith(":!Keks"))
			chat("Kekse sind leider alle :(");
	}

	return true;
}


void IrcClient::taskIn(void){
	// do not read if there is no stream defined/available
	if (_inStream == NULL || !_inStream->available()) return;

	// if message start with "!" it will send a raw command
	if (_inStream->peek() == '!') {
		if (_outStream)
			_outStream->println("=======COMMAND======");
		// discard this '!' byte
		_inStream->read();
	}
	else {
		// print text message to irc
		if (_outStream)
			_outStream->println("========CHAT========");
		// sends a chat beginning message. always use a linefeed after!
		chat();
	}

	// print all data to the serial and irc
	while (_inStream->available())
		write(Serial.read());

	// add the linefeed
	println();
}

