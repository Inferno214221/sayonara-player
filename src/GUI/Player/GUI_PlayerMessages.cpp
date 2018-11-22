/* GUI_PlayerMessages.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#include "GUI_Player.h"
#include "Utils/Language.h"
#include <QMessageBox>

Message::Answer convert_answer(QMessageBox::StandardButton answer)
{
	switch(answer){
		case QMessageBox::Ok:
			return Message::Answer::Ok;

		case QMessageBox::Cancel:
		case QMessageBox::Abort:
			return Message::Answer::Cancel;

		case QMessageBox::Yes:
			return Message::Answer::Yes;

		case QMessageBox::No:
			return Message::Answer::No;

		default:
			return Message::Answer::Undefined;
	}
}

Message::Answer GUI_Player::error_received(const QString &error, const QString &sender_name)
{
	QString title = sender_name.isEmpty()
			? Lang::get(Lang::Error) + ":"
			: sender_name + ": " + Lang::get(Lang::Error);

	return convert_answer (QMessageBox::critical(this, title, error));
}

Message::Answer GUI_Player::warning_received(const QString &warning, const QString &sender_name)
{
	QString title = sender_name.isEmpty()
			? Lang::get(Lang::Warning)
			: sender_name + ": " + Lang::get(Lang::Warning);

	return convert_answer (QMessageBox::warning(this, title, warning));
}

Message::Answer GUI_Player::info_received(const QString &info, const QString &sender_name)
{
	QString title = sender_name.isEmpty()
			? Lang::get(Lang::Info)
			: sender_name + ": " + Lang::get(Lang::Info);

	return convert_answer (QMessageBox::information(this, title, info));
}

Message::Answer GUI_Player::question_received(const QString &question, const QString &sender_name, Message::QuestionType type)
{
	QString title = sender_name.isEmpty()
			? Lang::get(Lang::Info)
			: sender_name + ": " + Lang::get(Lang::Info);

	if(type == Message::QuestionType::YesNo){
		return convert_answer(QMessageBox::information(this, title, question, QMessageBox::Yes, QMessageBox::No));
	}

	else {
		return convert_answer(QMessageBox::information(this, title, question, QMessageBox::Ok, QMessageBox::Cancel));
	}
}
