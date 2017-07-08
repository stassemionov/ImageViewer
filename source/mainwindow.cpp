#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_properties_dialog.h"

#include <QStringListModel>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QMessageBox>
#include <QScrollBar>
#include <QDateTime>
#include <QPixmap>

#include <service.h>

#include <QDebug>

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
    ui->edit_rotate_dial->setEnabled(false);

    m_screne_label.setAlignment(Qt::AlignCenter);
    m_screne_label.setScaledContents(true);

    ui->viewer_scroll_area->setAlignment(Qt::AlignCenter);
    ui->viewer_scroll_area->setWidgetResizable(false);
    ui->viewer_scroll_area->setWidget(&m_screne_label);

    // *** SIGNALS AND SLOTS CONNECTION *** //
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
    connect(&m_screne_label, SIGNAL(imageScaledSignal(QWheelEvent*)),
            this, SLOT(onImageScaled(QWheelEvent*)));

    // *** SETTINGS LOADING *** //
    QSettings settings{QCoreApplication::organizationName(),
                       QString::fromUtf8("ImageViewer")};
    if (settings.contains(QString::fromUtf8("geometry")))
    {
        restoreGeometry(settings.value(
                        QString::fromUtf8("geometry")).toByteArray());
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

    // *** MAIN MENUES ADDING *** //
    QMenu* menu_file = menuBar()->addMenu(tr("File"));
    m_mode_menu = menuBar()->addMenu(tr("Mode"));
    QMenu* menu_tools = menuBar()->addMenu(tr("Tools"));
    QMenu* menu_info = menuBar()->addMenu(tr("Info"));

    m_open_action = menu_file->addAction(tr("Open..."),
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
    menu_tools->addAction(tr("Settings"), this, SLOT(onOpenSettings()));

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

    // *** CONTEXT MENUES ADDING *** //
    m_context_menu_viewer  = new QMenu(this);
    m_context_menu_editor  = new QMenu(this);
    m_context_menu_browser = new QMenu(this);

    m_context_menu_viewer->addAction(m_save_action);
    m_print_action = m_context_menu_viewer->addAction(
                tr("Print..."),
                this,
                SLOT(onPrintPicture()),
                QKeySequence(Qt::CTRL + Qt::Key_P));
    m_properties_action = m_context_menu_viewer->addAction(
                tr("Properties"),
                this,
                SLOT(onShowProperties()));

    // *** TOOLBAR BUTTONS ADDING *** //
    QToolButton* open_file_button  = new QToolButton();
    QToolButton* undo_button       = new QToolButton();
    QToolButton* redo_button       = new QToolButton();
    QToolButton* save_button       = new QToolButton();
    QToolButton* print_button      = new QToolButton();
    QToolButton* properties_button = new QToolButton();

    m_undo_action = new QAction();
    m_redo_action = new QAction();
    m_undo_action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Z));
    m_redo_action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Y));
    m_undo_action->setIcon(QIcon(QString::fromUtf8(":/undo_icon.png")));
    m_redo_action->setIcon(QIcon(QString::fromUtf8(":/redo_icon.png")));
    m_open_action->setIcon(QIcon(QString::fromUtf8(":/open_icon.png")));
    m_save_action->setIcon(QIcon(QString::fromUtf8(":/save_icon.png")));
    m_print_action->setIcon(QIcon(QString::fromUtf8(":/printer_icon.png")));
    m_properties_action->setIcon(QIcon(QString::fromUtf8(":/properties_icon.png")));
    m_undo_action->setToolTip(tr("Cancel recent action"));
    m_redo_action->setToolTip(tr("Try recent action again"));
    m_open_action->setToolTip(tr("Open image"));
    m_save_action->setToolTip(tr("Save current image"));
    m_print_action->setToolTip(tr("Print current image"));
    m_properties_action->setToolTip(tr("Properties of image"));
    connect(m_undo_action, SIGNAL(triggered(bool)), this, SLOT(onUndo()));
    connect(m_redo_action, SIGNAL(triggered(bool)), this, SLOT(onRedo()));
    undo_button->setDefaultAction(m_undo_action);
    redo_button->setDefaultAction(m_redo_action);
    open_file_button->setDefaultAction(m_open_action);
    save_button->setDefaultAction(m_save_action);
    print_button->setDefaultAction(m_print_action);
    properties_button->setDefaultAction(m_properties_action);

    m_history_combobox = new QComboBox;
    m_history_stringlist_model = new QStringListModel(
                m_history_stringlist);
    m_history_combobox->setModel(m_history_stringlist_model);
    m_history_combobox->setMinimumWidth(150);
    connect(m_history_combobox, SIGNAL(activated(int)),
            this, SLOT(onHistoryJump(int)));

    ui->mainToolBar->addWidget(open_file_button);
    ui->mainToolBar->addWidget(save_button);
    ui->mainToolBar->addWidget(undo_button);
    ui->mainToolBar->addWidget(redo_button);
    ui->mainToolBar->addWidget(m_history_combobox);
    ui->mainToolBar->addWidget(print_button);
    ui->mainToolBar->addWidget(properties_button);

    m_print_action->setDisabled(true);
    m_properties_action->setDisabled(true);

    // *** EDIT HISTORY CREATION *** //
    m_edit_history = new EditHistory(QString::fromUtf8(""),
                                     m_max_stored_records);

    // *** EDITOR WIDGETS SIGNALS AND SLOTS CONNECTION *** //
    connect(ui->edit_left_rot_button, SIGNAL(clicked(bool)),
            this, SLOT(onRotateLeft()));
    connect(ui->edit_right_rot_button, SIGNAL(clicked(bool)),
            this, SLOT(onRotateRight()));
    connect(ui->edit_original_angle_button, SIGNAL(clicked(bool)),
            this, SLOT(onRestoreOriginalAngle()));
    connect(ui->edit_rotate_dial, SIGNAL(valueChanged(int)),
            this, SLOT(onRotate(int)));
    connect(ui->edit_accuracy_low_radioButton, SIGNAL(toggled(bool)),
            this, SLOT(onChangeRotationMode(bool)));

    connect(ui->edit_brightness_min_button, SIGNAL(clicked(bool)),
            this, SLOT(onBrightnessDec()));
    connect(ui->edit_brightness_mmin_button, SIGNAL(clicked(bool)),
            this, SLOT(onBrightnessDoubleDec()));
    connect(ui->edit_brightness_plus_button, SIGNAL(clicked(bool)),
            this, SLOT(onBrightnessInc()));
    connect(ui->edit_brightness_pplus_button, SIGNAL(clicked(bool)),
            this, SLOT(onBrightnessDoubleInc()));

    connect(ui->edit_saturation_min_button, SIGNAL(clicked(bool)),
            this, SLOT(onSaturationDec()));
    connect(ui->edit_saturation_mmin_button, SIGNAL(clicked(bool)),
            this, SLOT(onSaturationDoubleDec()));
    connect(ui->edit_saturation_plus_button, SIGNAL(clicked(bool)),
            this, SLOT(onSaturationInc()));
    connect(ui->edit_saturation_pplus_button, SIGNAL(clicked(bool)),
            this, SLOT(onSaturationDoubleInc()));

    connect(ui->edit_red_min_button, SIGNAL(clicked(bool)),
            this, SLOT(onRedDec()));
    connect(ui->edit_red_mmin_button, SIGNAL(clicked(bool)),
            this, SLOT(onRedDoubleDec()));
    connect(ui->edit_red_plus_button, SIGNAL(clicked(bool)),
            this, SLOT(onRedInc()));
    connect(ui->edit_red_pplus_button, SIGNAL(clicked(bool)),
            this, SLOT(onRedDoubleInc()));

    connect(ui->edit_green_min_button, SIGNAL(clicked(bool)),
            this, SLOT(onGreenDec()));
    connect(ui->edit_green_mmin_button, SIGNAL(clicked(bool)),
            this, SLOT(onGreenDoubleDec()));
    connect(ui->edit_green_plus_button, SIGNAL(clicked(bool)),
            this, SLOT(onGreenInc()));
    connect(ui->edit_green_pplus_button, SIGNAL(clicked(bool)),
            this, SLOT(onGreenDoubleInc()));

    connect(ui->edit_blue_min_button, SIGNAL(clicked(bool)),
            this, SLOT(onBlueDec()));
    connect(ui->edit_blue_mmin_button, SIGNAL(clicked(bool)),
            this, SLOT(onBlueDoubleDec()));
    connect(ui->edit_blue_plus_button, SIGNAL(clicked(bool)),
            this, SLOT(onBlueInc()));
    connect(ui->edit_blue_pplus_button, SIGNAL(clicked(bool)),
            this, SLOT(onBlueDoubleInc()));

    connect(ui->edit_negative_button, SIGNAL(clicked(bool)),
            this, SLOT(onNegatived()));
    connect(ui->edit_colorize_button, SIGNAL(clicked(bool)),
            this, SLOT(onUncolourized()));
    connect(ui->edit_blur_button, SIGNAL(clicked(bool)),
            this, SLOT(onLinearSmoothing()));
    connect(ui->edit_gauss_filter_button, SIGNAL(clicked(bool)),
            this, SLOT(onGaussFilterAppying()));
    connect(ui->edit_custom_filter_button, SIGNAL(clicked(bool)),
            this, SLOT(onCustomizeFilter()));
    connect(ui->edit_clarity_button, SIGNAL(clicked(bool)),
            this, SLOT(onClarityIncreasing()));

    // Other interface connects.
    connect(ui->edit_clarity_slider, SIGNAL(valueChanged(int)),
            ui->edit_clarity_degree_label, SLOT(setNum(int)));

    // Matrix filter customizer signals configuration.
    m_filter_customizer.reset(new FilterCustomizer);
    int w = m_filter_customizer->width();
    int h = m_filter_customizer->height();
    m_filter_customizer->setGeometry(
                QApplication::desktop()->width() / 2 - w / 2,
                QApplication::desktop()->height() / 2 - h / 2,
                w, h);
    connect(&(*m_filter_customizer), SIGNAL(matrixUpdated()),
            this, SLOT(onCustomFilterApplied()));
    connect(ui->edit_custom_filter_apply_button, SIGNAL(clicked(bool)),
            this, SLOT(onCustomFilterApplied()));

    // *** CONFIGURATION OF INFORMATION WINDOW *** //
    this->configureInfoWidget();

    this->setSavedStatus(true);
    this->updateUndoRedoStatus();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_info_widget;
    delete m_mode_mapper;
    delete m_edit_history;
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
    this->loadImage(name);
}

bool MainWindow::loadImage(const QString& file_name)
{
    if (!m_main_image.load(file_name))
    {
        return false;
    }

    m_current_image_fileinfo.setFile(file_name);

    QFileInfo finfo(file_name);
    m_edit_history->clean();
    m_edit_history->setFileName(finfo.fileName());
    m_edit_history->add(m_main_image);
    m_history_stringlist.clear();
    this->writeActionName(tr("Original"));

    // searching for name of loaded file in list of recent files.
    int index = -1;
    for (int i = 0; i < m_recent_files.size(); ++i)
    {
        if (m_recent_files[i] == file_name)
        {
            index = i;
            break;
        }
    }
    // if no then push this name to hte beginning of recent files list.
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
    else if (index != 0)    // else, move this name to the beginning of this list.
    {
        m_recent_files.move(index, 0);
        m_recent_files_menu->actions().move(index, 0);
    }
    this->updateRecentList();

    // Update histogram.
    ui->edit_hist_label->setPixmap(QPixmap());

    m_intermediate_image = QSharedPointer<QImage>(
                new QImage(m_main_image.copy()));
    m_showed_image = QSharedPointer<QImage>(
                new QImage(m_main_image.copy()));
    m_screne_label.setPixmap(QPixmap::fromImage(*m_showed_image));

    m_pos_label.setText(QString::fromUtf8(""));
    ui->edit_rotate_dial->setEnabled(true);
    if (ui->edit_accuracy_low_radioButton->isChecked())
    {
        ui->edit_rotate_dial->setValue(180);
    }
    else
    {
        m_angle = 0.0;
        // generates rotation.
        ui->edit_accuracy_low_radioButton->setChecked(true);
    }

    m_print_action->setEnabled(true);
    m_properties_action->setEnabled(true);
    ui->edit_custom_filter_apply_button->setDisabled(true);
    this->updateScale();
    this->setSavedStatus(true);
    this->updateUndoRedoStatus();

    return true;
}

void MainWindow::updateView()
{
    if (m_showed_image.isNull())
    {
        return;
    }

    this->onRotate(ui->edit_rotate_dial->value());
    this->updateScale();
}

bool MainWindow::onSavePicture()
{
    if (m_intermediate_image->isNull())
    {
        return false;
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
        return false;
    }
    // Rotation before saving to get actual view.
    QTransform transform;
    QImage image_to_save = m_intermediate_image->
                transformed(transform.rotate(m_angle));
    if (image_to_save.save(fileName, 0, 100))
    {
        QFileInfo finfo(fileName);
        m_last_save_dir = finfo.path();
        this->setSavedStatus(true);
        return true;
    }
    return false;
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
        ui->viewer_scroll_area->setLineWidth(0);
        ui->viewer_scroll_area->show();
        ui->editor_scroll_area->hide();
        ui->mainToolBar->hide();
        this->updateView();
    }
    else if (mode == 2)
    {
        m_mode = AppMode::Mode_Editor;
        this->statusBar()->setHidden(false);
        ui->viewer_scroll_area->setLineWidth(2);
        ui->viewer_scroll_area->show();
        ui->editor_scroll_area->show();
        ui->mainToolBar->show();
        this->updateView();
    }
    else // AppMode::Mode_Browser
    {
        m_mode = AppMode::Mode_Browser;
        this->statusBar()->setHidden(false);
        ui->viewer_scroll_area->hide();
        ui->editor_scroll_area->hide();
        ui->mainToolBar->hide();
    }
}

void MainWindow::closeEvent(QCloseEvent *pEvent)
{
    if (!m_is_saved)
    {
        QMessageBox msgBox;
        msgBox.setIconPixmap(
            QPixmap(QString::fromUtf8(":/wonna_save.png")).scaledToHeight(100));
        msgBox.setText(tr("You made some changes in the image"));
        msgBox.setInformativeText(tr("Do you want to save your changes?"));
        msgBox.setDetailedText(tr("There will be something interesting soon..."));
        msgBox.setStandardButtons(
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();
        switch (ret)
        {
            case QMessageBox::Save:
                if (!this->onSavePicture())
                {
                    pEvent->ignore();
                    return;
                }
                break;
            case QMessageBox::Cancel:
                pEvent->ignore();
                return;
        }
    }

    pEvent->accept();
    QSettings settings{QCoreApplication::organizationName(),
                QString::fromUtf8(("ImageViewer"))};
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
            QString::fromUtf8("X = %0, Y = %1").
                    arg((int) (xcoord / m_scale)).
                    arg((int) (ycoord / m_scale)));
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
   ui->viewer_scroll_area->verticalScrollBar()->setValue(
        qMax(0, ui->viewer_scroll_area->verticalScrollBar()->value() - pos.y()));
   ui->viewer_scroll_area->horizontalScrollBar()->setValue(
        qMax(0, ui->viewer_scroll_area->horizontalScrollBar()->value() - pos.x()));
}

void MainWindow::updateScale()
{
    if (m_showed_image.isNull())
    {
        return;
    }

    double x_scale = 1.0 * ui->viewer_scroll_area->viewport()->width() /
            m_showed_image->width();
    double y_scale = 1.0 * ui->viewer_scroll_area->viewport()->height() /
            m_showed_image->height();
    // If showed image has side that is longer then corresponding size of viewport,
    // then image is adjusted to viewport with saving of proportions of image.
    m_scale = (x_scale < 1.0 || y_scale < 1.0) ? qMin(x_scale, y_scale) : 1.0;
    m_screne_label.resize(m_scale * m_showed_image->size());
}

void MainWindow::onImageScaled(QWheelEvent* event)
{
    const bool direction = event->angleDelta().y() > 0;
    const double ratio = (direction ? m_scale_step : (1.0 / m_scale_step)) - 1.0;   // new_scale / old_scale

    m_scale = direction ? (m_scale * m_scale_step) : (m_scale / m_scale_step);
    int new_h = m_scale * m_showed_image->height();
    int new_w = m_scale * m_showed_image->width();
    if ((new_w < ui->viewer_scroll_area->width() / 16)  ||
        (new_h < ui->viewer_scroll_area->height() / 16) ||
        (new_w > ui->viewer_scroll_area->width() * 8)  ||
        (new_h > ui->viewer_scroll_area->height() * 8))
    {
        m_scale = direction ? (m_scale / m_scale_step) :
                              (m_scale * m_scale_step);
        return;
    }

    int new_vvalue = ui->viewer_scroll_area->verticalScrollBar()->value()   + ratio * event->y();
    int new_hvalue = ui->viewer_scroll_area->horizontalScrollBar()->value() + ratio * event->x();

    m_screne_label.resize(m_showed_image->size() * m_scale);

    ui->viewer_scroll_area->horizontalScrollBar()->setValue(new_hvalue > 0 ? new_hvalue : 0);
    ui->viewer_scroll_area->verticalScrollBar()->setValue(new_vvalue > 0 ? new_vvalue : 0);

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

void MainWindow::onOpenSettings()
{

}

void MainWindow::configureInfoWidget()
{
    const int w = 670;
    const int h = 350;
    QString info_path = (QLocale::system().language() == QLocale::Russian) ?
                QString::fromUtf8(":/info_rus.txt") :
                QString::fromUtf8(":/info_international.txt");

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

void MainWindow::setSavedStatus(bool is_saved)
{
    m_is_saved = is_saved;
    m_save_action->setEnabled(!is_saved);
}

void MainWindow::updateUndoRedoStatus()
{
    if (m_edit_history->getHistoryLenght() < 2)
    {
        m_undo_action->setDisabled(true);
        m_redo_action->setDisabled(true);
    }
    else
    {
        m_undo_action->setDisabled(m_edit_history->isAtStart());
        m_redo_action->setDisabled(m_edit_history->isAtEnd());
    }
    m_history_combobox->setEnabled(
        m_edit_history->getHistoryLenght() > 0);
    m_history_combobox->setCurrentIndex(
        m_edit_history->getCurrentIndex());

//    qDebug() << "Total     :" << m_edit_history->m_total_stored_count;
//    qDebug() << "SHift     :" << m_edit_history->m_shift;
//    qDebug() << "List size :" << m_edit_history->m_list.size();
//    qDebug() << "Pointer   :" << m_edit_history->m_local_pointer;
//    qDebug() << "================";
}

void MainWindow::onChangeRotationMode(bool isLow)
{
    if (isLow)  // new mode is low accuracy
    {
        m_bisectr_angle = 0.0;
        m_angle = (int) m_angle;
        ui->edit_rotate_dial->setValue(
            (m_angle < 180) ? (m_angle + 180): (m_angle - 180));
    }
    else    // new mode is high accuracy
    {
        m_bisectr_angle = m_angle;
        ui->edit_rotate_dial->setValue(180);
    }
}

void MainWindow::writeActionName(const QString& act_name)
{
    // Current version, stored in history, is new version.
    int curr_version_index = m_edit_history->getCurrentIndex();
    // We need to remove all newer versons,
    // including version with the same index.
    if (curr_version_index < m_edit_history->getHistoryLenght())
    {
        m_history_stringlist.erase(
            m_history_stringlist.begin() + curr_version_index,
            m_history_stringlist.end());
    }
    m_history_stringlist.append(
        QString::fromUtf8("%0. %1").
        arg(curr_version_index + 1).
        arg(act_name));
    m_history_stringlist_model->setStringList(m_history_stringlist);
    m_history_combobox->setCurrentIndex(curr_version_index);
}

void MainWindow::onCustomizeFilter()
{
    if (m_intermediate_image.isNull())
    {
        return;
    }
    m_filter_customizer->show();
}
