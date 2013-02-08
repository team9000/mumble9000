#include "mumble_pch.hpp"

#include "M9Config.h"

#include "Global.h"
#include "MainWindow.h"
#include "OSInfo.h"

static ConfigWidget *M9ConfigNew(Settings &st) {
	return new M9Config(st);
}

static ConfigRegistrar registrar(999999, M9ConfigNew);

M9Config::M9Config(Settings &st) : ConfigWidget(st) {
	setupUi(this);
}

QString M9Config::title() const {
	return tr("Mumble9000");
}

QIcon M9Config::icon() const {
	return QIcon(QLatin1String("skin:config_network.png"));
}

void M9Config::load(const Settings &r) {
	loadCheckBox(qcbRemoveNotifications, r.nkRemoveNotifications);
	loadCheckBox(qcbRemoveNotificationsOnFocused, r.nkRemoveNotificationsOnFocused);
	loadCheckBox(qcbInvertText, r.nkInvertText);
	loadCheckBox(qcbFixAudio, r.nkFixAudio);
	loadCheckBox(qcbDisablePositional, r.nkDisablePositional);
	loadCheckBox(qcbDisableOverlay, r.nkDisableOverlay);
}

void M9Config::save() const {
	s.nkRemoveNotifications = qcbRemoveNotifications->isChecked();
	s.nkRemoveNotificationsOnFocused = qcbRemoveNotificationsOnFocused->isChecked();
	s.nkInvertText = qcbInvertText->isChecked();
	s.nkFixAudio = qcbFixAudio->isChecked();
	s.nkDisablePositional = qcbDisablePositional->isChecked();
	s.nkDisableOverlay = qcbDisableOverlay->isChecked();
}

void M9Config::accept() const {
}

bool M9Config::expert(bool b) {
	return true;
}
