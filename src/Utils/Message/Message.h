/* GlobalMessage.h */

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

#ifndef GLOBALMESSAGE_H
#define GLOBALMESSAGE_H

#include <QString>

class MessageReceiverInterface;

/**
 * @brief The GlobalMessage class
 * @ingroup GUIHelper
 */
namespace Message
{
	enum class Answer : unsigned char
	{
		Yes=0,
		No,
		Ok,
		Cancel,
		Undefined
	};

	enum class QuestionType : unsigned char
	{
		YesNo=0,
		OkCancel
	};

	Message::Answer info(
			const QString& info,
			const QString& sender_name=QString());

	Message::Answer warning(
			const QString& warning,
			const QString& sender_name=QString());

	Message::Answer error(
			const QString& error,
			const QString& sender_name=QString());

	Message::Answer question(
			const QString& question,
			const QString& sender_name,
			QuestionType type);

	Message::Answer question_yn(
			const QString& question,
			const QString& sender_name=QString());

	Message::Answer question_ok(
			const QString& question,
			const QString& sender_name=QString());

	/**
	 * @brief register a receiver here, so it is called whenever a message has to be written
	 * @param receiver the receiver class
	 * @return false, if there's already another receiver. True else
	 */
	bool register_receiver(MessageReceiverInterface* receiver);
}

#endif // GLOBALMESSAGE_H
