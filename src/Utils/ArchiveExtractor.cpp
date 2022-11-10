/* ArchiveExtractor.cpp */
/*
 * Copyright (C) 2011-2022 Michael Lugmair
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

#include "ArchiveExtractor.h"

#include "Utils/Logger/Logger.h"
#include "Utils/FileUtils.h"

#include <QStringList>
#include <QProcess>
#include <QDir>

namespace
{
	constexpr const auto* RarExtension = "rar";
	constexpr const auto* ZipExtension = "zip";
	constexpr const auto* TarGzExtension = "tar.gz";
	constexpr const auto* TgzExtension = "tgz";

	struct ArchiveScanner
	{
		QString binary;
		QStringList arguments;
		QList<int> successReturnValues;
	};

	ArchiveScanner createArchiveScanner(const QString& filename, const QString& targetDir)
	{
		const auto extension = Util::File::getFileExtension(filename).toLower();
		if(extension == RarExtension)
		{
			return {"rar", {"x", filename, targetDir}, {0}};
		}
		if(extension == ZipExtension)
		{
			return {"unzip", {filename, "-d", targetDir}, {0, 1, 2}};
		}
		if((extension == TarGzExtension) || (extension == TgzExtension))
		{
			return {"tar", {"xzf", filename, "-C", targetDir}, {0}};
		}

		return {};
	}

	class ArchiveExtractorImpl :
		public Util::ArchiveExtractor
	{
		public:
			~ArchiveExtractorImpl() noexcept override = default;

			QStringList extractArchive(const QString& filename, const QString& targetDirectory) override
			{
				if(!isSupportedArchive(filename))
				{
					return {};
				}

				const auto archiveScanner = createArchiveScanner(filename, targetDirectory);
				const auto returnValue = QProcess::execute(archiveScanner.binary, archiveScanner.arguments);
				if(returnValue < 0)
				{
					spLog(Log::Error, this) << archiveScanner.binary << " not found or crashed";
					return {};
				}

				if(!archiveScanner.successReturnValues.contains(returnValue))
				{
					spLog(Log::Error, this) << archiveScanner.binary << " exited with error " << returnValue;
					return {};
				}

				return QDir(targetDirectory).entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
			}

			[[nodiscard]] QStringList supportedArchives() const override
			{
				return {RarExtension, ZipExtension, TgzExtension, TarGzExtension};
			}
	};
}

namespace Util
{
	ArchiveExtractor::ArchiveExtractor() = default;
	ArchiveExtractor::~ArchiveExtractor() noexcept = default;

	ArchiveExtractorPtr ArchiveExtractor::create()
	{
		return std::make_shared<ArchiveExtractorImpl>();
	}

	bool ArchiveExtractor::isSupportedArchive(const QString& filename) const
	{
		const auto extension = Util::File::getFileExtension(filename).toLower();
		return supportedArchives().contains(extension);
	}
}