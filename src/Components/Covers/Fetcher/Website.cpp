/* Website.cpp
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

Cover::Fetcher::Website::Website(const QString& website) :
	Base()
{
	m = Pimpl::make<Private>();
	setWebsite(website);
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
	m->website = (website.startsWith("http") || website.isEmpty())
	              ? website
	              : QString("https://%1").arg(website);
}
