#include "StdAfx.h"

#include "SAMPDialog.h"

SAMPDialog::SAMPDialog() {}

SAMPDialog::~SAMPDialog() {}

void SAMPDialog::reset() {
	_dialogActive = false;
	_dialogStyle = 0;
	_dialogId = 0;
}

void SAMPDialog::setDialogActive(bool dialogActive) {
	_dialogActive = dialogActive;
}

bool SAMPDialog::isDialogActive() {
	return _dialogActive;
}

void SAMPDialog::setDialogStyle(uint8_t dialogStyle) {
	_dialogStyle = dialogStyle;
}

uint8_t SAMPDialog::getDialogStyle() {
	return _dialogStyle;
}

void SAMPDialog::setDialogId(uint16_t dialogId) {
	_dialogId = dialogId;
}

uint16_t SAMPDialog::getDialogId() {
	return _dialogId;
}

void SAMPDialog::setDialogTitle(std::string dialogTitle) {
	_dialogTitle = dialogTitle;
}

std::string SAMPDialog::getDialogTitle() {
	return _dialogTitle;
}

void SAMPDialog::setOkButtonText(std::string okButtonText) {
	_okButtonText = okButtonText;
}

std::string SAMPDialog::getOkButtonText() {
	return _okButtonText;
}

void SAMPDialog::setCancelButtonText(std::string cancelButtonText) {
	_cancelButtonText = cancelButtonText;
}

std::string SAMPDialog::getCancelButtonText() {
	return _cancelButtonText;
}

void SAMPDialog::setDialogText(std::string dialogText) {
	_dialogText = dialogText;
}

std::string SAMPDialog::getDialogText() {
	return _dialogText;
}
