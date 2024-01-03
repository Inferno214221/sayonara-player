/* ImportImportCacher.cpp */

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

#include "CacheProcessor.h"

#include "Utils/Algorithm.h"
#include "Utils/ArchiveExtractor.h"
#include "Utils/DirectoryReader.h"
#include "Utils/FileSystem.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/StandardPaths.h"
#include "Utils/Tagging/TagReader.h"
#include "Utils/Utils.h"

#include <QDir>
#include <QThread>

namespace
{
	constexpr const auto* ClassName = "ImportCacher";

	QString createTempDirectory(const Util::FileSystemPtr& fileSystem)
	{
		constexpr const auto StringSize = 16;
		auto tempDirectory = QString("%1/%2")
			.arg(Util::tempPath("import"))
			.arg(Util::randomString(StringSize));

		const auto success = fileSystem->createDirectories(tempDirectory);
		if(!success)
		{
			spLog(Log::Warning, ClassName) << "Cannot create temp directory " << tempDirectory;
			return {};
		}

		return tempDirectory;
	}

	Library::CacheProcessor::CacheResult
	initCacheResult(const QString& libraryPath, const Tagging::TagReaderPtr& tagReader)
	{
		return {
			std::make_shared<Library::ImportCache>(libraryPath, tagReader), QStringList {}
		};
	}

	class CacheProcessorImpl :
		public Library::CacheProcessor
	{
		public:
			CacheProcessorImpl(QStringList sourceFiles,
			                   const QString& libraryPath,
			                   const Tagging::TagReaderPtr& tagReader,
			                   Util::ArchiveExtractorPtr archiveExtractor,
			                   Util::DirectoryReaderPtr directoryReader,
			                   Util::FileSystemPtr fileSystem) :
				m_sourceFiles {std::move(sourceFiles)},
				m_cacheResult {initCacheResult(libraryPath, tagReader)},
				m_archiveExtractor {std::move(archiveExtractor)},
				m_directoryReader {std::move(directoryReader)},
				m_fileSystem(std::move(fileSystem)) {}

			~CacheProcessorImpl() override = default;

		protected:
			[[nodiscard]] CacheProcessor::CacheResult cacheResult() const override { return m_cacheResult; }

			void cacheFiles() override
			{
				m_cancelled = false;
				m_cacheResult.cache->clear();
				spLog(Log::Develop, this) << "Read files";
				scanRootFiles();

				if(wasCancelled())
				{
					m_cacheResult.cache->clear();
				}

				spLog(Log::Debug, this) << "Caching finished: " << m_cacheResult.cache->count() << " files";
				emit sigFinished();
			}

		private:
			void scanRootFiles()
			{
				for(const auto& filename: m_sourceFiles)
				{
					if(wasCancelled())
					{
						return;
					}

					scanRootFile(filename);
				}
			}

			void scanRootFile(const QString& filename)
			{
				if(m_fileSystem->isDir(filename))
				{
					scanDirectory(filename);
				}

				else if(m_fileSystem->isFile(filename))
				{
					if(m_archiveExtractor->isSupportedArchive(filename))
					{
						cacheArchive(filename);
					}

					else
					{
						addFile(filename);
					}
				}

				emitCachedFilesChanged();
			}

			void addFile(const QString& file, const QString& relativeDir = QString())
			{
				m_cacheResult.cache->addFile(file, relativeDir);
				emitCachedFilesChanged();
			}

			void scanDirectory(const QString& dir)
			{
				const auto files = m_directoryReader->scanFilesRecursively(dir, QStringList {"*"});

				spLog(Log::Debug, this) << "Found " << files.size() << " files";

				auto upperDir = QDir(dir);
				{
					// Example:
					// dir = /dir/we/want/to/import
					// files:
					//	/dir/we/want/to/import/file1
					//	/dir/we/want/to/import/file2
					//	/dir/we/want/to/import/deeper/file1
					// -> cache:
					// we want the 'import' directory in the target
					// directory, too and not only its contents
					upperDir.cdUp();
				}

				for(const auto& file: files)
				{
					addFile(file, upperDir.absolutePath());
				}
			}

			void cacheArchive(const QString& archiveName)
			{
				const auto tempDirectory = createTempDirectory(m_fileSystem);
				m_cacheResult.temporaryFiles << tempDirectory;

				auto dir = QDir {tempDirectory};

				const auto extractedFiles = m_archiveExtractor->extractArchive(archiveName, tempDirectory);
				for(const auto& extractedFile: extractedFiles)
				{
					const auto filename = dir.absoluteFilePath(extractedFile);
					if(m_fileSystem->isDir(filename))
					{
						scanDirectory(filename);
					}

					else if(m_fileSystem->isFile(filename))
					{
						addFile(filename, tempDirectory);
					}
				}
			}

			void cancel() override { m_cancelled = true; }

			[[nodiscard]] bool wasCancelled() const override { return m_cancelled; }

			QStringList m_sourceFiles;
			Library::CacheProcessor::CacheResult m_cacheResult;
			Util::ArchiveExtractorPtr m_archiveExtractor;
			Util::DirectoryReaderPtr m_directoryReader;
			Util::FileSystemPtr m_fileSystem;
			std::atomic<bool> m_cancelled {false};
	};
}

namespace Library
{
	CacheProcessor::~CacheProcessor() noexcept = default;

	CacheProcessor* CacheProcessor::create(const QStringList& fileList,
	                                       const QString& libraryPath,
	                                       const Tagging::TagReaderPtr& tagReader,
	                                       const Util::ArchiveExtractorPtr& archiveExtractor,
	                                       const Util::DirectoryReaderPtr& directoryReader,
	                                       const Util::FileSystemPtr& fileSystem)
	{
		return new CacheProcessorImpl(fileList,
		                              libraryPath,
		                              tagReader,
		                              archiveExtractor,
		                              directoryReader,
		                              fileSystem);
	}

	void CacheProcessor::emitCachedFilesChanged() { emit sigCachedFilesChanged(); }
}