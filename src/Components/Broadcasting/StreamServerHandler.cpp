/* StreamServerHandler.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
 *
 * This file is part of sayonara player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "StreamServerHandler.h"
#include "StreamServer.h"

struct StreamServerHandler::Private
{
	StreamServer* stream_server=nullptr;

	Private() {}
	~Private()
	{
		if(stream_server){
			delete stream_server;
		}
	}
};

StreamServerHandler::StreamServerHandler()
{
	m = Pimpl::make<Private>();

	m->stream_server = new StreamServer();
}

StreamServerHandler::~StreamServerHandler() {}

void StreamServerHandler::active_changed() {}

void StreamServerHandler::port_changed()
{
	m->stream_server->restart();
}
