#include "StdAfx.h"

#include "SAMPDialog.h"

SAMPDialog::SAMPDialog() {}

SAMPDialog::~SAMPDialog() {}

void SAMPDialog::reset() {
	Lock lock(&_sampDialogMutex);

	_dialogActive = false;
	_dialogStyle = 0;
	_dialogId = 0;
}

void SAMPDialog::setDialogActive(bool dialogActive) {
	Lock lock(&_sampDialogMutex);

	_dialogActive = dialogActive;
}

bool SAMPDialog::isDialogActive() {
	Lock lock(&_sampDialogMutex);

	return _dialogActive;
}

void SAMPDialog::setDialogStyle(uint8_t dialogStyle) {
	Lock lock(&_sampDialogMutex);

	_dialogStyle = dialogStyle;
}

uint8_t SAMPDialog::getDialogStyle() {
	Lock lock(&_sampDialogMutex);

	return _dialogStyle;
}

void SAMPDialog::setDialogId(uint16_t dialogId) {
	Lock lock(&_sampDialogMutex);

	_dialogId = dialogId;
}

uint16_t SAMPDialog::getDialogId() {
	Lock lock(&_sampDialogMutex);

	return _dialogId;
}

void SAMPDialog::setDialogTitle(std::string dialogTitle) {
	Lock lock(&_sampDialogMutex);

	_dialogTitle = dialogTitle;
}

std::string SAMPDialog::getDialogTitle() {
	Lock lock(&_sampDialogMutex);

	return _dialogTitle;
}

void SAMPDialog::setOkButtonText(std::string okButtonText) {
	Lock lock(&_sampDialogMutex);

	_okButtonText = okButtonText;
}

std::string SAMPDialog::getOkButtonText() {
	Lock lock(&_sampDialogMutex);

	return _okButtonText;
}

void SAMPDialog::setCancelButtonText(std::string cancelButtonText) {
	Lock lock(&_sampDialogMutex);

	_cancelButtonText = cancelButtonText;
}

std::string SAMPDialog::getCancelButtonText() {
	Lock lock(&_sampDialogMutex);

	return _cancelButtonText;
}

void SAMPDialog::setDialogText(std::string dialogText) {
	Lock lock(&_sampDialogMutex);

	_dialogText = dialogText;
}

std::string SAMPDialog::getDialogText() {
	Lock lock(&_sampDialogMutex);

	return _dialogText;
}
