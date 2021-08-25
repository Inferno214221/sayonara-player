/* AbstractPlaylistParser.h */

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

#ifndef ABSTRACTPLAYLISTPARSER_H
#define ABSTRACTPLAYLISTPARSER_H

#include "Utils/Pimpl.h"

/**
 * @brief The AbstractPlaylistParser class
 * @ingroup PlaylistParser
 */
class AbstractPlaylistParser
{
	PIMPL(AbstractPlaylistParser)

	public:
		explicit AbstractPlaylistParser(const QString& filepath);
		virtual ~AbstractPlaylistParser();

		virtual MetaDataList tracks(bool forceParse = false) final;

	protected:
		virtual void parse() = 0;

		void addTrack(const MetaData& track);
		void addTracks(const MetaDataList& tracks);
		const QString& content() const;

		QString getAbsoluteFilename(const QString& filename) const;

		bool isParseTagsActive() const;
};

#endif // ABSTRACTPLAYLISTPARSER_H
