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

QString Website::priv_identifier() const
{
	return "website";
}

bool Website::can_fetch_cover_directly() const
{
	return false;
}

QStringList Website::parse_addresses(const QByteArray& website) const
{
	if(!Util::File::is_www(m->website))
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
		if(!image.contains("://")){
			image.prepend(m->website + "/");
		}

		images << image;
		index = regex.indexIn(website_str, index+5);
	}

	return images;
}

int Website::estimated_size() const
{
	return 1;
}

QString Website::search_address(const QString& address) const
{
	Q_UNUSED(address)
	return m->website;
}

void Website::set_website(const QString& website)
{
	m->website = website;

	if(!m->website.startsWith("http")) {
		m->website.prepend("http://");
	}
}
