/* id3.h */

/* Copyright (C) 2011-2017 Lucio Carreras
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

#ifndef TAGGING_H_
#define TAGGING_H_

#include "TaggingEnums.h"

namespace TagLib { class FileRef; }

class QString;
class QPixmap;
class MetaData;
class QString;
class QByteArray;

namespace TagLib
{
	class FileRef;
}

/**
 * @brief Tagging namespace
 * @ingroup Tagging
 */
namespace Tagging
{

	namespace Utils
	{
		/**
		 * @brief get metadata of file. Filepath should be given within the MetaData struct
		 * @param md MetaData that will be filled
		 * @param quality fast, normal, accurate
		 * @return true, if metadata could be filled. false else
		 */
		bool getMetaDataOfFile(MetaData& md, Tagging::Quality quality=Tagging::Quality::Standard);

		/**
		 * @brief writes metadata into file specivied in MetaData::_filepath
		 * @param md MetaData struct to write
		 * @return true if metadata could be written. false else
		 */
		bool setMetaDataOfFile(const MetaData& md);

		bool is_valid_file(const TagLib::FileRef& f);

		Tagging::TagType get_tag_type(const QString& filepath);
		QString tag_type_to_string(Tagging::TagType);
		Tagging::ParsedTag tag_type_from_fileref(const TagLib::FileRef& f);
	}
}

#endif
