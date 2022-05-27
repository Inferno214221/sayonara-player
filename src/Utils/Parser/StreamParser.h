/* StreamParser.h */

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

#ifndef STREAMPARSER_H
#define STREAMPARSER_H

#include "Utils/Pimpl.h"
#include <QObject>

using PlaylistFiles = QStringList;

class StreamParser :
	public QObject
{
	Q_OBJECT
	PIMPL(StreamParser)

	signals:
		void sigFinished(bool success);
		void sigStopped();
		void sigUrlCountExceeded(int urlCount, int maxUrlCount);

	public:
		explicit StreamParser(QObject* parent = nullptr);
		~StreamParser() override;

		void parse(const QString& stationName, const QString& stationUrl,
		           int timeout = 5000); // NOLINT(readability-magic-numbers)
		void parse(const QStringList& urls, int timeout = 5000); // NOLINT(readability-magic-numbers)

		void setCoverUrl(const QString& coverUrl);

		void stop();
		[[nodiscard]] bool isStopped() const;

		[[nodiscard]] MetaDataList tracks() const;

	private slots:
		void awaFinished();
		void icyFinished();

	private: // NOLINT(readability-redundant-access-specifiers)
		[[nodiscard]] QPair<MetaDataList, PlaylistFiles> parseContent(const QByteArray& data) const;
		[[nodiscard]] QPair<MetaDataList, PlaylistFiles> parseWebsite(const QByteArray& arr) const;

		void setMetadataTag(MetaData& metadata, const QString& streamUrl, const QString& coverUrl = QString()) const;
		bool parseNextUrl();
};

#endif
