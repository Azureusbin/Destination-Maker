#include "CMainWindow.h"

#include "ui_ItemWidget.h"

#include <QIcon>
#include <QPainter>
#include <QFileInfo>
#include <QClipboard>
#include <QPushButton>
#include <QFileDialog>
#include <QCloseEvent>
#include <QMessageBox>

#include "qdebug.h"


#include "WinAreaDef.h"

const char* default_title = u8"目的地数据制作器";
const char* author_text = "By AzureuBin 2019/9/25";
const char* software_version_ = "version 1.0.3";

CMainWindow::CMainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	setWindowTitle(default_title);

	connect(ui.QA_Exit, &QAction::triggered, [this]() {this->close(); });
	connect(ui.QA_About, &QAction::triggered, [this]() 
	{
		// TODO:Show info about author.
		QDialog aboutDialog;
		QGridLayout* pLayout = new QGridLayout(&aboutDialog);
		QPushButton* Ok = new QPushButton(u8"确定", &aboutDialog);

		QPixmap pixmap(":/Image/Gear.png");
		pixmap.scaled(QSize(64, 64));
		QLabel* pIcon = new QLabel(&aboutDialog);
		pIcon->setPixmap(pixmap);

		QFont font(u8"微软雅黑", 20);
		QLabel* info = new QLabel(author_text, &aboutDialog);
		QLabel* info2 = new QLabel(software_version_, &aboutDialog);
		info->setFont(font);
		info2->setFont(font);

		connect(Ok, &QPushButton::clicked, &aboutDialog, &QDialog::accept);

		pLayout->addWidget(pIcon, 0, 0, 2, 1);
		pLayout->addWidget(info, 0, 1, 1, 1);
		pLayout->addWidget(info2, 1, 1, 1, 1, Qt::AlignRight);
		pLayout->addWidget(Ok, 2, 1, 1, 1, Qt::AlignRight);

		aboutDialog.layout()->setSizeConstraint(QLayout::SetFixedSize);
		aboutDialog.setWindowFlags(aboutDialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		aboutDialog.setWindowTitle(u8"关于");
		aboutDialog.setLayout(pLayout);
		aboutDialog.setModal(true);
		aboutDialog.exec();
	});

	connect(ui.label, &QLabel::linkActivated, this, &CMainWindow::OnClickCenterTips);

	connect(ui.QA_New, &QAction::triggered, this, &CMainWindow::OnNewFile);
	connect(ui.QA_Open, &QAction::triggered, this, &CMainWindow::OnOpenFile);
	connect(ui.QA_CloseFile, &QAction::triggered, this, &CMainWindow::OnCloseFile);
	connect(ui.QA_Save, &QAction::triggered, this, &CMainWindow::OnSaveFile);
	connect(ui.QA_SaveAs, &QAction::triggered, this, &CMainWindow::OnSaveAs);

	ui.mainWidget->hide();

	connect(ui.Btn_Add, &QPushButton::clicked, this, &CMainWindow::OnAdd);
	connect(ui.Btn_Edit, &QPushButton::clicked, this, &CMainWindow::OnEdit);
	connect(ui.Btn_Del, &QPushButton::clicked, this, &CMainWindow::OnDelete);

	connect(ui.listWidget, &QListWidget::doubleClicked, this, &CMainWindow::OnEdit);

	// Search function
	m_pTimer = new QTimer;
	m_pTimer->setInterval(800);
	connect(m_pTimer, &QTimer::timeout, this, &CMainWindow::OnSearch);
	connect(ui.lineEdit, &QLineEdit::textEdited, [this]() {m_pTimer->start(); });

	updateActions();
}

void CMainWindow::closeEvent(QCloseEvent *pEvent)
{
	if (CDestinationManager::Instance()->IsFileOpen() && m_bNeedSave)
	{
		if (OnShouldSaveFile() == false)
		{
			pEvent->ignore();
			return;
		}
	}

	pEvent->accept();
	qApp->quit();
}

void CMainWindow::paintEvent(QPaintEvent * e)
{
	QMainWindow::paintEvent(e);
}

void CMainWindow::OpenFile(QString file)
{
	QFileInfo info(file);

	if (!info.isFile() || !info.exists() || !CDestinationManager::Instance()->OpenFile(file.toLocal8Bit().constData()))
	{
		QMessageBox::critical(this, u8"错误", u8"打开文件失败, 可能是文件已损坏。", u8"关闭", u8"");
		return;
	}

	m_bNeedSave = false;
	m_currentFile = info.fileName();
	m_currentPath = info.path() + "/";
	UpdateTitle(m_currentFile);
	UpdateUI();
	updateListWidget();
	updateActions();
}

void CMainWindow::UpdateTitle(QString fileName)
{
	QString temp = fileName + u8" - DEST制作器";
	setWindowTitle(temp);
}

void CMainWindow::UpdateUI()
{
	if (CDestinationManager::Instance()->IsFileOpen())
	{
		ui.welcomeWidget->hide();
		ui.mainWidget->show();
	}
	else
	{
		ui.welcomeWidget->show();
		ui.mainWidget->hide();
	}
}

void CMainWindow::OnClickCenterTips()
{
	QMessageBox msgbox(this);
	QPushButton* New = msgbox.addButton(u8"新建文件", QMessageBox::YesRole);
	QPushButton* Open = msgbox.addButton(u8"打开文件", QMessageBox::NoRole);
	QPushButton* Cancel = msgbox.addButton(u8"取消", QMessageBox::RejectRole);
	msgbox.setIcon(QMessageBox::Question);
	msgbox.setDefaultButton(New);
	msgbox.setWindowTitle(u8"您想做些什么？");
	msgbox.setText(u8"请选择一种操作");
	msgbox.exec();

	auto cbtn = msgbox.clickedButton();

	if (cbtn == New)
	{
		OnNewFile();
	}
	else if (cbtn == Open)
	{
		OnOpenFile();
	}
}

bool CMainWindow::OnShouldSaveFile()
{
	auto pMgr = CDestinationManager::Instance();

	QMessageBox msgbox(QMessageBox::Question, u8"保存文件", u8"是否保存当前文件？", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, this);
	msgbox.setButtonText(QMessageBox::Yes, u8"保存");
	msgbox.setButtonText(QMessageBox::No, u8"不保存");
	msgbox.setButtonText(QMessageBox::Cancel, u8"取消");
	int result = msgbox.exec();

	if (result == QMessageBox::Cancel)
		return false;

	if (result == QMessageBox::Yes)
	{
		QString SavePath = m_currentPath + m_currentFile;
		pMgr->SaveFile(SavePath.toLocal8Bit().constData());
	}

	pMgr->CloseFile();

	return true;
}

void CMainWindow::OnNewFile()
{
	if (CDestinationManager::Instance()->IsFileOpen() && m_bNeedSave)
	{
		if (OnShouldSaveFile() == false)
			return;
	}

	CDestinationManager::Instance()->NewFile();

	m_currentPath = ".";
	m_currentFile = u8"无标题.DESTDAT";

	SetShouldSave(m_currentFile.toUtf8().constData());
	
	UpdateUI();
	updateListWidget();
	updateActions();
}

void CMainWindow::OnOpenFile()
{
	if (CDestinationManager::Instance()->IsFileOpen() && m_bNeedSave)
	{
		if (OnShouldSaveFile() == false)
			return;
	}

	QString openFile = QFileDialog::getOpenFileName(this, u8"打开目的地数据", m_currentPath, u8"目的地数据集(*.DESTDAT)");

	if (!openFile.isEmpty())
	{
		// OPEN FILE
		OpenFile(openFile);
	}
}

void CMainWindow::OnCloseFile()
{
	if (CDestinationManager::Instance()->IsFileOpen() && m_bNeedSave)
	{
		if (OnShouldSaveFile() == false)
			return;
	}

	// Make sure file was closed.
	CDestinationManager::Instance()->CloseFile();

	m_bNeedSave = false;
	m_currentFile = "";
	m_currentPath = "";
	setWindowTitle(default_title);
	updateListWidget();
	UpdateUI();
	updateActions();
}

void CMainWindow::OnSaveFile()
{
	QString savePath = m_currentPath + m_currentFile;

	if (m_currentFile.isEmpty() || m_currentPath.isEmpty())
	{
		savePath = QFileDialog::getSaveFileName(this, u8"文件保存到", m_currentPath, u8"目的地数据集(*.DESTDAT)");

		if (savePath.isEmpty())
			return;

		QFileInfo info(savePath);
		m_currentFile = info.fileName();
		m_currentPath = info.path() + "/";
	}

	UpdateTitle(m_currentFile);

	m_bNeedSave = false;
	CDestinationManager::Instance()->SaveFile(savePath.toLocal8Bit().constData());
}

void CMainWindow::OnSaveAs()
{
	QString savePath = QFileDialog::getSaveFileName(this, u8"另存为", m_currentPath, u8"目的地数据集(*.DESTDAT)");

	if (savePath.isEmpty())
		return;

	QFileInfo info(savePath);
	m_currentFile = info.fileName();
	m_currentPath = info.path() + "/";
	UpdateTitle(m_currentFile);

	m_bNeedSave = false;
	CDestinationManager::Instance()->SaveFile(savePath.toLocal8Bit().constData());
}

void CMainWindow::OnAdd()
{
	CEditor editor(this);
	editor.exec();

	int bid = editor.clickedButton();

	switch (bid)
	{
		case 0:
		case 1:
		{
			SDestinationV2 DATA = editor.GetDATA();
			CDestinationManager::Instance()->AddDestination(DATA);

			QWidget* itemWidget = createItemWidget(DATA);
			QListWidgetItem* item = new QListWidgetItem;
			item->setSizeHint(QSize(0, 90));
			ui.listWidget->addItem(item);
			ui.listWidget->setItemWidget(item, itemWidget);

			SetShouldSave(m_currentFile.toUtf8().constData());

			if (bid == 1)
				OnAdd();
		}
		break;
	}
}

void CMainWindow::OnEdit()
{
	QListWidgetItem *currentItem = ui.listWidget->currentItem();
	if (!currentItem)
		return;

	QWidget* itemWidget = ui.listWidget->itemWidget(currentItem);

	// GET DATA
	QGroupBox*pGroupbox = itemWidget->findChild<QGroupBox*>("groupBox", Qt::FindDirectChildrenOnly);
	QLabel*pLabelPY = itemWidget->findChild<QLabel*>("l_PY");
	QLabel*pLabelFloor = itemWidget->findChild<QLabel*>("l_floor");
	QLabel*pUidLabel = itemWidget->findChild<QLabel*>("UID");

	SDestinationV2 oldDATA;
	sprintf(oldDATA.name, pGroupbox->title().toUtf8().constData());
	sprintf(oldDATA.initialsPy, pLabelPY->text().toLatin1().constData());
	oldDATA.floorNum = pLabelFloor->text().replace("F","", Qt::CaseInsensitive).toInt();
	oldDATA.uid = DEST_UID::FromStringInternal(pUidLabel->text().toLatin1().data());

	CEditor editor(this);
	editor.setEditMode(oldDATA);
	editor.exec();

	if (editor.clickedButton() == 0)
	{
		SDestinationV2 newDATA = editor.GetDATA();

		// update ListItemWidget
		pGroupbox->setTitle(newDATA.name);
		pLabelPY->setText(newDATA.initialsPy);
		pLabelFloor->setText(QString::number(newDATA.floorNum)+"F");

		SetShouldSave(m_currentFile.toUtf8().constData());

		CDestinationManager::Instance()->ReplaceDestination(oldDATA, newDATA);
	}
}

void CMainWindow::OnDelete()
{
	QListWidgetItem *currentItem = ui.listWidget->currentItem();
	if (!currentItem)
		return;

	QWidget* itemWidget = ui.listWidget->itemWidget(currentItem);
	QGroupBox*pGroupbox = itemWidget->findChild<QGroupBox*>("groupBox", Qt::FindDirectChildrenOnly);

	SDestinationV2 DATA = { 0 };
	sprintf(DATA.name, pGroupbox->title().toUtf8().constData());

	SetShouldSave(m_currentFile.toUtf8().constData());

	// using DATA.name to delete destination.
	CDestinationManager::Instance()->RemoveDestination(DATA);

	delete currentItem;
}

void CMainWindow::OnSearch()
{
	m_pTimer->stop();

	m_bEnableSearch = !ui.lineEdit->text().isEmpty();
	updateListWidget();
}

void CMainWindow::SetShouldSave(const char* szFileName)
{
	m_bNeedSave = true;
	UpdateTitle(szFileName + QString("*"));
}

QWidget * CMainWindow::createItemWidget(SDestinationV2& DATA)
{
	Ui::ItemWidget ui;
	QWidget* pWidget = new QWidget;
	ui.setupUi(pWidget);
	
	ui.groupBox->setTitle(DATA.name);
	ui.l_PY->setText(DATA.initialsPy);
	ui.l_floor->setText(QString::number(DATA.floorNum)+"F");

	char data[16];
	DATA.GetUID(data, 16);
	ui.UID->setText(data);

	pWidget->setLayout(ui.gridLayout);

	return pWidget;
}

void CMainWindow::updateListWidget()
{
	ui.listWidget->clear();

	if (CDestinationManager::Instance()->IsFileOpen())
	{
		std::vector<SDestinationV2> dataVector;

		if (m_bEnableSearch)
		{
			QString searchKey = ui.lineEdit->text();
			CDestinationManager::Instance()->searchByPinYin(searchKey.toLatin1().constData(), dataVector);
		}
		else
		{
			auto tempData = CDestinationManager::Instance()->getData();
			dataVector = *tempData;
		}
		

		for (auto DATA : dataVector)
		{
			QWidget* itemWidget = createItemWidget(DATA);
			QListWidgetItem* item = new QListWidgetItem;
			item->setSizeHint(QSize(0,90));
			ui.listWidget->addItem(item);
			ui.listWidget->setItemWidget(item, itemWidget);
		}
	}
}

void CMainWindow::updateActions()
{
	bool bIsFileOpen = CDestinationManager::Instance()->IsFileOpen();
	ui.QA_Save->setEnabled(bIsFileOpen);
	ui.QA_SaveAs->setEnabled(bIsFileOpen);
	ui.QA_CloseFile->setEnabled(bIsFileOpen);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


CEditor::CEditor(QWidget *parent) : QDialog(parent)
{
	ui.setupUi(this);
	setWindowTitle(u8"添加");

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	m_ButtonId = -1;

	connect(ui.ADD, &QPushButton::clicked, [this]() { CloseAfterCheck(0); });
	connect(ui.ADD_C, &QPushButton::clicked, [this]() { CloseAfterCheck(1);  });
	connect(ui.CANCEL, &QPushButton::clicked, [this]() {m_ButtonId = 2; close(); });
	connect(ui.UID_CPY, &QPushButton::clicked, [this]() 
	{
		QApplication::clipboard()->setText(ui.UID_LE->text());
	});
	connect(ui.UID_GEN, &QPushButton::clicked, [this]() 
	{
		DEST_UID uid; 
		uid.GenerateUID(); 
		char data[16];
		uid.ToString(data, 16);
		ui.UID_LE->setText(data);
	});
}

void CEditor::setEditMode(SDestinationV2& DATA)
{
	setWindowTitle(u8"编辑");

	ui.ADD->setText(u8"修改");
	ui.ADD_C->hide();

	ui.T1->setText(DATA.name);
	ui.T2->setText(DATA.initialsPy);
	ui.spinBox->setValue(DATA.floorNum);

	char data[16];
	DATA.uid.ToString(data, 16);
	ui.UID_LE->setText(data);
}

SDestinationV2 CEditor::GetDATA()
{
	QString name = ui.T1->text();
	QString PY = ui.T2->text().toUpper();
	QString UID_QS = ui.UID_LE->text();

	SDestinationV2 data = { 0 };

	QByteArray nameQBA = name.toUtf8();
	QByteArray pyQBA = PY.toUtf8();

	memcpy_s(data.name, sizeof(data.name), nameQBA.constData(), strlen(nameQBA.constData()));
	memcpy_s(data.initialsPy, sizeof(data.initialsPy), pyQBA.constData(), strlen(pyQBA.constData()));

	data.uid = DEST_UID::FromStringInternal(UID_QS.toLatin1().data());
	data.floorNum = ui.spinBox->value();

	return data;
}

void CEditor::CloseAfterCheck(int id)
{
	bool result = checkData();

	if (result)
	{
		m_ButtonId = id;
		this->close();
		return;
	}
	
	QMessageBox::warning(this, u8"无效的数据", u8"输入的数据无效，无法添加 ", u8"确定", u8"");
}

bool CEditor::checkData()
{
	QString name = ui.T1->text();
	QString PY = ui.T2->text();

	if (name.isEmpty() || PY.isEmpty())
		return false;

	return PY.contains(QRegExp("^[a-zA-Z]+$"));
}
