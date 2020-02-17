/* MetaDataList.cpp */

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

#include "MetaDataList.h"
#include "MetaDataSorting.h"

#include "Utils/Algorithm.h"
#include "Utils/Set.h"
#include "Utils/globals.h"
#include "Utils/Logger/Logger.h"

#include <QStringList>

#include <assert.h>

const MetaData& MetaDataList::operator[](int i) const
{
	return *(this->cbegin() + i);
}

MetaData& MetaDataList::operator[](int i)
{
	return *(this->begin() + i);
}

MetaDataList::MetaDataList() :
	MetaDataList::Parent()
{}

MetaDataList::MetaDataList(const MetaData& md) :
	MetaDataList::Parent()
{
	append(md);
}

MetaDataList::MetaDataList(const MetaDataList& other) :
	MetaDataList::Parent()
{
	this->resize(other.size());
	std::copy(other.begin(), other.end(), this->begin());
}

MetaDataList::MetaDataList(MetaDataList&& other) noexcept :
	MetaDataList::Parent()
{
	this->resize(other.size());
	std::move(other.begin(), other.end(), this->begin());
}

MetaDataList::~MetaDataList() = default;

MetaDataList& MetaDataList::operator=(const MetaDataList& other)
{
	this->resize(other.size());
	std::copy(other.begin(), other.end(), this->begin());

	return (*this);
}

MetaDataList& MetaDataList::operator=(MetaDataList&& other) noexcept
{
	this->resize(other.size());
	std::move(other.begin(), other.end(), this->begin());

	return (*this);
}


MetaDataList& MetaDataList::insertTrack(const MetaData& md, int tgt_idx)
{
	if(tgt_idx >= this->count())
	{
		return this->append(md);
	}

	tgt_idx = std::max(tgt_idx, 0);

	MetaDataList v_md{md};
	return insertTracks(v_md, tgt_idx);
}

MetaDataList& MetaDataList::insertTracks(const MetaDataList& v_md, int tgt_idx)
{
	if(tgt_idx >= this->count())
	{
		return this->append(v_md);
	}

	tgt_idx = std::max(tgt_idx, 0);

	std::copy(v_md.begin(), v_md.end(), std::inserter(*this, this->begin() + tgt_idx));

	return *this;
}

MetaDataList& MetaDataList::copyTracks(const IndexSet& indexes, int tgt_idx)
{
	tgt_idx = std::max<int>(0, std::min<int>(tgt_idx, this->count() - 1));

	MetaDataList tracks; tracks.reserve(indexes.size());

	for(int idx : indexes)
	{
		tracks << this->at( size_t(idx) );
	}

	return insertTracks(tracks, tgt_idx);
}


MetaDataList& MetaDataList::moveTracks(const IndexSet& indexes, int tgt_idx) noexcept
{
	tgt_idx = std::max<int>(0, std::min<int>(tgt_idx, this->count()));

	MetaDataList tracksToMove; 		tracksToMove.reserve(indexes.size());
	MetaDataList tracksBeforeTgt; 	tracksBeforeTgt.reserve(size());
	MetaDataList tracksAfterTgt; 	tracksAfterTgt.reserve(size());

	int i=0;

	for(auto it=this->begin(); it!=this->end(); it++, i++)
	{
		MetaData& md = *it;

		if(indexes.contains(i))
		{
			tracksToMove << std::move( md );
		}

		else if(i < tgt_idx)
		{
			tracksBeforeTgt << std::move( md );
		}

		else
		{
			tracksAfterTgt << std::move( md );
		}
	}

	auto it = this->begin();
	std::move(tracksBeforeTgt.begin(), tracksBeforeTgt.end(), it);
	it += tracksBeforeTgt.count();

	std::move(tracksToMove.begin(), tracksToMove.end(), it);
	it += tracksToMove.count();

	std::move(tracksAfterTgt.begin(), tracksAfterTgt.end(), it);

	return *this;
}

MetaDataList& MetaDataList::removeTrack(int idx)
{
	return removeTracks(idx, idx);
}

MetaDataList& MetaDataList::removeTracks(int first, int last)
{
	if( !Util::between(first, this) ||
		!Util::between(last, this))
	{
		return *this;
	}

	this->erase(this->begin() + first, this->begin() + last + 1);

	return *this;
}

MetaDataList& MetaDataList::removeTracks(std::function<bool (const MetaData&)> attr)
{
	this->erase( std::remove_if(this->begin(), this->end(), attr), this->end() );
	return *this;
}

MetaDataList& MetaDataList::removeTracks(const IndexSet& indexes)
{
	for(auto it=indexes.rbegin(); it != indexes.rend(); it++)
	{
		if(Util::between(*it, *this))
		{
			this->erase(this->begin() + *it);
		}
	}

	return *this;
}


bool MetaDataList::contains(const MetaData& md) const
{
	auto it = std::find_if(this->begin(), this->end(), [&md](const MetaData& md_tmp){
		return md.isEqual(md_tmp);
	});

	return (it != this->end());
}

QList<int> MetaDataList::findTracks(Id id) const
{
	IdxList ret;

	if(id == -1) {
		return ret;
	}

	int idx=0;
	for(auto it=this->begin(); it != this->end(); it++, idx++)
	{
		if(it->id() == id){
			ret << idx;
		}
	}

	return ret;
}

QList<int> MetaDataList::findTracks(const QString& path) const
{
	QList<int> ret;

	if(path.isEmpty()) {
		return ret;
	}

#ifdef Q_OS_UNIX
	Qt::CaseSensitivity sensitivity = Qt::CaseSensitive;
#else
	Qt::CaseSensitivity sensitivity = Qt::CaseInsensitive;
#endif

	int idx=0;
	for(auto it=this->begin(); it != this->end(); it++, idx++)
	{
		if(it->filepath().compare(path, sensitivity) == 0){
			ret << idx;
		}
	}

	return ret;
}


QStringList MetaDataList::toStringList() const
{
	QStringList lst;

	auto lambda = [&lst](const MetaData& md)
	{
		if(md.id() >= 0){
			lst << QString::number(md.id());
		}
		else{
			lst << md.filepath();
		}
	};

	std::for_each(this->begin(), this->end(), lambda);

	return lst;
}


MetaDataList& MetaDataList::operator <<(const MetaDataList& v_md)
{
	return append(v_md);
}

MetaDataList& MetaDataList::operator <<(const MetaData& md)
{
	return append(md);
}

MetaDataList& MetaDataList::operator <<(MetaDataList&& v_md) noexcept
{
	return append(std::move(v_md));
}

MetaDataList& MetaDataList::operator <<(MetaData&& md) noexcept
{
	return append(std::move(md));
}

MetaDataList& MetaDataList::append(const MetaDataList& v_md)
{
	auto old_size = this->size();
	resize(old_size + v_md.size());

	std::copy
	(
		v_md.begin(),
		v_md.end(),
		this->begin() + old_size
	);

	return *this;
}

MetaDataList& MetaDataList::append(MetaDataList&& v_md) noexcept
{
	auto old_size = this->size();
	resize(old_size + v_md.size());

	std::move(v_md.begin(), v_md.end(), this->begin() + old_size);
	return *this;
}

MetaDataList& MetaDataList::append(const MetaData& md)
{
	push_back(md);
	return *this;
}

MetaDataList& MetaDataList::append(MetaData&& md) noexcept
{
	resize(this->size() + 1);
	auto it=this->begin() + this->size() - 1;
	*it = std::move(md);

	return *this;
}

QList<UniqueId> MetaDataList::unique_ids() const
{
	QList<UniqueId> ret;
	for(auto it=this->begin(); it != this->end(); it++)
	{
		ret << it->uniqueId();
	}

	return ret;
}

bool MetaDataList::contains(TrackID id) const
{
	auto it = std::find_if(this->begin(), this->end(), [&id](const MetaData& md){
		return (id == md.id());
	});

	return (it != this->end());
}

void MetaDataList::removeDuplicates()
{
	for(auto it=this->begin(); it != this->end(); it++)
	{
		auto rit = std::remove_if(std::next(it), this->end(), [it](const MetaData& md)
		{
			return (it->filepath().compare(md.filepath()) == 0);
		});

		this->erase(rit, this->end());
	}
}

bool MetaDataList::isEmpty() const
{
	return this->empty();
}

MetaDataList& MetaDataList::appendUnique(const MetaDataList& other)
{
	std::copy_if(other.begin(), other.end(), std::back_inserter(*this), [=](const MetaData& md)
	{
		return std::none_of(this->begin(), this->end(), [&md](const MetaData& md2)
		{
			return (md.filepath().compare(md2.filepath()) == 0);
		});
	});

	return *this;
}

const MetaData& MetaDataList::first() const
{
	return *(this->begin());
}

const MetaData& MetaDataList::last() const
{
	return *(std::prev(this->end()));
}

int MetaDataList::count() const
{
	return int(MetaDataList::Parent::size());
}


MetaData MetaDataList::takeAt(int idx)
{
	MetaData md(std::move(this->at( size_t(idx) )));
	this->erase(this->begin() + idx);

	return md;
}


void MetaDataList::sort(Library::SortOrder so)
{
	if(so == Library::SortOrder::NoSorting){
		return;
	}

	MetaDataSorting::sortMetadata(*this, so);
}

void MetaDataList::reserve(size_t items)
{
	Q_UNUSED(items)
}

size_t MetaDataList::capacity() const
{
	return 0;
}
