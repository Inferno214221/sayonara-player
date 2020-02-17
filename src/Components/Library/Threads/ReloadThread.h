/* ReloadThread.h */

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
 * ReloadThread.h
 *
 *  Created on: Jun 19, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#ifndef RELOADTHREAD_H_
#define RELOADTHREAD_H_

#include "Utils/Library/LibraryNamespaces.h"
#include "Utils/Pimpl.h"

#include <QThread>
#include <QDir>

namespace Library
{
	class ReloadThread :
			public QThread
	{
		Q_OBJECT
		PIMPL(ReloadThread)

	signals:
		void sigReloadingLibrary(const QString& message, int progress);
		void sigNewBlockSaved();

	public:
		explicit ReloadThread(QObject* parent);
		~ReloadThread() override;

		void pause();
		void goon();
		void stop();
		bool isRunning() const;
		void setQuality(ReloadQuality quality);
		void setLibrary(LibraryId id, const QString& libraryPath);

	protected:
		virtual void run() override;

	private:
		bool			getAndSaveAllFiles(const QHash<QString, MetaData>& pathMetadataMap);
		QStringList		getFilesRecursive(QDir base_dir);
		QStringList		filterValidFiles(const QDir& dir, const QStringList& filesInDir);
		void			storeMetadataBlock(const MetaDataList& v_md);
	};
}

#endif /* RELOADTHREAD_H_ */
