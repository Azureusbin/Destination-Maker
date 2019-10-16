#pragma once

#include <QTimer>
#include <QDialog>
#include <QtWidgets/QMainWindow>
#include "ui_MainForm.h"
#include "ui_EditForm.h"

#include "DestSystem\DestinationManager.h"

class CEditor : public QDialog
{
	Q_OBJECT

public:
	CEditor(QWidget *parent = Q_NULLPTR);

	void setEditMode(SDestinationV2&);
	int clickedButton() { return m_ButtonId; }

	SDestinationV2 GetDATA();

private:
	void CloseAfterCheck(int);
	bool checkData();

	Ui::EditForm ui;
	int m_ButtonId = -1;
};

class CMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	CMainWindow(QWidget *parent = Q_NULLPTR);

	virtual void closeEvent(QCloseEvent*) override;
	virtual void paintEvent(QPaintEvent *e) override;

	void OpenFile(QString);

	void UpdateTitle(QString);
	void UpdateUI();

	void OnClickCenterTips();
	bool OnShouldSaveFile();

	void OnNewFile();
	void OnOpenFile();
	void OnCloseFile();
	void OnSaveFile();
	void OnSaveAs();

	void OnAdd();
	void OnEdit();
	void OnDelete();

	void OnSearch();

	void SetShouldSave(const char* szFileName);

private:
	Ui::CMainWindowClass ui;

	QString m_currentPath;
	QString m_currentFile;

	bool m_bNeedSave = false;
	bool m_bEnableSearch = false;

	QTimer* m_pTimer;

	QWidget* createItemWidget(SDestinationV2&);
	void updateListWidget();
	void updateActions();
};
