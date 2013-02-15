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
	loadCheckBox(qcbRemoveNotificationsOnFocused, r.nkRemoveNotificationsOnFocused);
	loadCheckBox(qcbInvertText, r.nkInvertText);

	loadCheckBox(qcbDisablePositional, r.nkDisablePositional);
	loadCheckBox(qcbDisableOverlay, r.nkDisableOverlay);
	loadCheckBox(qcbDisableTTS, r.nkDisableTTS);
	loadCheckBox(qcbDisableSounds, r.nkDisableSounds);
	loadCheckBox(qcbDisablePopups, r.nkDisablePopups);
	loadCheckBox(qcbDisableAttenuate, r.nkDisableAttenuate);
	loadCheckBox(qcbTrimLog, r.nkTrimLog);

	loadCheckBox(qcbAudioEnable, r.nkAudioEnable);
	loadComboBox(qcbAudioType, r.nkAudioType);
	loadComboBox(qcbAudioNoise, r.nkAudioNoise);
	loadCheckBox(qcbAudioSpeakers, r.nkAudioSpeakers);
}

void M9Config::save() const {
	s.nkRemoveNotificationsOnFocused = qcbRemoveNotificationsOnFocused->isChecked();
	s.nkInvertText = qcbInvertText->isChecked();

	s.nkDisablePositional = qcbDisablePositional->isChecked();
	s.nkDisableOverlay = qcbDisableOverlay->isChecked();
	s.nkDisableTTS = qcbDisableTTS->isChecked();
	s.nkDisableSounds = qcbDisableSounds->isChecked();
	s.nkDisablePopups = qcbDisablePopups->isChecked();
	s.nkDisableAttenuate = qcbDisableAttenuate->isChecked();
	s.nkTrimLog = qcbTrimLog->isChecked();

	s.nkAudioEnable = qcbAudioEnable->isChecked();
	s.nkAudioType = qcbAudioType->currentIndex();
	s.nkAudioNoise = qcbAudioNoise->currentIndex();
	s.nkAudioSpeakers = qcbAudioSpeakers->isChecked();
}

void M9Config::accept() const {
}

bool M9Config::expert(bool b) {
	return true;
}
