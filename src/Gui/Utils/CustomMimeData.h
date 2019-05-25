/* CustomMimeData.h */

/* Copyright 2011-2019  Lucio Carreras
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

#ifndef _CUSTOMMIMEDATA_H
#define _CUSTOMMIMEDATA_H

#include "Utils/Pimpl.h"

#include <QMimeData>

/**
 * @brief Mimedata class for drag and dropping metadata
 * @ingroup GUIHelper
 */
class CustomMimeData : public QMimeData
{

private:
	PIMPL(CustomMimeData)

	CustomMimeData(const void* p);
	const void* ptr() const;

public:
	/**
	 * @brief Constructor
	 */

	template<typename T>
	CustomMimeData(const T* class_instance) :
		CustomMimeData(static_cast<const void*>(class_instance))
	{}

	virtual ~CustomMimeData();

	/**
	 * @brief Set metadata you want to drag and drop
	 * @param v_md metadata that should be sent
	 */
	void set_metadata(const MetaDataList& v_md);

	/**
	 * @brief get metadata from drag and drop
	 * @param v_md reference to metadata
	 * @return size of metadata
	 */
	const MetaDataList& metadata() const;

	/**
	 * @brief check, if the custom mimedata has metadata
	 * @return true if yes, false else
	 */
	bool has_metadata() const;

	void set_playlist_source_index(int playlist_idx);
	int playlist_source_index() const;

	QString cover_url() const;
	void set_cover_url(const QString& url);

	template<typename T>
	bool has_source(const T* class_instance) const
	{
		const void* void_ptr = ptr();
		const T* p = static_cast<const T*>(void_ptr);
		if(!p){
			return false;
		}

		return (p == class_instance);
	}
};

#endif
