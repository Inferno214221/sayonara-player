#include "Station.h"
#include <QString>

Station::Station() {}
Station::Station(const Station&) : Station() {}

Station::~Station() = default;

Station& Station::station(const Station&)
{
	return *this;
}

struct Stream::Private
{
	QString name;
	QString url;

	Private(const QString& name, const QString& url) :
		name(name),
		url(url)
	{}
};

Stream::Stream() :
	Stream(QString(), QString())
{}

Stream::Stream(const QString& name, const QString& url) :
	Station()
{
	m = Pimpl::make<Private>(name, url);
}

Stream::Stream(const Stream& other) :
	Stream()
{
	*m = *(other.m);
}

Stream& Stream::operator=(const Stream& other)
{
	*m = *(other.m);
	return *this;
}

Stream::~Stream() = default;

QString Stream::name() const
{
	return m->name;
}

void Stream::setName(const QString& name)
{
	m->name = name;
}

QString Stream::url() const
{
	return m->url;
}

void Stream::setUrl(const QString& url)
{
	m->url = url;
}

struct Podcast::Private
{
	QString name;
	QString url;
	bool reversed;

	Private(const QString& name, const QString& url, bool reversed) :
		name(name),
		url(url),
		reversed(reversed)
	{}
};

Podcast::Podcast() :
	Podcast(QString(), QString(), false)
{}

Podcast::Podcast(const QString& name, const QString& url, bool reversed) :
	Station()
{
	m = Pimpl::make<Private>(name, url, reversed);
}

Podcast::Podcast(const Podcast& other) :
	Podcast()
{
	*m = *(other.m);
}

Podcast::~Podcast() = default;

QString Podcast::name() const
{
	return m->name;
}

void Podcast::setName(const QString& name)
{
	m->name = name;
}

QString Podcast::url() const
{
	return m->url;
}

void Podcast::setUrl(const QString& url)
{
	m->url = url;
}

bool Podcast::reversed() const
{
	return m->reversed;
}

void Podcast::setReversed(bool b)
{
	m->reversed = b;
}

Podcast& Podcast::operator=(const Podcast& other)
{
	*m = *(other.m);
	return *this;
}
