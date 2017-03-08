#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_properties_dialog.h"

#include <QDesktopWidget>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QScrollBar>
#include <QDateTime>
#include <QPixmap>
#include <QAction>

#include <service.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_mode = AppMode::Mode_View;
    this->statusBar()->addPermanentWidget(&m_pos_label);
    this->statusBar()->hide();
    ui->editor_scroll_area->hide();
    ui->mainToolBar->hide();

    //ui->scrollArea->setWidget(&m_screne_label);
    ui->scroll_area_layout->setAlignment(Qt::AlignCenter);
    ui->scroll_area_layout->addWidget(&m_screne_label);

    connect(&m_screne_label, SIGNAL(mouseEnterSignal()),
            this, SLOT(onMouseEnterOnImage()));
    connect(&m_screne_label, SIGNAL(mouseLeaveSignal()),
            this, SLOT(onMouseLeaveOnImage()));
    connect(&m_screne_label, SIGNAL(mouseMoveSignal(int,int)),
            this, SLOT(onMouseMoveOnImage(int, int)));
    connect(&m_screne_label, SIGNAL(mousePressSignal(Qt::MouseButton, QPoint)),
            this, SLOT(onMousePressOnImage(Qt::MouseButton, QPoint)));
    connect(&m_screne_label, SIGNAL(imageMoveSignal(QPoint)),
             this, SLOT(onImageMoveSignal(QPoint)));
    connect(&m_screne_label, SIGNAL(imageScaledSignal(bool)),
            this, SLOT(onImageScaled(bool)));

    QSettings settings{QCoreApplication::organizationName(),
                       QString::fromLatin1("ImageViewer")};
    if (settings.contains(QString::fromLatin1("geometry")))
    {
        restoreGeometry(settings.value(
                        QString::fromLatin1("geometry")).toByteArray());
    }
    else
    {
        this->setGeometry(QApplication::desktop()->width()  / 4,
                          QApplication::desktop()->height() / 4,
                          QApplication::desktop()->width()  / 2,
                          QApplication::desktop()->height() / 2);
    }
    m_recent_files  = settings.value(QString::fromUtf8("recent")).toStringList();
    m_last_open_dir = settings.value(QString::fromUtf8("last_open_dir")).toString();
    m_last_save_dir = settings.value(QString::fromUtf8("last_save_dir")).toString();

    QMenu* menu_file = menuBar()->addMenu(tr("File"));
    m_mode_menu = menuBar()->addMenu(tr("Mode"));
    QMenu* menu_info = menuBar()->addMenu(tr("Info"));

    QAction* open_action = menu_file->addAction(tr("Open..."),
                                this, SLOT(onOpenImage()),
                                QKeySequence(Qt::CTRL + Qt::Key_O));
    m_recent_files_menu = menu_file->addMenu(tr("Recent files..."));
    menu_file->addSeparator();
    m_save_action = menu_file->addAction(tr("Save..."),
                         this, SLOT(onSavePicture()),
                         QKeySequence(Qt::CTRL + Qt::Key_S));
    menu_file->addSeparator();
    menu_file->addAction(tr("Exit"),
                         this, SLOT(close()),
                         QKeySequence(Qt::CTRL + Qt::Key_Q));
//    menu_edit->addAction(tr("Входные данные..."),
//                         this, SLOT(editInputData()),
//                         QKeySequence(Qt::CTRL + Qt::Key_I));
//    menu_edit->addAction(QString::fromUtf8("Шаг сетки..."),
//                         this, SLOT(specifyGridStep()),
//                         QKeySequence(Qt::CTRL + Qt::Key_G));
//    menu_edit->addSeparator();
//    menu_edit->addAction(QString::fromUtf8("Очистить график"),
//                         this, SLOT(clearData()));
    QAction* view_mode_enable_action = m_mode_menu->addAction(tr("View mode"));
    QAction* edit_mode_enable_action = m_mode_menu->addAction(tr("Edit mode"));
    QAction* browse_mode_enable_action = m_mode_menu->addAction(tr("Browse mode"));
    view_mode_enable_action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_1));
    edit_mode_enable_action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_2));
    browse_mode_enable_action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_3));

    m_mode_mapper = new QSignalMapper(this);
    connect(m_mode_mapper, SIGNAL(mapped(int)),
            this,          SLOT(onChangeMode(int)));
    connect(view_mode_enable_action, SIGNAL(triggered(bool)),
            m_mode_mapper, SLOT(map()));
    connect(edit_mode_enable_action, SIGNAL(triggered(bool)),
            m_mode_mapper, SLOT(map()));
    connect(browse_mode_enable_action, SIGNAL(triggered(bool)),
            m_mode_mapper, SLOT(map()));
    m_mode_mapper->setMapping(view_mode_enable_action, (int)AppMode::Mode_View);
    m_mode_mapper->setMapping(edit_mode_enable_action, (int)AppMode::Mode_Editor);
    m_mode_mapper->setMapping(browse_mode_enable_action, (int)AppMode::Mode_Browser);

    m_mode_menu->actions()[0]->setChecked(true);
    m_mode_menu->actions()[1]->setChecked(false);
    m_mode_menu->actions()[2]->setChecked(false);
    menu_info->addAction(tr("About"), this, SLOT(onShowInfo()));

    m_recent_files_menu->actions().reserve(m_max_recent_count + 2);
    for (int i = 0; i < m_max_recent_count; ++i)
    {
        QAction* p_action = new QAction(m_recent_files_menu);
        connect(p_action, SIGNAL(triggered()), this, SLOT(onOpenRecentFile()));
        m_recent_files_menu->addAction(p_action);
    }
    QAction* clear_action = new QAction(
                tr("Clear recent list"), m_recent_files_menu);
    connect(clear_action, SIGNAL(triggered()), this, SLOT(onClearRecentList()));
    m_recent_files_menu->addSeparator();
    m_recent_files_menu->addAction(clear_action);
    this->updateRecentList();

    // *** Context menues *** //

    m_context_menu_viewer =  new QMenu(this);
    m_context_menu_editor =  new QMenu(this);
    m_context_menu_browser = new QMenu(this);

    m_context_menu_viewer->addAction(m_save_action);
    QAction* print_action = m_context_menu_viewer->addAction(
                tr("Print..."),
                this,
                SLOT(onPrintPicture()),
                QKeySequence(Qt::CTRL + Qt::Key_P));
    QAction* properties_action = m_context_menu_viewer->addAction(
                tr("Properties"),
                this,
                SLOT(onShowProperties()));

    QToolButton* open_file_button  = new QToolButton();
    QToolButton* undo_button       = new QToolButton();
    QToolButton* redo_button       = new QToolButton();
    QToolButton* save_button       = new QToolButton();
    QToolButton* print_button      = new QToolButton();
    QToolButton* properties_button = new QToolButton();
    open_file_button->setIcon(
        QIcon(QString::fromUtf8(":/open_icon.png")));
    undo_button->setIcon(
        QIcon(QString::fromUtf8(":/undo_icon.png")));
    redo_button->setIcon(
        QIcon(QString::fromUtf8(":/redo_icon.png")));
    save_button->setIcon(
        QIcon(QString::fromUtf8(":/save_icon.png")));
    print_button->setIcon(
        QIcon(QString::fromUtf8(":/printer_icon.png")));
    properties_button->setIcon(
        QIcon(QString::fromUtf8(":/properties_icon.png")));

    connect(open_file_button, SIGNAL(clicked(bool)), open_action,      SLOT(trigger()));
    connect(undo_button,      SIGNAL(clicked(bool)), this,             SLOT(onUndo()));
    connect(redo_button,      SIGNAL(clicked(bool)), this,             SLOT(onRedo()));
    connect(save_button,      SIGNAL(clicked(bool)), m_save_action,    SLOT(trigger()));
    connect(print_button,     SIGNAL(clicked(bool)), print_action,     SLOT(trigger()));
    connect(properties_button,SIGNAL(clicked(bool)), properties_action,SLOT(trigger()));

    open_file_button->setToolTip(tr("Open image"));
    undo_button->setToolTip(tr("Cancel recent action"));
    redo_button->setToolTip(tr("Try recent action again"));
    save_button->setToolTip(tr("Save current image"));
    print_button->setToolTip(tr("Print current image"));
    properties_button->setToolTip(tr("Properties of image"));
    ui->mainToolBar->addWidget(open_file_button);
    ui->mainToolBar->addWidget(undo_button);
    ui->mainToolBar->addWidget(redo_button);
    ui->mainToolBar->addWidget(save_button);
    ui->mainToolBar->addWidget(print_button);
    ui->mainToolBar->addWidget(properties_button);

    m_edit_history = new EditHistory(QString::fromUtf8(""), 15);
    connect(ui->edit_brightness_slider, SIGNAL(valueChanged(int)),
            this, SLOT(onColourEdited()));
    connect(ui->edit_saturation_slider, SIGNAL(valueChanged(int)),
            this, SLOT(onColourEdited()));
    connect(ui->edit_red_slider, SIGNAL(valueChanged(int)),
            this, SLOT(onColourEdited()));
    connect(ui->edit_green_slider, SIGNAL(valueChanged(int)),
            this, SLOT(onColourEdited()));
    connect(ui->edit_blue_slider, SIGNAL(valueChanged(int)),
            this, SLOT(onColourEdited()));
    connect(ui->edit_negative_button, SIGNAL(clicked(bool)),
            this, SLOT(onNegatived()));
    connect(ui->edit_colorize_button, SIGNAL(clicked(bool)),
            this, SLOT(onUncolourized()));

    this->configureInfoWidget();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_info_widget;
    delete m_mode_mapper;
    delete m_edit_history;
 //   if (m_intermediate_image != nullptr)
 //   {
 //       delete m_intermediate_image;
 //   }
}

void MainWindow::updateRecentList()
{
    QMutableStringListIterator it{m_recent_files};
    while (it.hasNext())
    {
        if (!QFile::exists(it.next()))
        {
            it.remove();
        }
    }

    QList<QAction*> actions = m_recent_files_menu->actions();
    for (int i = 0; i < actions.size() - 2; ++i)
    {
        if (i < m_recent_files.size())
        {
            actions[i]->setText(m_recent_files[i]);
            actions[i]->setVisible(true);
        }
        else
        {
            actions[i]->setVisible(false);
        }
    }

    actions.back()->setVisible(!m_recent_files.empty());
}

void MainWindow::onClearRecentList()
{
    m_recent_files.clear();
    this->updateRecentList();
}

void MainWindow::onOpenImage()
{
    //    BMP	Windows Bitmap
    //    GIF	Graphic Interchange Format (optional)
    //    JPG	Joint Photographic Experts Group
    //    JPEG	Joint Photographic Experts Group
    //    MNG	Multiple-image Network Graphics
    //    PNG	Portable Network Graphics
    //    PBM	Portable Bitmap
    //    PGM	Portable Graymap
    //    PPM	Portable Pixmap
    //    TIFF	Tagged Image File Format
    //    XBM	X11 Bitmap
    //    XPM	X11 Pixmap
    //    SVG	Scalable Vector Graphics
    //    TGA   Targa Image Format

    QDir open_dir(m_last_open_dir);
    if (!open_dir.exists())
    {
        m_last_open_dir.clear();
    }

    QString file_name = QFileDialog::getOpenFileName(
                this, tr("Open image"), m_last_open_dir,
                tr("Images (") + QString::fromUtf8(
                "*.bmp *.gif *.jpg *.jpeg *.mng *.png *.pbm "
                "*.pgm *.ppm *.tiff *.xbm *.xpm *.svg *.tga)"));
    if (this->loadImage(file_name))
    {
        m_last_open_dir = m_current_image_fileinfo.path();
    }
}

void MainWindow::onOpenRecentFile()
{
    QString name = dynamic_cast<QAction*>(sender())->text();
    loadImage(name);
}

bool MainWindow::loadImage(const QString& file_name)
{
    if (m_main_image.load(file_name))
    {
        m_showed_image = m_main_image;
        if (m_intermediate_image != nullptr)
        {
            delete m_intermediate_image;
        }
        if (m_colour_buffer_image != nullptr)
        {
            delete m_colour_buffer_image;
        }
        m_intermediate_image = new QImage(m_main_image);
        m_colour_buffer_image = new QImage(m_main_image);
        m_screne_label.setPixmap(QPixmap::fromImage(m_showed_image));

        m_uncolourized = false;
        m_negatived = false;

        m_current_image_fileinfo.setFile(file_name);

        QFileInfo finfo(file_name);
        m_edit_history->clean();
        m_edit_history->setFileName(finfo.fileName());
        m_edit_history->add(*m_intermediate_image);

        int index = -1;
        for (int i = 0; i < m_recent_files.size(); ++i)
        {
            if (m_recent_files[i] == file_name)
            {
                index = i;
                break;
            }
        }

        if (index == -1)
        {
            m_recent_files.push_front(file_name);

            QAction* p_action = new QAction(m_recent_files_menu);
            connect(p_action, SIGNAL(triggered()), this, SLOT(onOpenRecentFile()));

            if (!m_recent_files_menu->actions().empty())
                m_recent_files_menu->insertAction(
                    m_recent_files_menu->actions()[0], p_action);
            else
                m_recent_files_menu->addAction(p_action);
        }
        else if (index != 0)
        {
            m_recent_files.move(index, 0);
            m_recent_files_menu->actions().move(index, 0);
        }

        this->updateRecentList();
        return true;
    }
    return false;
}

void MainWindow::onSavePicture()
{
    if (m_main_image.isNull())
    {
        return;
    }

    QDir save_dir(m_last_save_dir);
    if (!save_dir.exists())
    {
        m_last_save_dir.clear();
    }

    const QString dir = m_last_save_dir.isEmpty() ?
                QDir::currentPath() : m_last_save_dir;
    QString fileName = QFileDialog::getSaveFileName(
                this,
                tr("Save image"),
                dir + QString::fromUtf8("/") + m_current_image_fileinfo.fileName(),
                tr("Images (") + QString::fromUtf8(
                "*.bmp *.gif *.jpg *.jpeg *.mng *.png *.pbm "
                "*.pgm *.ppm *.tiff *.xbm *.xpm *.svg *.tga)"));
    if (fileName.isEmpty())
    {
        return;
    }
    if (m_intermediate_image->save(fileName, 0, 100))
    {
        QFileInfo finfo(fileName);
        m_last_save_dir = finfo.path();
    }
}

void MainWindow::onShowInfo()
{
    m_info_widget->show();
}

void MainWindow::onChangeMode(int mode)
{
    if ((int)m_mode == mode)
    {
        return;
    }

    m_mode_menu->actions()[0]->setChecked(false);
    m_mode_menu->actions()[1]->setChecked(false);
    m_mode_menu->actions()[2]->setChecked(false);
    m_mode_menu->actions()[mode - 1]->setChecked(true);

    if (mode == 1)
    {
        m_mode = AppMode::Mode_View;
        this->statusBar()->setHidden(true);
        ui->scrollArea->setLineWidth(0);
        ui->scrollArea->show();
        ui->editor_scroll_area->hide();
        ui->mainToolBar->hide();

    }
    else if (mode == 2)
    {
        m_mode = AppMode::Mode_Editor;
        this->statusBar()->setHidden(false);
        ui->scrollArea->setLineWidth(2);
        ui->scrollArea->show();
        ui->editor_scroll_area->show();
        ui->mainToolBar->show();

    }
    else // AppMode::Mode_Browser
    {
        m_mode = AppMode::Mode_Browser;
        this->statusBar()->setHidden(false);
        ui->scrollArea->hide();
        ui->editor_scroll_area->hide();
        ui->mainToolBar->hide();
    }
}

void MainWindow::closeEvent(QCloseEvent *pEvent)
{
    pEvent->accept();
    QSettings settings{QCoreApplication::organizationName(),
                QString::fromLatin1(("ImageViewer"))};
    settings.setValue(QString::fromUtf8("geometry"), saveGeometry());
    settings.setValue(QString::fromUtf8("recent"), m_recent_files);
    settings.setValue(QString::fromUtf8("last_open_dir"), m_last_open_dir);
    settings.setValue(QString::fromUtf8("last_save_dir"), m_last_save_dir);
    m_edit_history->clean();
}

void MainWindow::onMouseEnterOnImage()
{
    if (m_mode == AppMode::Mode_Editor)
    {
        QApplication::setOverrideCursor(Qt::PointingHandCursor);
        m_pos_label.show();
    }
}

void MainWindow::onMouseLeaveOnImage()
{
    if (m_mode == AppMode::Mode_Editor)
    {
        QApplication::restoreOverrideCursor();
        m_pos_label.hide();
    }
}

void MainWindow::onMouseMoveOnImage(int xcoord, int ycoord)
{
    if (m_mode == AppMode::Mode_Editor)
    {
        m_pos_label.setText(
            QString::fromUtf8("X = ")   + QString::number(xcoord) +
            QString::fromUtf8(", Y = ") + QString::number(ycoord));
    }
}

void MainWindow::onMousePressOnImage(Qt::MouseButton button, QPoint pos)
{
    if (m_mode == AppMode::Mode_View)
    {
        if (button == Qt::RightButton)
        {
            m_context_menu_viewer->exec(pos);
        }
    }
    else if (m_mode == AppMode::Mode_Editor)
    {
        if (button == Qt::RightButton)
        {
            m_context_menu_editor->exec(pos);
        }
    }
}

void MainWindow::onImageMoveSignal(QPoint pos)
{
   ui->scrollArea->verticalScrollBar()->setValue(
        qMax(0, ui->scrollArea->verticalScrollBar()->value() - pos.y()));
   ui->scrollArea->horizontalScrollBar()->setValue(
        qMax(0, ui->scrollArea->horizontalScrollBar()->value() - pos.x()));
}

void MainWindow::onImageScaled(bool direction)
{
    m_scale += direction ? 0.1 : -0.1;
    if ((m_scale * m_intermediate_image->width()  <
         QApplication::desktop()->width()  / 8) ||
        (m_scale * m_intermediate_image->height() <
         QApplication::desktop()->height() / 4))
    {
        m_scale -= direction ? 0.1 : -0.1;
    }

    m_showed_image = m_intermediate_image->scaledToHeight(
        m_intermediate_image->height() * m_scale);
    m_screne_label.setPixmap(QPixmap::fromImage(m_showed_image));

    m_pos_label.setText(QString::fromUtf8(""));
}

void MainWindow::onPrintPicture()
{

}

void MainWindow::onShowProperties()
{
    if (m_main_image.isNull())
    {
        return;
    }

    Ui::ImagePrpertiesDialog* dialog = new Ui::ImagePrpertiesDialog;
    QDialog* dial = new QDialog(this);
    dialog->setupUi(dial);

    dialog->prop_date_lbl->setText(
        m_current_image_fileinfo.created().
            toString(QString::fromUtf8("d MMMM yyyy hh:mm:ss")));
    dialog->prop_format_lbl->setText(
        m_current_image_fileinfo.completeSuffix().toUpper());
    dialog->prop_heigth_lbl->setText(
        QString::number(m_main_image.height()) + tr(" pixels"));
    dialog->prop_width_lbl->setText(
        QString::number(m_main_image.width()) + tr(" pixels"));
    dialog->prop_name_lbl->setText(
        m_current_image_fileinfo.completeBaseName());
    dialog->prop_value_lbl->setText(
        QString::number((double)m_current_image_fileinfo.size() / 1024,
            char('f'), 1) + tr(" Kb"));

    QImage sc_image = m_main_image.scaledToWidth(100);
    if (sc_image.height() > 160)
    {
        sc_image = sc_image.scaledToHeight(160);
    }
    dialog->prop_img_lbl->setPixmap(QPixmap::fromImage(sc_image));

    dial->exec();
}

void MainWindow::onUndo()
{
    QImage* prev_image = m_edit_history->back();
    if (prev_image == nullptr)
    {
        return;
    }
    *m_intermediate_image = *prev_image;
    m_showed_image = *prev_image;
    m_screne_label.setPixmap(QPixmap::fromImage(m_showed_image));
}

void MainWindow::onRedo()
{
    QImage* prev_image = m_edit_history->forward();
    if (prev_image == nullptr)
    {
        return;
    }
    *m_intermediate_image = *prev_image;
    m_showed_image = *prev_image;
    m_screne_label.setPixmap(QPixmap::fromImage(m_showed_image));
}

void MainWindow::configureInfoWidget()
{
    const int w = 670;
    const int h = 350;
    QString info_path = (QLocale::system().language() == QLocale::Russian) ?
                QString::fromLatin1(":/info_rus.txt") :
                QString::fromLatin1(":/info_international.txt");

    m_info_widget = new QWidget;
    m_info_widget->setWindowTitle(QString::fromUtf8("ImageViewer"));
    m_info_widget->setFixedSize(w, h);
    m_info_widget->setGeometry(
                QApplication::desktop()->width()  / 2 - w / 2,
                QApplication::desktop()->height() / 2 - h / 2,
                w, h );

    QVBoxLayout* vlayout = new QVBoxLayout;
    QHBoxLayout* hlayout = new QHBoxLayout;
    QHBoxLayout* hlayout2 = new QHBoxLayout;
    vlayout->addLayout(hlayout);
    vlayout->addLayout(hlayout2);
    hlayout2->setAlignment(Qt::AlignRight);
    hlayout->setAlignment(Qt::AlignLeft);
    m_info_widget->setLayout(vlayout);
    QLabel* l = new QLabel;
    l->setPixmap(QPixmap(
        QString::fromUtf8(":/main_icon.png")).scaled(h/2, h/2));
    l->setFixedSize(h/2, h/2);
    hlayout->addWidget(l);
    QLabel* textlabel = new QLabel(loadTextFileData(info_path));
    textlabel->setLayout(new QHBoxLayout);
    textlabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    textlabel->setFrameShape(QFrame::NoFrame);
    textlabel->setWordWrap(true);
    hlayout->addWidget(textlabel);
    QPushButton* button = new QPushButton(tr("Close"));
    button->setMaximumWidth(w/6);
    connect(button, SIGNAL(clicked(bool)), m_info_widget, SLOT(close()));
    hlayout2->addWidget(button);
}
