/* GUI_PlayerMessages.cpp */

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

#include "GUI_Player.h"
#include "Utils/Language/Language.h"
#include <QMessageBox>

static Message::Answer convertAnswer(int answer)
{
	const auto button = QMessageBox::StandardButton(answer);
	switch(button)
	{
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

Message::Answer GUI_Player::errorReceived(const QString& error, const QString& senderName) // NOLINT(google-default-arguments)
{
	const auto title = senderName.isEmpty()
	                ? Lang::get(Lang::Error) + ":"
	                : senderName + ": " + Lang::get(Lang::Error);

	auto msgBox = QMessageBox(this);
	msgBox.setWindowTitle(title);
	msgBox.setIcon(QMessageBox::Critical);
	msgBox.setText(error);
	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.setButtonText(QMessageBox::Ok, Lang::get(Lang::OK));

	return convertAnswer(msgBox.exec());
}

Message::Answer GUI_Player::warningReceived(const QString& warning, const QString& senderName) // NOLINT(google-default-arguments)
{
	const auto title = senderName.isEmpty()
	                ? Lang::get(Lang::Warning)
	                : senderName + ": " + Lang::get(Lang::Warning);

	auto msgBox = QMessageBox(this);
	msgBox.setWindowTitle(title);
	msgBox.setIcon(QMessageBox::Warning);
	msgBox.setText(warning);
	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.setButtonText(QMessageBox::Ok, Lang::get(Lang::OK));

	return convertAnswer(msgBox.exec());
}

Message::Answer GUI_Player::infoReceived(const QString& info, const QString& senderName) // NOLINT(google-default-arguments)
{
	const auto title = senderName.isEmpty()
	                ? Lang::get(Lang::Info)
	                : senderName + ": " + Lang::get(Lang::Info);

	auto msgBox = QMessageBox(this);
	msgBox.setWindowTitle(title);
	msgBox.setText(info);
	msgBox.setIcon(QMessageBox::Information);
	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.setButtonText(QMessageBox::Ok, Lang::get(Lang::OK));

	return convertAnswer(msgBox.exec());
}

Message::Answer
GUI_Player::questionReceived(const QString& question, const QString& senderName, Message::QuestionType type) // NOLINT(google-default-arguments)
{
	const auto title = senderName.isEmpty()
	                ? Lang::get(Lang::Info)
	                : senderName + ": " + Lang::get(Lang::Info);

	auto msgBox = QMessageBox(this);
	msgBox.setWindowTitle(title);
	msgBox.setText(question);
	msgBox.setIcon(QMessageBox::Question);

	if(type == Message::QuestionType::YesNo)
	{
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		msgBox.setButtonText(QMessageBox::Yes, Lang::get(Lang::Yes));
		msgBox.setButtonText(QMessageBox::No, Lang::get(Lang::No));
	}

	else
	{
		msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
		msgBox.setButtonText(QMessageBox::Ok, Lang::get(Lang::OK));
		msgBox.setButtonText(QMessageBox::Cancel, Lang::get(Lang::Cancel));
	}

	return convertAnswer(msgBox.exec());
}
