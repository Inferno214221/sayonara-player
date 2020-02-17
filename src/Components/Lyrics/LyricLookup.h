/* LyricLookup.h */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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


/*
 * LyricLookup.h
 *
 *  Created on: May 21, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#ifndef LYRICLOOKUP_H_
#define LYRICLOOKUP_H_

#include <QObject>

#include "Utils/Pimpl.h"

namespace Lyrics
{
	class Server;

	/**
	 * @brief The LyricLookupThread class
	 * @ingroup Lyrics
	 */
	class LookupThread :
			public QObject
	{
		Q_OBJECT
		PIMPL(LookupThread)

	signals:
		void sigFinished();

	public:
		explicit LookupThread(QObject* parent=nullptr);
		virtual	~LookupThread();

		QString	lyricData() const;
		QString lyricHeader() const;
		QStringList servers() const;

		void run(const QString& artist, const QString& title, int serverIndex);
		void stop();

		bool hasError() const;

	private:
		void initServerList();
		void initCustomServers();

		void addServer(Server* server);
		void startSearch(const QString& url);
		void callWebsite(const QString& url);


	private slots:
		void contentFetched();
		void searchFinished();
	};
}

#endif /* LYRICLOOKUP_H_ */
