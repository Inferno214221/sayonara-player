/* CustomPlaylistSkeleton.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#include "CustomPlaylistSkeleton.h"
#include <QString>

struct CustomPlaylistSkeleton::Private
{
    QString name;
    int     id;
    int     num_tracks;
    bool    is_temporary;

    Private() :
        id(-1),
        num_tracks(0),
        is_temporary(false)
    {}

    Private(const Private& other) :
        CASSIGN(name),
        CASSIGN(id),
        CASSIGN(num_tracks),
        CASSIGN(is_temporary)
    {}

    Private& operator=(const Private& other)
    {
        ASSIGN(name);
        ASSIGN(id);
        ASSIGN(num_tracks);
        ASSIGN(is_temporary);

        return *this;
    }
};


CustomPlaylistSkeleton::CustomPlaylistSkeleton()
{
    m = Pimpl::make<Private>();
}

CustomPlaylistSkeleton::CustomPlaylistSkeleton(const CustomPlaylistSkeleton& other)
{
    m = Pimpl::make<Private>(*(other.m));
}

CustomPlaylistSkeleton& CustomPlaylistSkeleton::operator=(const CustomPlaylistSkeleton &other)
{
    *m = *(other.m);

    return *this;
}

CustomPlaylistSkeleton::~CustomPlaylistSkeleton() {}

int CustomPlaylistSkeleton::id() const
{ 
    return m->id;
}
 
void CustomPlaylistSkeleton::set_id(int id)
{
    m->id = id;
}

QString CustomPlaylistSkeleton::name() const
{
    return m->name;
}

void CustomPlaylistSkeleton::set_name(const QString& name)
{
    m->name = name;
}

bool CustomPlaylistSkeleton::temporary() const
{
    return m->is_temporary;
}


void CustomPlaylistSkeleton::set_temporary(bool temporary)
{
    m->is_temporary = temporary;
}

int CustomPlaylistSkeleton::num_tracks() const
{
    return m->num_tracks;
}

void CustomPlaylistSkeleton::set_num_tracks(int num_tracks)
{
    m->num_tracks = num_tracks;
}


