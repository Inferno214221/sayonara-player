#include "DirectFetcher.h"

#include "Utils/FileUtils.h"
#include "Utils/Utils.h"

#include <QStringList>

using Cover::Fetcher::DirectFetcher;

DirectFetcher::DirectFetcher() :
	Cover::Fetcher::Base() {}

DirectFetcher::~DirectFetcher() = default;

QString DirectFetcher::privateIdentifier() const
{
	return "direct";
}

bool DirectFetcher::canFetchCoverDirectly() const
{
	return true;
}

QStringList DirectFetcher::parseAddresses([[maybe_unused]] const QByteArray& website) const
{
	return QStringList();
}

QString DirectFetcher::artistAddress([[maybe_unused]] const QString& artist) const
{
	return QString();
}

QString DirectFetcher::albumAddress([[maybe_unused]] const QString& artist, [[maybe_unused]] const QString& album) const
{
	return QString();
}

QString DirectFetcher::fulltextSearchAddress(const QString& str) const
{
	return (Util::File::isWWW(str) && Util::File::isImageFile(str))
	       ? str
	       : QString();
}

int DirectFetcher::estimatedSize() const
{
	return 1;
}

bool Cover::Fetcher::DirectFetcher::isWebserviceFetcher() const
{
	return false;
}

