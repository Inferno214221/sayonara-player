#include "ChangeInformation.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Logger/Logger.h"
#include <QPixmap>

using Tagging::ChangeInformation;

struct ChangeInformation::Private
{
	MetaData originalMetadata;
	MetaData changedMetadata;

	QPixmap newCover;

	bool hasChanges;
	bool hasNewCover;

	Private(const MetaData& track) :
		originalMetadata(track),
		changedMetadata(track),
		hasChanges(false),
		hasNewCover(false) {}
};

ChangeInformation::ChangeInformation(const MetaData& track)
{
	m = Pimpl::make<Private>(track);
}

ChangeInformation::~ChangeInformation() = default;

Tagging::ChangeInformation::ChangeInformation(const Tagging::ChangeInformation& other)
{
	m = Pimpl::make<Private>(*other.m);
}

Tagging::ChangeInformation& ChangeInformation::operator=(const Tagging::ChangeInformation& other)
{
	*m = *(other.m);
	return *this;
}

void ChangeInformation::update(const MetaData& track)
{
	const auto isEqual = track.isEqualDeep(m->originalMetadata);
	if(!isEqual)
	{
		m->changedMetadata = track;
		m->hasChanges = true;
	}
}

void ChangeInformation::updateCover(const QPixmap& pm)
{
	if(pm.isNull())
	{
		spLog(Log::Warning, this) << "Bad cover: Will not update";
		return;
	}

	m->newCover = pm;
	m->hasNewCover = true;
}

void ChangeInformation::apply()
{
	m->originalMetadata = m->changedMetadata;
	m->hasChanges = false;
	m->hasNewCover = false;
}

void ChangeInformation::undo()
{
	m->hasChanges = false;
	m->changedMetadata = m->originalMetadata;
	m->newCover = QPixmap();
	m->hasNewCover = false;
}

bool ChangeInformation::hasChanges() const
{
	return m->hasChanges;
}

void ChangeInformation::setChanged(bool b)
{
	m->hasChanges = b;
}

bool ChangeInformation::hasNewCover() const
{
	return m->hasNewCover;
}

QPixmap ChangeInformation::cover() const
{
	return m->newCover;
}

MetaData& ChangeInformation::currentMetadata()
{
	return m->changedMetadata;
}

MetaData& ChangeInformation::originalMetadata()
{
	return m->originalMetadata;
}

const MetaData& ChangeInformation::currentMetadata() const
{
	return m->changedMetadata;
}

const MetaData& ChangeInformation::originalMetadata() const
{
	return m->originalMetadata;
}
