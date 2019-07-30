#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include "Utils/Singleton.h"
#include "Utils/Pimpl.h"

#include <QObject>

class Translator
{
	SINGLETON(Translator)
	PIMPL(Translator)

private:
	bool switch_translator(QObject* parent, const QString& four_letter, const QString& dir);

public:
	void change_language(QObject* parent, const QString& language);

};

#endif // TRANSLATOR_H
