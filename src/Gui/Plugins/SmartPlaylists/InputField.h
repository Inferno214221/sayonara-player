/* InputField.h */
/*
 * Copyright (C) 2011-2024 Michael Lugmair
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
#ifndef SAYONARA_PLAYER_INPUTFIELD_H
#define SAYONARA_PLAYER_INPUTFIELD_H

#include "Utils/Pimpl.h"

#include <QLineEdit>
#include <optional>

namespace SmartPlaylists
{
	enum class InputFormat;
	class StringConverter;
}

class InputField :
	public QLineEdit
{
	Q_OBJECT
	PIMPL(InputField)

	public:
		explicit InputField(QWidget* parent = nullptr);
		~InputField() override;

		std::optional<int> data() const;
		void setData(SmartPlaylists::InputFormat inputFormat,
		             const std::shared_ptr<SmartPlaylists::StringConverter>& converter, const int value);

	private slots:
		void mousePressed(QMouseEvent* e);
		void timeSpanAccepted();
		void calendarAccepted(const QDate& date);

	private:
		using QLineEdit::setText;
		using QLineEdit::text;
};

#endif //SAYONARA_PLAYER_INPUTFIELD_H
