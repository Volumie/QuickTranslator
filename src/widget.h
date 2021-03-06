#ifndef WIDGET_H
#define WIDGET_H

#include <QMessageBox>
#include <QDir>
#include <QWidget>
#include <QDesktopWidget>
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
#include <QProgressBar>
#include <QListWidgetItem>
#include <QDateTime>


#include "translate.h"
#include "box.h"
#include "textfield.h"
#include "smarttranslate.h"
#include "grablineedit.h"
#include "help.h"
#include "settings.h"
#include "autorun.h"
#include "crossplatform.h"
#include "defaulttranslator.h"
#include "update.h"


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
    void langListFlagsInit();
    void appLngChange(QString lng);
    void checkUpdates();
    void configUpdates(QVariant check, QDateTime time);
    void resizeEvent(QResizeEvent *ev);
    void moveEvent(QMoveEvent *ev);
    void timerEvent(QTimerEvent *ev);

    // Translate window types
    const static int TW_DEFAULT  = 0; // default
    const static int TW_CURSOR   = 1; // window at the cursor
    const static int TW_NOTIFIER = 2; // at the tray notifier


    int       translateWindowType;
    bool      similarWords;
    QString   fromLang,
              toLang,
              autoLang;
    QPair<QStringList,QStringList> lngs;



private:
   void trayMenuInit();
   void langListInit();
   void themesInit();

   void geometrySaveEvent();
   void appLanguageInit();
   void showNotifAppUseInt();

   Ui::Widget        *ui;
   QProcess          process;
   QProgressBar      *pb;
   QxtGlobalShortcut hotkey,
                     hotkeyField,
                     hotkeySmart;
   QSystemTrayIcon   *trayIcon;
   QMenu             *trayMenu;
   QList<QAction*>   trayActions;
   QPair<QStringList,QStringList> themeNames;
   QPair<QStringList,QStringList> flagNames;
   QPair<QList<QListWidgetItem*>, QList<QListWidgetItem*> > listWgtItms;

   QStringList       appLanguages;
   int               lastFromListIndex,
                     lastToListIndex;
   QString           lastAppLng;
   QString           appLngShort;

   QMessageBox       *aboutMB;

   Translate         *trans;
   Box               *box;
   TextField         *textfield;
   SmartTranslate    *smarttranslate;
   GrabLineEdit      *lineEdit;
   GrabLineEdit      *lineEditField;
   GrabLineEdit      *lineEditSmart;
   Help              *help;
   Settings          *settings;
   Autorun           *autorun;
   DefaultTranslator *defTrans;
   Update            *update;

   QBasicTimer       *gsTimer;          // GeometrySaveTimer
   int               gsTimerId;

   bool              smartMode;

   bool              notifUseInternet;

   bool              isCheckUpdates;
   QDateTime         nextUpdatesCheckTime;



signals:
   void nextTranslateDataSignal(QString translate, QString origin);



public slots:
   void startProcess();
   void startSmartTranslating();
   void finishSmartTranslating();
   void errorProcess(QProcess::ProcessError err);
   void getSelected();
   void trayMenuSlot(QAction* a);
   void changeTheme(QString name);
   void setFromLanguage(QModelIndex index);
   void setToLanguage(QModelIndex index);
   void changeHotkey(GrabLineEdit *gle, QString key);
   void changeHotkey(QString gle, QString key);
   void changeAutorun(bool status);
   void showTranslate(QString translate, QString origin, QString autoLang);
   void changeInfoType(int index);
   void translateText(QString str);
   void smartTranslateCount(int count);
   void applicationLanguageChange(int index);
   void translateToClipboard(bool val);
   void translateSimilarWords(bool val);
   void setFromLanguage(int i);
   void setToLanguage(int i);
   void languageReverse();
   void about();
   void changeCheckUpdates(bool val);
   void readNotifUseInternet(bool iRead);

};

#endif // WIDGET_H
