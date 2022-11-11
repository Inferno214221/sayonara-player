/* ImportImportCacher.cpp */

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

#include "CachingThread.h"

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

	Library::ImportCacher::CacheResult
	initCacheResult(const QString& libraryPath, const Tagging::TagReaderPtr& tagReader)
	{
		return {
			std::make_shared<Library::ImportCache>(libraryPath, tagReader), QStringList {}
		};
	}

	class ImportCacherImpl :
		public Library::ImportCacher
	{
		public:
			ImportCacherImpl(QStringList sourceFiles,
			                 const QString& libraryPath,
			                 const Tagging::TagReaderPtr& tagReader,
			                 Util::ArchiveExtractorPtr archiveExtractor,
			                 Util::DirectoryReaderPtr directoryReader,
			                 Util::FileSystemPtr fileSystem,
			                 QObject* parent) :
				ImportCacher(parent),
				m_sourceFiles {std::move(sourceFiles)},
				m_cacheResult {initCacheResult(libraryPath, tagReader)},
				m_archiveExtractor {std::move(archiveExtractor)},
				m_directoryReader {std::move(directoryReader)},
				m_fileSystem(std::move(fileSystem)) {}

			~ImportCacherImpl() override = default;

		protected:
			[[nodiscard]] ImportCacher::CacheResult cacheResult() const override { return m_cacheResult; }

			void cacheFiles() override
			{
				m_cacheResult.cache->clear();
				spLog(Log::Develop, this) << "Read files";
				scanRootFiles();

				if(isCancelled())
				{
					m_cacheResult.cache->clear();
				}
			}

		private:
			void scanRootFiles()
			{
				for(const auto& filename: m_sourceFiles)
				{
					if(isCancelled())
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

				emit sigCachedFilesChanged(); // NOLINT(readability-misleading-indentation)
			}

			void addFile(const QString& file, const QString& relativeDir = QString())
			{
				m_cacheResult.cache->addFile(file, relativeDir);
				emit sigCachedFilesChanged();
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

			QStringList m_sourceFiles;
			Library::ImportCacher::CacheResult m_cacheResult;
			Util::ArchiveExtractorPtr m_archiveExtractor;
			Util::DirectoryReaderPtr m_directoryReader;
			Util::FileSystemPtr m_fileSystem;
	};
}

namespace Library
{
	struct ImportCacher::Private
	{
		bool cancelled {false};
	};

	ImportCacher::ImportCacher(QObject* parent) :
		QObject(parent),
		m {Pimpl::make<Private>()} {}

	ImportCacher::~ImportCacher() noexcept = default;

	ImportCacher* ImportCacher::create(const QStringList& fileList,
	                                   const QString& libraryPath,
	                                   const Tagging::TagReaderPtr& tagReader,
	                                   const Util::ArchiveExtractorPtr& archiveExtractor,
	                                   const Util::DirectoryReaderPtr& directoryReader,
	                                   const Util::FileSystemPtr& fileSystem,
	                                   QObject* parent)
	{
		return new ImportCacherImpl(fileList,
		                            libraryPath,
		                            tagReader,
		                            archiveExtractor,
		                            directoryReader,
		                            fileSystem,
		                            parent);
	}

	void ImportCacher::cancel() { m->cancelled = true; }

	bool ImportCacher::isCancelled() const { return m->cancelled; }
}