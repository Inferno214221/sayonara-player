/* ChangeablePipeline.h */

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

#ifndef CHANGEABLEPIPELINE_H
#define CHANGEABLEPIPELINE_H

#include "gst/gst.h"

namespace PipelineExtensions
{
	/**
	 * @brief The ChangeablePipeline class
	 * @ingroup EngineInterfaces
	 */
	class Changeable
	{
		public:
			Changeable();
			virtual ~Changeable();

			/**
			 * @brief Add an element between two elements
			 * @param element element to add
			 * @param firstElement element, after which new element is inserted
			 * @param secondElement element, before which new element is inserted (may be null)
			 */
			bool addElement(GstElement* element, GstElement* firstElement, GstElement* secondElement);

			/**
			 * @brief remove an element between two elements
			 * @param element element to remove
			 * @param firstElement element, after which new element is removed
			 * @param secondElement element, before which new element is removed (may be null)
			 */
			bool removeElement(GstElement* element, GstElement* firstElement, GstElement* secondElement);


			bool replaceSink(GstElement* oldSink, GstElement* newSink, GstElement* element_before, GstElement* pipeline, GstElement* bin);
	};
}

#endif // CHANGEABLEPLAYLIST_H
