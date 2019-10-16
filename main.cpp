#include "CMainWindow.h"
#include <QtWidgets/QApplication>

#include <QFile>
#include <QSettings>

#include <shlobj_core.h>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	a.setQuitOnLastWindowClosed(false);
	a.setWindowIcon(QIcon(":/Image/Default.ico"));

	QString filepath = "stylesheet.qss";
	QFile styleSheetFile(filepath);

	if (styleSheetFile.exists())
	{
		if (styleSheetFile.open(QFile::ReadOnly))
		{
			qApp->setStyleSheet(styleSheetFile.readAll());
		}
	}

	QString className("iSystemTool.DMaker");
	QString appPath(argv[0]);
	QString ext(".DESTDAT");
	QString extDes(u8"路径 数据文件");

	QString baseUrl("HKEY_CURRENT_USER\\Software\\Classes");
	QSettings settingClasses(baseUrl, QSettings::NativeFormat);

	settingClasses.setValue("/" + className + "/Shell/Open/Command/.", "\"" + appPath + "\" \"%1\"");
	settingClasses.setValue("/" + className + "/.", extDes);
	settingClasses.setValue("/" + className + "/DefaultIcon/.", appPath + ",1");
	settingClasses.setValue("/" + ext + "/OpenWithProgIds/" + className, "");
	settingClasses.sync();

	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

	CMainWindow w;
	w.show();

	if (argc>=2 && argv[1] != NULL)
	{
		w.OpenFile(QString::fromLocal8Bit(argv[1]));
	}

	return a.exec();
}
