#ifndef SETTINGCONVERTIBLE_H
#define SETTINGCONVERTIBLE_H

class QString;

class SettingConvertible
{
public:
	SettingConvertible();
	virtual ~SettingConvertible();

	virtual bool loadFromString(const QString& str)=0;
	virtual QString toString() const = 0;
};

#endif // SETTINGCONVERTIBLE_H
