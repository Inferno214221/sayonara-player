/* MinMaxIntegerDialog.h */
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
#ifndef SAYONARA_PLAYER_MINMAXINTEGERDIALOG_H
#define SAYONARA_PLAYER_MINMAXINTEGERDIALOG_H

#include "Utils/Pimpl.h"

#include <QDialog>

class SmartPlaylist;
class QLineEdit;

namespace SmartPlaylists
{
	enum class Type;
}

class MinMaxIntegerDialog :
	public QDialog
{
	Q_OBJECT
	PIMPL(MinMaxIntegerDialog)

	public:
		explicit MinMaxIntegerDialog(QWidget* parent);
		MinMaxIntegerDialog(const std::shared_ptr<SmartPlaylist>& smartPlaylist, QWidget* parent);
		~MinMaxIntegerDialog() override;

		[[nodiscard]] int fromValue() const;
		[[nodiscard]] int toValue() const;

		[[nodiscard]] SmartPlaylists::Type type() const;

	private:
		MinMaxIntegerDialog(SmartPlaylists::Type type, QWidget* parent);
		
		void fillLayout(QWidget* headerWidget);

	private slots: // NOLINT(readability-redundant-access-specifiers)
		void textChanged(const QString& text);
		void currentIndexChanged(int index);
};

#endif //SAYONARA_PLAYER_MINMAXINTEGERDIALOG_H
