/* FileTypeResolver.cpp */
/*
 * Copyright (C) 2011-2024 Michael Lugmair
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
#include "FileTypeResolver.h"

#include <QFile>
#include <QString>

#include <taglib/mpegfile.h>

namespace
{
	template<typename T>
	auto tryFileType(const TagLib::FileName& fileName, bool readAudioProperties,
	                 TagLib::AudioProperties::ReadStyle audioPropertiesStyle) -> T*
	{
		auto* file = new T(fileName, readAudioProperties, audioPropertiesStyle);
		if(file->isValid())
		{
			return file;
		}

		delete file;
		return nullptr;
	}

	bool isAAC(const QString& filename)
	{
		auto f = QFile(filename);
		if(!f.open(QFile::ReadOnly))
		{
			return false;
		}

		auto bytes = f.read(2);
		f.close();

		bytes[1] = bytes[1] & static_cast<char>(0xF0);

		return ((bytes[0] == static_cast<char>(0xFF)) &&
		        (bytes[1] == static_cast<char>(0xF0)));
	}
}

namespace Tagging
{
	TagLib::File* FileTypeResolver::createFile(TagLib::FileName fileName, bool readAudioProperties,
	                                           TagLib::AudioProperties::ReadStyle audioPropertiesStyle) const
	{
		if(isAAC(fileName))
		{
			auto* file = tryFileType<TagLib::MPEG::File>(fileName, readAudioProperties, audioPropertiesStyle);
			if(file)
			{
				return file;
			}
		}

		return nullptr;
	}

	void FileTypeResolver::addFileTypeResolver()
	{
		static bool added = false;
		if(!added)
		{
			TagLib::FileRef::addFileTypeResolver(new FileTypeResolver());
			added = true;
		}
	}
}