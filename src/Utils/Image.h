/* Image.h */

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

#ifndef UTIL_IMAGE_H
#define UTIL_IMAGE_H

class QPixmap;
class QImage;
class QSize;

namespace Util
{
	/**
	 * @brief The Image class
	 * @ingroup Helper
	 */
	class Image
	{
		private:
			struct Private;
			Private* m=nullptr;

		public:
			Image();
			Image(const QPixmap& pm);
			Image(const QPixmap& pm, const QSize& max_size);

			Image(const Image& other);
			~Image();

			Image& operator=(const Image& other);

			QPixmap pixmap() const;
	};
}



#endif // UTIL_IMAGE_H
