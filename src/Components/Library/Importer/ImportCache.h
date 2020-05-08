/* ImportCache.h */

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

#ifndef IMPORTCACHE_H
#define IMPORTCACHE_H

#include "Utils/Pimpl.h"

#include <QList>
#include <QPair>

class MetaData;
class MetaDataList;

namespace Library
{
	/**
	 * @brief The ImportCache class
	 * @ingroup Library
	 */
	class ImportCache
	{
		PIMPL(ImportCache)

		public:
			explicit ImportCache(const QString& libraryPath);
			virtual ~ImportCache();

			ImportCache(const ImportCache& other);
			ImportCache& operator=(const ImportCache& other);

			void			clear();

			void			addFile(const QString& filename);
			void			addFile(const QString& filename, const QString& parentDirectory);

			QStringList		files() const;
			MetaDataList	soundfiles() const;
			int				count() const;
			int				soundFileCount() const;

			QString			targetFilename(const QString& srcFilename, const QString& targetDirectory) const;
			MetaData		metadata(const QString& filename) const;
			void			changeMetadata(const QList<QPair<MetaData, MetaData>>& changedTracks);

		private:
			void			addSoundfile(const QString& filename);
	};

	using ImportCachePtr=std::shared_ptr<ImportCache>;
}



#endif // IMPORTCACHE_H
