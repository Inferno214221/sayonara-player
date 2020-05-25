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

	const QString website_str = QString::fromLocal8Bit(website);

	QRegExp regex("[\"'](\\S+\\.(jpg|png|gif|tiff|svg))[\"']");
	regex.setMinimal(true);

	QStringList images;
	int index = regex.indexIn(website_str);
	while(index > 0)
	{
		QString image = regex.cap(1);
		if(!image.contains("://"))
		{
			image.prepend(m->website + "/");
		}

		images << image;
		index = regex.indexIn(website_str, index + 5);
	}

	return images;
}

int Website::estimatedSize() const
{
	return 1;
}

QString Website::fulltextSearchAddress(const QString& address) const
{
	Q_UNUSED(address)
	return m->website;
}

void Website::setWebsite(const QString& website)
{
	m->website = website;

	if(!m->website.startsWith("http"))
	{
		m->website.prepend("http://");
	}
}
