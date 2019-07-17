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

#include <QStringList>

#include <assert.h>

struct MetaDataList::Private
{
	int current_track;
	Private() :
		current_track(-1)
	{}

	Private(const Private& other) :
		CASSIGN(current_track)
	{}

	Private(Private&& other) :
		CMOVE(current_track)
	{}

	Private& operator=(const Private& other)
	{
		ASSIGN(current_track);
		return (*this);
	}

	Private& operator=(Private&& other)
	{
		MOVE(current_track);
		return (*this);
	}
};

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
{
	m = Pimpl::make<Private>();
	assert(m != nullptr);
}

MetaDataList::MetaDataList(const MetaData& md) :
	MetaDataList::Parent()
{
	m = Pimpl::make<Private>();
	assert(m != nullptr);

	append(md);
}

MetaDataList::MetaDataList(const MetaDataList& other) :
	MetaDataList::Parent()
{
	m = Pimpl::make<Private>(*(other.m));
	assert(m != nullptr);

	this->resize(other.size());
	std::copy(other.begin(), other.end(), this->begin());
}

MetaDataList::MetaDataList(MetaDataList&& other) :
	MetaDataList::Parent()
{
	m = Pimpl::make<Private>(std::move(*(other.m)));
	assert(m != nullptr);

	this->resize(other.size());
	std::move(other.begin(), other.end(), this->begin());
}

MetaDataList::~MetaDataList() {}

MetaDataList& MetaDataList::operator=(const MetaDataList& other)
{
	(*m) = *(other.m);

	this->resize(other.size());
	std::copy(other.begin(), other.end(), this->begin());

	return (*this);
}

MetaDataList& MetaDataList::operator=(MetaDataList&& other)
{
	(*m) = std::move(*(other.m));

	this->resize(other.size());
	std::move(other.begin(), other.end(), this->begin());

	return (*this);
}

int MetaDataList::current_track() const
{
	return m->current_track;
}

void MetaDataList::set_current_track(int idx)
{
	m->current_track = -1;

	if( !Util::between(idx, this)) {
		return;
	}

	int tmp_idx=0;
	for(auto it=this->begin(); it != this->end(); it++ ){
		it->pl_playing = (idx == tmp_idx);
		tmp_idx++;
	}

	m->current_track = idx;
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

	if(current_track() >= tgt_idx){
		set_current_track(current_track() + v_md.count());
	}

	return *this;
}

MetaDataList& MetaDataList::copy_tracks(const IndexSet& indexes, int tgt_idx)
{
	MetaDataList v_md; v_md.reserve(indexes.size());

	for(int idx : indexes){
		v_md << this->at(idx);
	}

	return insert_tracks(v_md, tgt_idx);
}

#include "Utils/Logger/Logger.h"
MetaDataList& MetaDataList::move_tracks(const IndexSet& indexes, int tgt_idx)
{
	sp_log(Log::Develop, this) << "Move " << indexes << " to " << tgt_idx;

	MetaDataList v_md_to_move; 		v_md_to_move.reserve(indexes.size());
	MetaDataList v_md_before_tgt; 	v_md_before_tgt.reserve(count());
	MetaDataList v_md_after_tgt; 	v_md_after_tgt.reserve(count());

	int i=0;
	int n_tracks_after_cur_idx = 0;
	int n_tracks_before_cur_idx = 0;

	bool contains_cur_track = false;

	for(auto it=this->begin(); it!=this->end(); it++, i++)
	{
		const MetaData& md = *it;

		it->pl_playing = (i == m->current_track);

		if(indexes.contains(i))
		{
			contains_cur_track |= (i == m->current_track);

			if(i < m->current_track){
				n_tracks_before_cur_idx++;
			}

			else if(i > m->current_track){
				n_tracks_after_cur_idx++;
			}

			v_md_to_move << std::move( md );
			sp_log(Log::Crazy, this) << "Track to move: " << md.title();
		}

		else if(i<tgt_idx)
		{
			v_md_before_tgt << std::move( md );
			sp_log(Log::Crazy, this) << "Track before: " << md.title();
		}

		else
		{
			v_md_after_tgt << std::move( md );
			sp_log(Log::Crazy, this) << "Track after: " << md.title();
		}
	}

	auto it = this->begin();
	std::move(v_md_before_tgt.begin(), v_md_before_tgt.end(), it);
	it += v_md_before_tgt.count();

	std::move(v_md_to_move.begin(), v_md_to_move.end(), it);
	it += v_md_to_move.count();

	std::move(v_md_after_tgt.begin(), v_md_after_tgt.end(), it);

	if(contains_cur_track) {
		m->current_track = (n_tracks_before_cur_idx) + v_md_before_tgt.count();
	}

	if(!contains_cur_track)
	{
		if(tgt_idx <= m->current_track) {
			m->current_track += n_tracks_after_cur_idx;
		}

		else {
			m->current_track -= n_tracks_before_cur_idx;
		}
	}

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

	int n_elems = last - first + 1;
	this->erase(this->begin() + first, this->begin() + last + 1);

	if(m->current_track >= first && m->current_track <= last){
		set_current_track(-1);
	}

	if(m->current_track > last){
		set_current_track( m->current_track - n_elems );
	}

	return *this;
}

MetaDataList& MetaDataList::remove_tracks(std::function<bool (const MetaData&)> attr)
{
	this->erase( std::remove_if(this->begin(), this->end(), attr), this->end() );
	return *this;
}

MetaDataList& MetaDataList::remove_tracks(const IndexSet& indexes)
{
	int deleted_elements = 0;
	for(int i : indexes)
	{
		std::move(
				this->begin() + (i - deleted_elements + 1),
				this->end(),
				this->begin() + (i - deleted_elements)
		);

		deleted_elements++;
	}

	this->resize(this->count() - deleted_elements);

	if(indexes.contains(m->current_track))
	{
		m->current_track = -1;
	}

	else
	{
		int n_tracks_before_cur_track = Util::Algorithm::count_if(indexes, [=](int idx){
			return (idx < m->current_track);
		});

		m->current_track -= n_tracks_before_cur_track;
	}

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
		if(it->id == id){
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
		if(md.id >= 0){
			lst << QString::number(md.id);
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
	int old_size = this->count();
	resize(old_size + v_md.count());

	std::copy(v_md.begin(),
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
		return (id == md.id);
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

				this->resize(this->count() - 1);

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
	long long diff_cap = other.size() - (this->capacity() - this->size());
	if(diff_cap > 0)
	{
		this->reserve(this->capacity() + diff_cap);
	}

	for(auto it = other.begin(); it != other.end(); it++)
	{
		if(!this->contains(it->id)){
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
	return at(this->count() - 1);
}

int MetaDataList::count() const
{
	return (int) MetaDataList::Parent::size();
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
