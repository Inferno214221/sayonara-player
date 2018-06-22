#include "TagEditItem.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QGridLayout>

#include "Utils/Settings/Settings.h"

struct TagEditItemString::Private
{
	Lang::Term	language_term;
	bool		has_all;
	QString		value;
	QLabel*		label=nullptr;
	QLineEdit*	lineedit=nullptr;
	QCheckBox*	checkbox=nullptr;

	Private(QWidget* parent, Lang::Term language_term, bool has_all) :
		language_term(language_term),
		has_all(has_all)
	{
		lineedit = new QLineEdit(parent);
		label = new QLabel(Lang::get(language_term), parent);

		if(has_all)
		{
			checkbox = new QCheckBox(parent);
		}
	}
};

TagEditItemString::TagEditItemString(Lang::Term language_term, bool has_all, QGridLayout* layout, int row, int column, QWidget* parent)
{
	m = Pimpl::make<Private>(parent, language_term, has_all);

	layout->addWidget(m->label, row, column);
	layout->addWidget(m->lineedit, row, column + 1);

	if(m->checkbox)
	{
		connect(m->checkbox, &QCheckBox::toggled, m->lineedit, &QWidget::setDisabled);
		layout->addWidget(m->checkbox, row, column + 2);
	}

	Set::listen<Set::Player_Language>(this, &TagEditItemString::language_changed);
}

TagEditItemString::~TagEditItemString() {}

QString TagEditItemString::value() const
{
	return m->value;
}

void TagEditItemString::set_value(const QString& value)
{
	m->value = value;
}

void TagEditItemString::language_changed()
{
	m->label->setText(Lang::get(m->language_term));
}



struct TagEditItemInt::Private
{
	Lang::Term	language_term;
	bool		has_all;
	int			value;
	int			lower;
	int			upper;

	QLabel*		label=nullptr;
	QSpinBox*	spinbox=nullptr;
	QCheckBox*	checkbox=nullptr;

	Private(QWidget* parent, Lang::Term language_term, bool has_all) :
		language_term(language_term),
		has_all(has_all)
	{
		spinbox = new QSpinBox(parent);
		label = new QLabel(Lang::get(language_term), parent);

		if(has_all)
		{
			checkbox = new QCheckBox(parent);
		}
	}
};


TagEditItemInt::TagEditItemInt(Lang::Term language_term, bool has_all, QGridLayout* layout, int row, int column, QWidget* parent)
{
	m = Pimpl::make<Private>(parent, language_term, has_all);

	layout->addWidget(m->label, row, column);
	layout->addWidget(m->spinbox, row, column + 1);

	if(m->checkbox)
	{
		connect(m->checkbox, &QCheckBox::toggled, m->spinbox, &QWidget::setDisabled);
		layout->addWidget(m->checkbox, row, column + 2);
	}

	Set::listen<Set::Player_Language>(this, &TagEditItemString::language_changed);
}

TagEditItemInt::~TagEditItemInt() {}

int TagEditItemInt::value() const
{
	return m->spinbox->value();
}

void TagEditItemInt::set_value(const int& value)
{
	m->spinbox->setValue(value);
}

void TagEditItemInt::set_lower(const int& lower)
{
	m->spinbox->setMinimum(lower);
}

void TagEditItemInt::set_upper(const int& upper)
{
	m->spinbox->setMaximum(upper);
}

void TagEditItemInt::language_changed()
{
	m->label->setText(Lang::get(m->language_term));
}

