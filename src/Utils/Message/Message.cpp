/* GlobalMessage.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#include "Message.h"
#include "MessageReceiverInterface.h"
#include "Utils/Logger/Logger.h"

static MessageReceiverInterface* s_receiver = nullptr;

bool Message::registerReceiver(MessageReceiverInterface* receiver)
{
	if(s_receiver != nullptr)
	{
		spLog(Log::Warning, "GlobalMessage") << "There's also another receiver: " << s_receiver->name();
		return false;
	}

	s_receiver = receiver;

	return true;
}

Message::Answer
Message::info(const QString& info, const QString& sender_name)
{
	if(!s_receiver)
	{
		return Message::Answer::Undefined;
	}

	return s_receiver->infoReceived(info, sender_name);
}

Message::Answer
Message::warning(const QString& warning, const QString& sender_name)
{
	if(!s_receiver)
	{
		return Message::Answer::Undefined;
	}

	return s_receiver->warningReceived(warning, sender_name);
}

Message::Answer
Message::error(const QString& error, const QString& sender_name)
{
	if(!s_receiver)
	{
		return Message::Answer::Undefined;
	}

	return s_receiver->errorReceived(error, sender_name);
}

Message::Answer
Message::question(const QString& question, const QString& sender_name, Message::QuestionType type)
{
	if(!s_receiver)
	{
		return Message::Answer::Undefined;
	}

	return s_receiver->questionReceived(question, sender_name, type);
}

Message::Answer Message::question_yn(const QString& question, const QString& sender_name)
{
	return Message::question(question, sender_name, Message::QuestionType::YesNo);
}

Message::Answer Message::question_ok(const QString& question, const QString& sender_name)
{
	return Message::question(question, sender_name, Message::QuestionType::OkCancel);
}
