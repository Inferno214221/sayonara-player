#include "CalendarWidget.h"
#include "Gui/Utils/Style.h"
#include "Utils/Language/Language.h"

#include <QDate>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPainter>
#include <QPalette>
#include <QPushButton>
#include <QRect>
#include <QVBoxLayout>

namespace
{
	void setBold(QLabel* w)
	{
		auto font = w->font();
		font.setBold(true);
		w->setFont(font);
	}
}

namespace Gui
{
	CalendarWidget::CalendarWidget(QWidget* parent) :
		QCalendarWidget(parent) {}

	CalendarWidget::~CalendarWidget() = default;

	void CalendarWidget::paintCell(QPainter* painter, const QRect& rect, const QDate& date) const
	{
		if(Style::isDark())
		{
			const auto isInvalidDate = (date < minimumDate() || date > maximumDate());
			if(isInvalidDate)
			{
				const auto color = palette().color(QPalette::ColorGroup::Disabled, QPalette::ColorRole::WindowText);

				painter->setPen(color);
				painter->drawText(rect, {Qt::AlignHCenter | Qt::AlignVCenter}, QString::number(date.day()));

				return;
			}
		}

		QCalendarWidget::paintCell(painter, rect, date);
	}

	struct CalendarDialog::Private
	{
		QDialogButtonBox* buttonBox {new QDialogButtonBox()};
		CalendarWidget* calendarWidget {new CalendarWidget()};

		Private()
		{
			buttonBox->setStandardButtons({QDialogButtonBox::Ok | QDialogButtonBox::Cancel});

			auto* btnOk = buttonBox->button(QDialogButtonBox::StandardButton::Ok);
			btnOk->setText(Lang::get(Lang::OK));

			auto* btnCancel = buttonBox->button(QDialogButtonBox::StandardButton::Cancel);
			btnCancel->setText(Lang::get(Lang::Cancel));
		}
	};

	CalendarDialog::CalendarDialog(const QString& title, const QString& text, QWidget* parent) :
		QDialog {parent},
		m {Pimpl::make<Private>()}
	{
		auto* layout = new QVBoxLayout();
		auto* labelTitle = new QLabel(title);
		auto* labelText = new QLabel(text);

		setBold(labelTitle);
		
		layout->addWidget(labelTitle);
		layout->addWidget(labelText);
		layout->addWidget(m->calendarWidget);
		layout->addWidget(m->buttonBox);

		setLayout(layout);

		connect(m->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
		connect(m->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	}

	CalendarDialog::~CalendarDialog() noexcept = default;

	QDate CalendarDialog::selectedDate() const
	{
		return m->calendarWidget->selectedDate();
	}
}