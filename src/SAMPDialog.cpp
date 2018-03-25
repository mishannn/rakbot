#include "StdAfx.h"

#include "SAMPDialog.h"

SAMPDialog::SAMPDialog() { }

SAMPDialog::~SAMPDialog() { }

void SAMPDialog::reset() {
	lock();
	_dialogActive = false;
	_dialogStyle = 0;
	_dialogId = 0;
	unlock();
}

void SAMPDialog::setDialogActive(bool dialogActive) {
	lock();
	_dialogActive = dialogActive;
	unlock();
}

bool SAMPDialog::isDialogActive() {
	return _dialogActive;
}

void SAMPDialog::setDialogStyle(uint8_t dialogStyle) {
	lock();
	_dialogStyle = dialogStyle;
	unlock();
}

uint8_t SAMPDialog::getDialogStyle() {
	return _dialogStyle;
}

void SAMPDialog::setDialogId(uint16_t dialogId) {
	lock();
	_dialogId = dialogId;
	unlock();
}

uint16_t SAMPDialog::getDialogId() {
	return _dialogId;
}

void SAMPDialog::setDialogTitle(std::string dialogTitle) {
	lock();
	_dialogTitle = dialogTitle;
	unlock();
}

std::string SAMPDialog::getDialogTitle() {
	return _dialogTitle;
}

void SAMPDialog::setOkButtonText(std::string okButtonText) {
	lock();
	_okButtonText = okButtonText;
	unlock();
}

std::string SAMPDialog::getOkButtonText() {
	return _okButtonText;
}

void SAMPDialog::setCancelButtonText(std::string cancelButtonText) {
	lock();
	_cancelButtonText = cancelButtonText;
	unlock();
}

std::string SAMPDialog::getCancelButtonText() {
	return _cancelButtonText;
}

void SAMPDialog::setDialogText(std::string dialogText) {
	lock();
	_dialogText = dialogText;
	unlock();
}

std::string SAMPDialog::getDialogText() {
	return _dialogText;
}
