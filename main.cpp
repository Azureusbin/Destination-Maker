#include "CMainWindow.h"
#include <QtWidgets/QApplication>

#include <QFile>
#include <QSettings>
#include <QMessageBox>

#include <shlobj_core.h>

#ifdef _DEBUG
#include <fstream>
#include <iostream>
std::ofstream ost("DebugOuput.txt");
#endif

int main(int argc, char *argv[]);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	QString cmdParam = QString::fromWCharArray(GetCommandLineW());

	QStringList list = cmdParam.split('"',QString::SkipEmptyParts);

	int ListSize = list.size();
	QVector<char *> argv;
	QVector<QByteArray> argvTemp(ListSize);

	for (int i = 0; i < ListSize; ++i)
	{
		QString tempStr = list[i].trimmed();

		if (tempStr.isEmpty())
			continue;

		argvTemp[i] = tempStr.toLocal8Bit();
		argv.push_back(argvTemp[i].data());

#ifdef _DEBUG
		ost << i << "  " << argvTemp[i].data() << std::endl;
#endif
	}

#ifdef _DEBUG
	ost << argv.size() << std::endl;
#endif

	int argc = argv.size();

	return main(argc, argv.data());
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	a.setQuitOnLastWindowClosed(false);

	if (argc >= 2 && stricmp(argv[1], "-Reg") == 0)
	{
		QString className("iSystemTool.DMaker");
		QString appPath(qApp->applicationFilePath());
		appPath.replace('/', '\\');
		QString ext(".DESTDAT");
		QString extDes(u8"路径 数据文件");

		QString baseUrl("HKEY_CURRENT_USER\\Software\\Classes");
		QSettings settingClasses(baseUrl, QSettings::NativeFormat);

		settingClasses.setValue("/" + className + "/Shell/Open/Command/.", "\"" + appPath + "\" \"-OpenFile\" \"%1\"");
		settingClasses.setValue("/" + className + "/.", extDes);
		settingClasses.setValue("/" + className + "/DefaultIcon/.", appPath + ",1");
		settingClasses.setValue("/" + ext + "/OpenWithProgIds/" + className, "");
		settingClasses.sync();

		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

		QMessageBox msgbox(nullptr);
		QPushButton* OK = msgbox.addButton(u8"确定", QMessageBox::YesRole);
		msgbox.setIcon(QMessageBox::Information);
		msgbox.setDefaultButton(OK);
		msgbox.setWindowTitle(u8"注册完成");
		msgbox.setText(u8"DMaker 注册完成，点击确定退出。");
		msgbox.exec();

		return 0;
	}

	QString filepath = "stylesheet.qss";
	QFile styleSheetFile(filepath);

	if (styleSheetFile.exists())
	{
		if (styleSheetFile.open(QFile::ReadOnly))
		{
			qApp->setStyleSheet(styleSheetFile.readAll());
		}
	}

	CMainWindow w;
	w.setWindowIcon(QIcon(":/Image/DestinationMaker.ico"));
	w.show();

	if (argc > 2 && stricmp(argv[1], "-OpenFile") == 0)
		w.OpenFile(QString::fromLocal8Bit(argv[2]));

	return a.exec();
}
