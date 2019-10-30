/* MetaDataList.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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


MetaDataList& MetaDataList::insert_track(const MetaData& md, int tgt_idx)
{
	MetaDataList v_md{md};
	return insert_tracks(v_md, tgt_idx);
}

MetaDataList& MetaDataList::insert_tracks(const MetaDataList& v_md, int tgt_idx)
{
	if(v_md.isEmpty()) {
		return *this;
	}

	auto it = this->begin() + tgt_idx;
	for(const MetaData& md : v_md)
	{
		it = this->insert(it, md);
		it++;
	}

	return *this;
}

MetaDataList& MetaDataList::copy_tracks(const IndexSet& indexes, int tgt_idx)
{
	MetaDataList v_md; v_md.reserve(indexes.size());

	for(int idx : indexes)
	{
		v_md << this->at( size_t(idx) );
	}

	return insert_tracks(v_md, tgt_idx);
}


MetaDataList& MetaDataList::move_tracks(const IndexSet& indexes, int tgt_idx)
{
	MetaDataList v_md_to_move; 		v_md_to_move.reserve(indexes.size());
	MetaDataList v_md_before_tgt; 	v_md_before_tgt.reserve(size());
	MetaDataList v_md_after_tgt; 	v_md_after_tgt.reserve(size());

	int i=0;

	for(auto it=this->begin(); it!=this->end(); it++, i++)
	{
		const MetaData& md = *it;

		if(indexes.contains(i))
		{
			v_md_to_move << std::move( md );
		}

		else if(i<tgt_idx)
		{
			v_md_before_tgt << std::move( md );
		}

		else
		{
			v_md_after_tgt << std::move( md );
		}
	}

	auto it = this->begin();
	std::move(v_md_before_tgt.begin(), v_md_before_tgt.end(), it);
	it += v_md_before_tgt.count();

	std::move(v_md_to_move.begin(), v_md_to_move.end(), it);
	it += v_md_to_move.count();

	std::move(v_md_after_tgt.begin(), v_md_after_tgt.end(), it);

	return *this;
}

MetaDataList& MetaDataList::remove_track(int idx)
{
	return remove_tracks(idx, idx);
}

MetaDataList& MetaDataList::remove_tracks(int first, int last)
{
	if( !Util::between(first, this) ||
		!Util::between(last, this))
	{
		return *this;
	}

	this->erase(this->begin() + first, this->begin() + last + 1);

	return *this;
}

MetaDataList& MetaDataList::remove_tracks(std::function<bool (const MetaData&)> attr)
{
	this->erase( std::remove_if(this->begin(), this->end(), attr), this->end() );
	return *this;
}

MetaDataList& MetaDataList::remove_tracks(const IndexSet& indexes)
{
	size_t deleted_elements = 0;
	for(int i : indexes)
	{
		std::move
		(
				this->begin() + (i - deleted_elements + 1),
				this->end(),
				this->begin() + (i - deleted_elements)
		);

		deleted_elements++;
	}

	this->resize(this->size() - deleted_elements);

	return *this;
}


bool MetaDataList::contains(const MetaData& md) const
{
	auto it = std::find_if(this->begin(), this->end(), [&md](const MetaData& md_tmp){
		return md.is_equal(md_tmp);
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
	for(auto it=this->begin(); it != this->end(); it++)
	{
		if(it->id() == id){
			ret << idx;
		}

		idx++;
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
	for(auto it=this->begin(); it != this->end(); it++)
	{
		if(it->filepath().compare(path, sensitivity) == 0){
			ret << idx;
		}

		idx++;
	}

	return ret;
}


QStringList MetaDataList::toStringList() const
{
	QStringList lst;

	auto lambda = [&lst](const MetaData& md){
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

MetaDataList& MetaDataList::append(const MetaDataList& v_md)
{
	size_t old_size = this->size();
	resize(old_size + v_md.size());

	std::copy
	(
		v_md.begin(),
		v_md.end(),
		this->begin() + old_size
	);

	return *this;
}

MetaDataList& MetaDataList::append(const MetaData& md)
{
	push_back(md);
	return *this;
}

bool MetaDataList::contains(TrackID id) const
{
	auto it = std::find_if(this->begin(), this->end(), [&id](const MetaData& md){
		return (id == md.id());
	});

	return (it != this->end());
}

void MetaDataList::remove_duplicates()
{
	for(auto it=this->begin(); it!=this->end(); it++)
	{
		auto it_next = it + 1;

		if(it_next == this->end()){
			break;
		}

		auto last_it2 = it;
		for(auto it2=it_next; it2 != this->end(); it2++) {

			if(it->filepath().compare(it2->filepath()) == 0){
				auto it2_next = it2 + 1;
				if(it2_next != this->end()){
					std::move(it2_next, this->end(), it2);
				}

				this->resize(this->size() - 1);

				it2 = last_it2 + 1;
				if(it2 == this->end()){
					break;
				}
			}

			else {
				last_it2 = it2;
			}
		}
	}
}

bool MetaDataList::isEmpty() const
{
	return this->empty();
}

MetaDataList& MetaDataList::append_unique(const MetaDataList& other)
{
	auto diff_cap = other.size() - (this->capacity() - this->size());
	if(diff_cap > 0)
	{
		this->reserve(this->capacity() + diff_cap);
	}

	for(auto it = other.begin(); it != other.end(); it++)
	{
		if(!this->contains(it->id())){
			this->push_back(*it);
		}
	}

	return *this;
}

const MetaData& MetaDataList::first() const
{
	return at(0);
}

const MetaData& MetaDataList::last() const
{
	return at(this->size() - 1);
}

int MetaDataList::count() const
{
	return int(MetaDataList::Parent::size());
}


MetaData MetaDataList::take_at(int idx)
{
	MetaData md = this->at(idx);
	this->remove_track(idx);
	return md;
}


void MetaDataList::sort(Library::SortOrder so)
{
	if(so == Library::SortOrder::NoSorting){
		return;
	}

	MetaDataSorting::sort_metadata(*this, so);
}

void MetaDataList::reserve(size_t items)
{
	Q_UNUSED(items)
}

size_t MetaDataList::capacity() const
{
	return 0;
}
