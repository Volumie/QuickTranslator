#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QMainWindow>
#include <QProcess>
#include <QxtGlobalShortcut>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include <QModelIndex>
#include <QPair>
#include <QListWidget>

#include "translate.h"
#include "box.h"
#include "grablineedit.h"
#include "settings.h"
#include "autorun.h"


namespace Ui {
    class Widget;
}


class Widget : public QMainWindow
{
   Q_OBJECT
   
public:
   explicit Widget(QMainWindow *parent = 0);
   ~Widget();

    void setFromLanguage(QString str);
    void setToLanguage(QString str);
    void langListInit(QString url, bool initList = true);
    void resizeEvent(QResizeEvent *ev);
    void moveEvent(QMoveEvent *ev);

    QStringList      themeTextColor;
    QString          getTTColor(); // getThemeTextColor
   

   
private:
   void trayMenuInit();
   void langListInit();
   void themesInit();

   Ui::Widget        *ui;
   QProcess          process;
   QxtGlobalShortcut hotkey;
   QSystemTrayIcon   *trayIcon;
   QMenu             *trayMenu;
   QList<QAction*>   trayActions;
   //QPair<QIcon,QString>             themeChangeItems[3];
   QPair<QStringList,QStringList>   lngs;
   QPair<QStringList,QStringList>   themeNames;
   QString           fromLang,
                     toLang;
   int               lastFromListIndex,
                     lastToListIndex;

   Translate         *trans;
   Box               *box;
   GrabLineEdit      *lineEdit;
   Settings          *settings;
   Autorun           *autorun;



public slots:
   void startProcess();
   void errorProcess(QProcess::ProcessError err);
   void getSelected();
   void trayMenuSlot(QAction* a);
   void changeTheme(QString name);
   void setFromLanguage(QModelIndex index);
   void setToLanguage(QModelIndex index);
   void changeHotkey(QString key);
   void changeAutorun(bool status);
};

#endif // WIDGET_H
