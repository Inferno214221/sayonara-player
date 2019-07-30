#include "Translator.h"

#include "Utils/Utils.h"
#include "Utils/Language/LanguageUtils.h"
#include "Utils/Logger/Logger.h"

#include <QTranslator>
#include <QApplication>

struct Translator::Private
{
	QList<QTranslator*> translators;
	QString current_language;
};

Translator::Translator()
{
	 m = Pimpl::make<Private>();
}

Translator::~Translator() = default;

bool Translator::switch_translator(QObject* parent, const QString& four_letter, const QString& dir)
{
	QString filename = Util::Language::get_used_language_file(four_letter);

	QTranslator* t = new QTranslator(parent);
	bool loaded = t->load(filename, dir);
	if(!loaded){
		sp_log(Log::Warning, this) << "Translator " << dir << "/" << filename << " could not be loaded";
		return false;
	}

	bool installed = QApplication::installTranslator(t);
	if(!installed){
		sp_log(Log::Warning, this) << "Translator " << dir << "/" << filename << " could not be installed";
		return false;
	}

	m->translators << t;
	return true;
}

void Translator::change_language(QObject* parent, const QString& language)
{
	for(QTranslator* t : m->translators)
	{
		QApplication::removeTranslator(t);
	}

	m->translators.clear();
	m->current_language = language;

	switch_translator(parent, m->current_language, Util::share_path("translations/"));
}
