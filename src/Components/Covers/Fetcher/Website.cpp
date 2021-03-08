#include "Website.h"
#include "Utils/FileUtils.h"

#include <QRegExp>
#include <QStringList>

using Cover::Fetcher::Website;
using Cover::Fetcher::Base;

struct Website::Private
{
	QString website;
};

Cover::Fetcher::Website::Website() :
	Base()
{
	m = Pimpl::make<Private>();
}

Website::~Website() = default;

QString Website::privateIdentifier() const
{
	return "website";
}

bool Website::canFetchCoverDirectly() const
{
	return false;
}

QStringList Website::parseAddresses(const QByteArray& website) const
{
	if(!Util::File::isWWW(m->website))
	{
		return QStringList();
	}

	const auto websiteData = QString::fromLocal8Bit(website);

	auto regex = QRegExp("[\"'](\\S+\\.(jpg|png|gif|tiff|svg))[\"']");
	regex.setMinimal(true);

	QStringList images;
	auto index = regex.indexIn(websiteData);
	while(index > 0)
	{
		const auto caption = regex.cap(1);
		const auto imagePath = (caption.contains("://"))
			? caption
			: QString("%1/%2").arg(m->website).arg(caption);

		images << imagePath;
		index = regex.indexIn(websiteData, index + 5);
	}

	return images;
}

int Website::estimatedSize() const
{
	return 1;
}

bool Website::isWebserviceFetcher() const
{
	return false;
}

QString Website::fulltextSearchAddress([[maybe_unused]] const QString& address) const
{
	return m->website;
}

void Website::setWebsite(const QString& website)
{
	m->website = (website.startsWith("http"))
	              ? website
	              : QString("https://%1").arg(website);
}
