#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QGraphicsColorizeEffect>
#include <QGraphicsBlurEffect>


// Constructor
Widget::Widget(QMainWindow *parent) :
   QMainWindow(parent),
   ui(new Ui::Widget)
{
   settings = new Settings(this);
   settings->Load();
   settings->applicationLanguageChange();

   appLanguages << "en" << "ru" << "uk";
   translateWindowType = TW_DEFAULT;
   smartMode           = false;

   ui->setupUi(this);
   ui->options->hide();

   autorun  = new Autorun();
   box      = new Box(this);
   textfield= new TextField(this);
   trans    = new Translate(this);
   defTrans = new DefaultTranslator(ui->defTrans, ui->options, this, trans);
   update   = new Update(this);
   smarttranslate = new SmartTranslate();
   lineEdit = new GrabLineEdit(ui->hotkey_le);
   lineEditField = new GrabLineEdit(ui->hotkey_le_field);
   lineEditSmart = new GrabLineEdit(ui->hotkey_le_smart);
   help     = new Help();
   pb       = new QProgressBar();
   gsTimer  = new QBasicTimer();
   aboutMB  = NULL;
   setWindowFlags(Crossplatform::_WindowCloseButtonHint());

   // Init some objects
   trayMenuInit();
   langListFlagsInit();
   langListInit();
   themesInit();
   settings->Init();
   help->Init(appLngShort);

   // Delete default QLineEdit (used for fine design only)
   ui->footer->layout()->addWidget(lineEdit);
   delete ui->hotkey_le;
   ui->horizontalLayout_field->addWidget(lineEditField);
   delete ui->hotkey_le_field;
   ui->horizontalLayout_smart->addWidget(lineEditSmart);
   delete ui->hotkey_le_smart;

   autoLang          = "";

   // Unstick options window and move
   ui->MainWidget->layout()->removeWidget(ui->options);
   ui->options->setGeometry(this->width()- ui->options->width(), 0, ui->options->width(), this->height());


   if(notifUseInternet)
       showNotifAppUseInt();


   // Connects
   connect(&hotkey,              SIGNAL(activated()),                           SLOT(startProcess()));
   connect(&hotkeyField,         SIGNAL(activated()), textfield,                SLOT(show()));
   connect(&hotkeySmart,         SIGNAL(activated()),                           SLOT(startSmartTranslating()));
   connect(smarttranslate,       SIGNAL(getPart(QString)),                      SLOT(translateText(QString)));
   connect(smarttranslate,       SIGNAL(finish()),                              SLOT(finishSmartTranslating()));
   connect(this, SIGNAL(nextTranslateDataSignal(QString, QString)), smarttranslate, SLOT(sendTranslateText(QString, QString)));
   connect(smarttranslate,       SIGNAL(getPacketCount(int)),                   SLOT(smartTranslateCount(int)));
   connect(&process,             SIGNAL(error(QProcess::ProcessError)),         SLOT(errorProcess(QProcess::ProcessError)));
   connect(&process,             SIGNAL(readyReadStandardOutput()),             SLOT(getSelected()));
   connect(textfield,            SIGNAL(getText(QString)),                      SLOT(translateText(QString)));
   connect(trayMenu,             SIGNAL(triggered(QAction*)),                   SLOT(trayMenuSlot(QAction*)));
 //connect(trans,                SIGNAL(showTranslate(QString)), box,           SLOT(showTranslate(QString)));
   connect(trans,                SIGNAL(showTranslate(QString,QString,QString)),SLOT(showTranslate(QString, QString, QString)));
   connect(ui->theme_cb,         SIGNAL(activated(QString)),                    SLOT(changeTheme(QString)));
   connect(ui->from_list,        SIGNAL(doubleClicked(QModelIndex)),            SLOT(setFromLanguage(QModelIndex)));
   connect(ui->to_list,          SIGNAL(doubleClicked(QModelIndex)),            SLOT(setToLanguage(QModelIndex)));
   connect(lineEdit,             SIGNAL(hotkeyChanged(GrabLineEdit*, QString)), SLOT(changeHotkey(GrabLineEdit*, QString)));
   connect(lineEditField,        SIGNAL(hotkeyChanged(GrabLineEdit*, QString)), SLOT(changeHotkey(GrabLineEdit*, QString)));
   connect(lineEditSmart,        SIGNAL(hotkeyChanged(GrabLineEdit*, QString)), SLOT(changeHotkey(GrabLineEdit*, QString)));
   connect(ui->autorun_cb,       SIGNAL(toggled(bool)),                         SLOT(changeAutorun(bool)));
   connect(ui->cpToClipboard_cb, SIGNAL(toggled(bool)),                         SLOT(translateToClipboard(bool)));
   connect(ui->infoWin_ch,       SIGNAL(activated(int)),                        SLOT(changeInfoType(int)));
   connect(ui->appLanguage,      SIGNAL(activated(int)),                        SLOT(applicationLanguageChange(int)));
   connect(ui->similar_cb,       SIGNAL(toggled(bool)),                         SLOT(translateSimilarWords(bool)));
   connect(defTrans->btnHelp,    SIGNAL(clicked(bool)), help,                   SLOT(show()));
   connect(defTrans->btnAbout,   SIGNAL(clicked(bool)),                         SLOT(about()));
   connect(ui->checkUpdates_cb,  SIGNAL(toggled(bool)),                         SLOT(changeCheckUpdates(bool)));

}





// Destructor
Widget::~Widget(){
    // Some doing
    trayIcon->hide(); // Hide tray icon for Windows

    for(int i=0; i<trayActions.size(); ++i)
        delete trayActions[i];

    // Deleting
    delete ui;
    delete box;
    delete trans;
    delete settings;
    delete autorun;
    delete textfield;
    delete smarttranslate;
    delete lineEdit;
    delete lineEditField;
    delete lineEditSmart;
    delete pb;
    delete gsTimer;
    delete trayIcon;
    delete trayMenu;
    delete help;
    delete update;
}





// Start "get selected text" process
void Widget::startProcess(){
#ifdef Q_OS_WIN
    if(translateWindowType == TW_CURSOR) {
        box->setFly(true);
        box->show();
        box->hide();
    }
#endif
    process.start(Crossplatform::_GetSelectedProcessName());
}





// Start smart translating
void Widget::startSmartTranslating(){
    QRect screenRect = QApplication::desktop()->availableGeometry();
    pb->setMaximum(pb->minimum());
    pb->setWindowFlags(Crossplatform::_WindowOnTopFrameIconHide());
    pb->setFormat(tr("Translated %p% of data"));
    pb->setGeometry(0, 0, screenRect.width(), 16);
    pb->show();

    smartMode = true;
    trans->setSimilarWords(false);
    smarttranslate->startTranslating();
}





// Finish smart translating
void Widget::finishSmartTranslating(){
    smartMode = false;
    trans->setSimilarWords(similarWords);
    pb->setValue(pb->maximum());
    pb->hide();
}





// Packets count of smart translate
void Widget::smartTranslateCount(int count){
    pb->setMaximum(count);
    pb->setValue(0);
}





// Process error
void Widget::errorProcess(QProcess::ProcessError err){
    qDebug() << "Error run process \"xsel\"\nMessage: " << process.errorString();
    if(err == QProcess::NotOpen){
        QMessageBox::critical(this, tr("Critical error!"),
            tr("Unable to run the utility <b>xsel</b>, maybe it is not exists."
               "<br>The program work is not possible without it!<br>"
               "Try reinstalling the program to its normal work!"));
    }
}





// Get selected text (process return data) and Translate it
void Widget::getSelected(){
   QByteArray selStr = process.readAll();
   process.close();
   trans->setData(fromLang, toLang, selStr);
}





// Translate some text
void Widget::translateText(QString str){
    QString tmpStr = str;

    if(str.contains(QRegExp("&[a-zA-Z]+;"))){
        QTextDocument td;
        td.setHtml(str);
        tmpStr = td.toPlainText();
    }

    trans->setData(fromLang, toLang, tmpStr.toUtf8());
    pb->setValue(pb->value() + 1);
}





// Init tray menu
void Widget::trayMenuInit(){
    trayIcon = new QSystemTrayIcon();
    trayIcon->setIcon(QIcon(":/files/imgs/icon-color.svg"));
    trayIcon->show();

    trayActions.append(new QAction(tr("Show/Hide"), 0));
    trayActions.append(new QAction(tr("About"), 0));
    trayActions.append(new QAction(tr("Exit"), 0));

    trayMenu = new QMenu();
    trayMenu->addActions(trayActions);
    trayMenu->insertSeparator(trayActions.last());

    trayIcon->setContextMenu(trayMenu);
}





// Init languages list
void Widget::langListInit(){
    langListInit(":/langs/lngList.json");
}


// Init languages list
void Widget::langListInit(QString url, bool initList){
    QString workDir = QApplication::applicationDirPath() +"/imgs/flags/";
    QFile fl(url);
    fl.open(QFile::ReadOnly);
    QString str = QString(fl.readAll());
    fl.close();

    if(lngs.first.count() == 0){
        QVariantList vl = QxtJSON::parse(str).value<QVariantList>();
        foreach (QVariant v, vl) {
            QVariantMap vm = v.value<QVariantMap>();
            QString mini = vm["mini"].toString();
            QString full = vm["full"].toString();
            lngs.first.append(mini);
            lngs.second.append(QString::fromUtf8(full.toAscii()));
        }
    }
    if(initList){
        if(flagNames.first.size() > 0) {
            for(int i=0; i<lngs.first.size(); i++) {
                int index = flagNames.first.indexOf(lngs.first[i]);

                QIcon icon;

                if(index != -1)
                    icon = QIcon(workDir + flagNames.second[index] + ".png");
                else
                    icon = QIcon(workDir + "non.png");

                QListWidgetItem *item1 = new QListWidgetItem(icon, lngs.second[i]);
                QListWidgetItem *item2 = new QListWidgetItem(icon, lngs.second[i]);
                listWgtItms.first.append(item1);
                listWgtItms.second.append(item2);
                ui->from_list->addItem(item1);
                ui->to_list->addItem(item2);
            }
            defTrans->loadLanguages(listWgtItms.first);
            box->loadLanguages(listWgtItms.first);
        }
        else {
            ui->from_list->addItems(lngs.second);
            ui->to_list->addItems(lngs.second);
            defTrans->loadLanguages(lngs.second);
            box->loadLanguages(lngs.second);
        }
        delete ui->to_list->takeItem(0);
    }

    lastFromListIndex = 0;
    lastToListIndex   = 0;
}





// Init languages list's flags
// MUST INIT BEFORE "langListInit" !!!
void Widget::langListFlagsInit() {
    QString workDir = QApplication::applicationDirPath() +"/imgs/flags/";
    QString url = workDir + "To_ISO_3166-1.json";
    QFile fl(url);
    fl.open(QFile::ReadOnly);
    QString str = QString(fl.readAll());
    fl.close();

    if(flagNames.first.size() > 0) {
        flagNames.first.clear();
        flagNames.second.clear();
    }

    QVariantList vl = QxtJSON::parse(str).value<QVariantList>();
    foreach (QVariant v, vl) {
        QVariantMap vm = v.value<QVariantMap>();
        QString from = vm["from"].toString();
        QString to = vm["to"].toString();
        flagNames.first.append(from);
        flagNames.second.append(to);
    }

    for(int i=0; i<flagNames.first.size(); i++) {
        QFile icon(workDir + flagNames.second[i] + ".png");
        if( !icon.exists() ) {
            qDebug() << "File: " << icon.fileName() << "don't exists!";
            flagNames.second[i] = "non";
        }
    }

}





// Init theme list
void Widget::themesInit(){
    QString themesUrl = QCoreApplication::applicationDirPath() + "/themes/";
    QStringList list  = QDir(themesUrl).entryList(QStringList() << "*.qss");
    QString tmp;

    themeNames.first.clear();
    themeNames.second.clear();


    foreach (QString elem, list){
        int end = elem.indexOf("{");
            end = (end != -1) ? end : elem.indexOf(".qss");
            tmp = elem.mid(0, end);
        themeNames.first.append(tmp);
        if(end != elem.indexOf(".qss"))
            tmp = elem.mid(end+1, elem.indexOf("}")-end-1);
        else
            tmp = "color";
        themeNames.second.append(tmp);
    }
    if(!themeNames.first.contains("Default")){
        themeNames.first.insert(0, "Default");
        themeNames.second.insert(0, "color");
    }

    ui->theme_cb->addItems(themeNames.first);
}





// TrayMenu slot
void Widget::trayMenuSlot(QAction *act){
    if(act == trayActions[0]){                                         // Show/Hide
        if(isHidden())
           show();
        else
           hide();
    }

    if(act == trayActions[1]){                                         // About
        about();
    }
    if(act == trayActions[2]){                                         // Exit
        trayMenu->hide();
        qApp->quit();
    }

}





// Copy to clipboard slot
void Widget::translateToClipboard(bool val){
    ui->cpToClipboard_cb->setChecked(val);
    settings->Update(settings->TRANS_CLIPBOARD, val);
}





// Show translate slot
void Widget::showTranslate(QString translate, QString origin, QString autoLang){
    this->autoLang = autoLang;
    defTrans->setAutoLang(autoLang);
    if(!smartMode){
        switch(translateWindowType){
            case TW_DEFAULT:
                box->setFly(false);
                emit box->showTranslate(translate);
                break;
            case TW_NOTIFIER:
                box->setHidden(true);
                trayIcon->showMessage(tr("Translate"), translate, QSystemTrayIcon::NoIcon, 5000);
                break;
            case TW_CURSOR:
                box->setFly(true);
                QPoint wp = QCursor::pos() + QPoint(16, 16);
                box->move(wp);
                box->showTranslate(translate);
                break;
        }
        // Copy to clipboard if need
        if(ui->cpToClipboard_cb->isChecked()) {
            QApplication::clipboard()->setText(translate);
        }
    } else {
        emit nextTranslateDataSignal(translate, origin);
    }
}





// Set fromTranslate list item
void Widget::setFromLanguage(QModelIndex i){
    ui->from_list->item(lastFromListIndex)->setFont(QFont());
    ui->from_list->item(lastFromListIndex)->setBackground(QBrush());

    fromLang  = ui->from_list->item(i.row())->text();
    int index = lngs.second.indexOf(fromLang);
    fromLang  = lngs.first[index];

    ui->from_list->item(i.row())->setFont(QFont(NULL, -1, 700));
    ui->from_list->item(i.row())->setBackground(QBrush(QColor(128, 128, 128, 100)));

    lastFromListIndex = i.row();

    settings->Update(settings->LANG_FROM, fromLang);
    emit defTrans->fromLng->setCurrentIndex(index);
    emit box->setFromIndex(i);
}





// Set fromTranslate list item
void Widget::setFromLanguage(QString str){
    int index = lngs.first.indexOf(str);
    setFromLanguage(index);
}





// Set fromTranslate list item
void Widget::setFromLanguage(int index){
    index = (index < 0) ? 0 : index;

    QModelIndex mi = ui->from_list->indexAt(QPoint(0, 0));
    setFromLanguage(mi.model()->index(index, 0));
}





// Set toTranslate list item
void Widget::setToLanguage(QModelIndex i){
    ui->to_list->item(lastToListIndex)->setFont(QFont());
    ui->to_list->item(lastToListIndex)->setBackground(QBrush());

    toLang    = ui->to_list->item(i.row())->text();
    int index = lngs.second.indexOf(toLang);
    toLang    = lngs.first[index];

    ui->to_list->item(i.row())->setFont(QFont(NULL, -1, 700));
    ui->to_list->item(i.row())->setBackground(QBrush(QColor(128, 128, 128, 100)));

    lastToListIndex = i.row();

    settings->Update(settings->LANG_TO, toLang);
    emit defTrans->toLng->setCurrentIndex(index-1);
    emit box->setToIndex(i);
}





// Set toTranslate list item
void Widget::setToLanguage(QString str){
    int index = lngs.first.indexOf(str) - 1; // -1 beacouse toList don't have "Detect language" item
    setToLanguage(index);
}





// Set toTranslate list item
void Widget::setToLanguage(int index){
    index = (index < 0) ? 0 : index;

    QModelIndex mi= ui->to_list->indexAt(QPoint(0, 0));
    setToLanguage(mi.model()->index(index, 0));
}





// Change application theme
void Widget::changeTheme(QString thName){
    int index = themeNames.first.indexOf(thName);

    QString name = thName;
    if(index == -1) {
        index = 0;
        name  = "Default";
    }
    QString themeName = name;
    QString iName     = themeNames.second[index];
    QString themesUrl = QCoreApplication::applicationDirPath() + "/themes/";
    name += "{" + iName + "}.qss";

    ui->theme_cb->setCurrentIndex(index);
    trayIcon->setIcon(QIcon(":/files/imgs/icon-" + iName + ".svg"));
    QFile f(themesUrl + name);
    if(!f.exists())
        f.setFileName(themesUrl + themeName + ".qss");
    if(!f.exists())
        f.setFileName(":/files/themes/" + name);
    f.open(QIODevice::ReadOnly);
    qApp->setStyleSheet(f.readAll());
    f.close();

    defTrans->changeTheme(iName);

    settings->Update(settings->APP_THEME, themeName);

}





// Application languagre change
void Widget::applicationLanguageChange(int index) {
    if(ui->appLanguage->itemText(index) != lastAppLng){
        settings->Update(settings->APP_LANG, appLanguages[index]);
        if(0 == QMessageBox::question(this, tr("Please note!"),
                               tr("Change of language will only occur after restarting the application.\n"
                                  "Restart the application now?"), tr("Yes"), tr("No")))
        {
            QProcess::startDetached(QApplication::applicationFilePath(), QStringList("--restart"));
            qApp->quit();
        }
    }
}





// Application language change at start
void Widget::appLngChange(QString lng) {
    appLngShort = lng;
    int index   = appLanguages.indexOf(lng);
    ui->appLanguage->setCurrentIndex(index);
    lastAppLng  = ui->appLanguage->currentText();
}





// Change hotkey by GrabLineEdit
void Widget::changeHotkey(GrabLineEdit *gle, QString key){
    QxtGlobalShortcut *gsh;
    QString settingKey;

    if(gle == lineEdit) {
        gsh = &hotkey;
        settingKey = settings->HOTKEY_MAIN;
    }
    if(gle == lineEditField) {
        gsh = &hotkeyField;
        settingKey = settings->HOTKEY_FIELD;
    }
    if(gle == lineEditSmart) {
        gsh = &hotkeySmart;
        settingKey = settings->HOTKEY_SMART;
    }

    gsh->setDisabled();
    gle->setText(key);

    if(!gsh->setShortcut(QKeySequence(key))){
        gle->setText(gle->text()+" "+tr("Change hotkey!"));
        gle->setStyleSheet("QLineEdit{color: red}");
    } else {
        gsh->setEnabled();
        gle->setStyleSheet("QLineEdit{color: black}");
    }

    settings->Update(settingKey, key);
}





// Change hotkey by String
void Widget::changeHotkey(QString gle, QString key){
   if (gle == settings->HOTKEY_MAIN)  changeHotkey(lineEdit,      key);
   if (gle == settings->HOTKEY_FIELD) changeHotkey(lineEditField, key);
   if (gle == settings->HOTKEY_SMART) changeHotkey(lineEditSmart, key);
}





// Change application autorun status
void Widget::changeAutorun(bool status){
    if(autorun->setAutorun(status)){
        ui->autorun_cb->setChecked(status);
        settings->Update(settings->APP_AUTORUN, status);
    } else {
        qDebug() << "Autorun Error!";
    }
}





// Translate similar words change
void Widget::translateSimilarWords(bool val) {
    trans->setSimilarWords(val);
    ui->similar_cb->setChecked(val);
    settings->Update(settings->TRANS_SIMILAR, val);
    similarWords = val;
}





// Main application's window resize
void Widget::resizeEvent(QResizeEvent *ev){
    geometrySaveEvent();

    // Move options window to new coordinates
    ui->options->setGeometry(this->width()- ui->options->width(), 0, ui->options->width(), this->height());
}





// Main application's window move
void Widget::moveEvent(QMoveEvent *ev){
    geometrySaveEvent();
}





// Geometry save event
void Widget::geometrySaveEvent(){
    if(gsTimer->isActive()){
        gsTimer->stop();
    }
    gsTimer->start(1000, this);
    gsTimerId = gsTimer->timerId();
}





// Show Message Box notification about internet use of this app
void Widget::showNotifAppUseInt() {
    QMessageBox *ms = new QMessageBox(
                   tr("Please note"),
                   tr("This program uses the internet for translation"
                      " so you should have its presence on your computer, and, if necessary,"
                      " should be given permission to use the Internet of this program on your firewall."),
                   QMessageBox::Information,
                   QMessageBox::Ok,
                   QMessageBox::Cancel | QMessageBox::Escape,
                   QMessageBox::NoButton);
    ms->setDefaultButton(QMessageBox::Ok);
    int btn = ms->exec();
    if(btn == QMessageBox::Ok)
        readNotifUseInternet(false);
    delete ms;
}






// Timer events
void Widget::timerEvent(QTimerEvent *ev){
    if(ev->timerId() == gsTimerId) {        // geometry save timer
        settings->Update(settings->APP_GEOMETRY, geometry());
        gsTimer->stop();
    }
}





// Change info window type
void Widget::changeInfoType(int index){
    translateWindowType = index;
    ui->infoWin_ch->setCurrentIndex(index);
    settings->Update(settings->APP_INFOWINTYPE, index);
}





// Reverse translate languages
void Widget::languageReverse() {
    QString oldFrom = this->fromLang;
    QString oldTo   = this->toLang;

    if(oldFrom == "auto") {
        oldFrom = autoLang;
        if(oldFrom == "") {
            oldFrom = "en";
        }
    }


    this->setFromLanguage(oldTo);
    this->setToLanguage(oldFrom);
}






// Show about messagebox
void Widget::about() {
    if(aboutMB == NULL){
        aboutMB = new QMessageBox(tr("About"),
                 tr("<center><h2>Quick Translator</h2></center><br>"
                    "This is simple program designed to quickly translate "
                    "selected text from an foreign language to yours.<br>"
                    "Features:"
                    "<ul>"
                    "<li> quick translation of selected text, a combination of keys that can be changed;</li>"
                    "<li> quick translate your text, that you can write in special text field;</li>"
                    "<li> Smart translate copied text, that can translate for example,"
                    "completely the entire table or formatted text;</li>"
                    "<li> display similar words from the translated;</li>"
                    "<li> translate not only words but also phrases;</li>"
                    "<li> living translation;</li>"
                    "<li> quick reverse translation languages;</li>"
                    "<li> translation into 103 languages.</li>"
                    "</ul>"
                    "The program is absolutely free and allowed to free distribution.<br><br>"
                    "If you like the program, you can support the further development of the program"
                    "and future projects:<br><br>"
                    "Web  Money<p style=\"margin: 0; margin-left: 40px\">"
                    "<b>WMR:</b> R243421992717<br>"
                    "<b>WMU:</b> U275261423550<br>"
                    "<b>WMK:</b> K407115109469<br>"
                    "<b>WMZ:</b> Z100236886128<br></p>"
                    "Author: <a href='http://vk.com/rozshko'>Mihail Rozshko</a><br>"
                    "Email: <a href='mailto:mihail.rozshko@gmail.com'>mihail.rozshko@gmail.com</a><br>"
                    "Site: <a href='http://bimusoft.tk'>BimuSoft.tk</a><br><br>"
                    "Copyright &copy; BimuSoft 2014-2016"),
                    QMessageBox::Information,
                    QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton,
                    this);
        aboutMB->setIconPixmap(QPixmap(":/files/imgs/icon-color.svg"));
        Crossplatform::setFocus(aboutMB);
        int ret = aboutMB->exec();
        if(ret == QMessageBox::Ok) {
            delete aboutMB;
            aboutMB = NULL;
        }
    }
}





// Check for updates
void Widget::checkUpdates() {
    if(isCheckUpdates && QDateTime::currentDateTime() >= nextUpdatesCheckTime) {
        update->check();
    }
}





// Change check updates slot
void Widget::changeCheckUpdates(bool val) {
    configUpdates(QVariant(val), QDateTime());
}




// Change state that user read notification aboute internet used of this app
void Widget::readNotifUseInternet(bool iRead) {
    this->notifUseInternet = iRead;
    settings->Update(settings->NOTIF_INTERNETUSE, iRead);
}





// Configure updates setting
void Widget::configUpdates(QVariant check, QDateTime time) {
    if(check.isValid()) {
        settings->Update(settings->UPDATE_CHECK, check);
        isCheckUpdates = check.toBool();
        ui->checkUpdates_cb->setChecked(isCheckUpdates);
    }
    if(time.isValid()) {
        settings->Update(settings->UPDATE_NEXTTIME, time);
        nextUpdatesCheckTime = time;
    }
}
