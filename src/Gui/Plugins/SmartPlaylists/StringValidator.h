/* StringValidator.h */
/*
 * Copyright (C) 2011-2022 Michael Lugmair
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
#ifndef SAYONARA_PLAYER_STRINGVALIDATOR_H
#define SAYONARA_PLAYER_STRINGVALIDATOR_H

#include "Utils/Pimpl.h"
#include "Components/SmartPlaylists/SmartPlaylist.h"

#include <QValidator>

using StringConverterPtr = std::shared_ptr<SmartPlaylists::StringConverter>;
class StringValidator :
	public QValidator
{
	Q_OBJECT
	PIMPL(StringValidator)

	public:
		explicit StringValidator(QObject* parent);
		~StringValidator() override;

		State validate(QString& str, int& i) const override;
		void setStringConverter(const StringConverterPtr& stringConverter);
};

#endif //SAYONARA_PLAYER_STRINGVALIDATOR_H
