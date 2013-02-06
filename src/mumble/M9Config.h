#ifndef M9CONFIG_H_
#define M9CONFIG_H_

#include "ConfigDialog.h"
#include "ui_M9Config.h"

class M9Config : public ConfigWidget, Ui::M9Config {
	private:
		Q_OBJECT
		Q_DISABLE_COPY(M9Config)
	public:
		M9Config(Settings &st);
		virtual QString title() const;
		virtual QIcon icon() const;
	public slots:
		void accept() const;
		void save() const;
		void load(const Settings &r);
		bool expert(bool b);
};

#endif
