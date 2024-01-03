/* CopyFolderThread.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#include "CopyProcessor.h"
#include "Components/Library/Importer/ImportCache.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/FileSystem.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Logger/Logger.h"

#include <atomic>

namespace
{
	constexpr const auto* ClassName = "Library::CopyProcessor";

	bool copyFile(const QString& filename, const QString& targetFilename, const Util::FileSystemPtr& fileSystem)
	{
		spLog(Log::Debug, ClassName) << "copy " << filename << " to \n\t" << targetFilename;

		if(fileSystem->exists(targetFilename))
		{
			spLog(Log::Info, ClassName) << "Overwrite " << targetFilename;
			fileSystem->deleteFiles({targetFilename});
		}

		const auto success = fileSystem->copyFile(filename, targetFilename);
		if(!success)
		{
			spLog(Log::Warning, ClassName) << "Copy error";
		}

		return success;
	}

	MetaDataList appendCacheTrack(MetaData cacheTrack, const QString& targetFilename, MetaDataList copiedTracks)
	{
		if(cacheTrack.isValid())
		{
			spLog(Log::Debug, ClassName) << "Set new filename: " << targetFilename;
			cacheTrack.setFilepath(targetFilename);
			copiedTracks << std::move(cacheTrack);
		}

		return copiedTracks;
	}

	QStringList extractFilepaths(const MetaDataList& tracks)
	{
		QStringList result;
		Util::Algorithm::transform(tracks, result, [](const auto& track) {
			return track.filepath();
		});

		return result;
	}

	class CopyProcessorImpl :
		public Library::CopyProcessor
	{
		public:
			CopyProcessorImpl(QString targetDirectory,
			                  std::shared_ptr<Library::ImportCache> importCache,
			                  std::shared_ptr<Util::FileSystem> fileSystem) :
				m_targetDir {std::move(targetDirectory)},
				m_importCache {std::move(importCache)},
				m_fileSystem {std::move(fileSystem)} {}

			~CopyProcessorImpl() override = default;

			void copy() override
			{
				m_cancelled = false;
				m_tracks.clear();

				const auto files = m_importCache->files();
				spLog(Log::Debug, this) << "Start copying " << files;
				for(const auto& filename: files)
				{
					if(wasCancelled())
					{
						return;
					}

					const auto targetFilename = m_importCache->targetFilename(filename, m_targetDir);
					if(targetFilename.isEmpty())
					{
						continue;
					}

					if(copyFile(filename, targetFilename, m_fileSystem))
					{
						m_tracks = appendCacheTrack(m_importCache->metadata(filename),
						                            targetFilename,
						                            std::move(m_tracks));
						emitPercent();
					}
				}

				emit sigFinished();
			}

			void rollback() override
			{
				auto copiedPaths = extractFilepaths(m_tracks);
				while(copiedPaths.count() > 0)
				{
					const auto filename = copiedPaths.takeLast();
					m_fileSystem->deleteFiles({filename});

					emitPercent();
				}
			}

			void cancel() override { m_cancelled = true; }

			[[nodiscard]] MetaDataList copiedMetadata() const override { return m_tracks; }

			[[nodiscard]] bool wasCancelled() const override { return m_cancelled; }

			[[nodiscard]] int copiedFileCount() const override { return m_tracks.count(); }

		private:
			void emitPercent()
			{
				const auto percent = (m_tracks.count() * 100) / m_importCache->count();
				emitProgress(percent);
			}

			QString m_targetDir;
			Library::ImportCachePtr m_importCache;
			Util::FileSystemPtr m_fileSystem;
			MetaDataList m_tracks;
			std::atomic<bool> m_cancelled {false};
	};
}

namespace Library
{
	void CopyProcessor::emitProgress(int progress)
	{
		emit sigProgress(progress);
	}

	CopyProcessor* CopyProcessor::create(const QString& targetDirectory,
	                                     const std::shared_ptr<ImportCache>& cache,
	                                     const std::shared_ptr<Util::FileSystem>& fileSystem)
	{
		return new CopyProcessorImpl(targetDirectory, cache, fileSystem);
	}
}