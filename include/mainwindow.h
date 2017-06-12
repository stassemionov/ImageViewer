#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QSignalMapper>
#include <QMainWindow>
#include <QFileInfo>
#include <QSettings>
#include <QImage>

#include <labelviewer.h>
#include <history.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    enum class AppMode
    {
        Mode_View = 1,
        Mode_Editor = 2,
        Mode_Browser = 3
    };

//signals:

protected slots:
    void onOpenSettings();

    void onMouseEnterOnImage();
    void onMouseLeaveOnImage();
    void onMouseMoveOnImage(int xcoord, int ycoord);
    void onMousePressOnImage(Qt::MouseButton button, QPoint pos);
    void onClearRecentList();
    void onOpenRecentFile();
    void onSavePicture();
    void onShowInfo();
    void onChangeMode(int);
    void onPrintPicture();
    void onOpenImage();
    void onImageMoveSignal(QPoint pos);
    void onShowProperties();
    void onImageScaled(QWheelEvent *event);
    void onUndo();
    void onRedo();

    void onBrightnessInc();
    void onSaturationInc();
    void onRedInc();
    void onGreenInc();
    void onBlueInc();
    void onBrightnessDec();
    void onSaturationDec();
    void onRedDec();
    void onGreenDec();
    void onBlueDec();
    void onBrightnessDoubleInc();
    void onSaturationDoubleInc();
    void onRedDoubleInc();
    void onGreenDoubleInc();
    void onBlueDoubleInc();
    void onBrightnessDoubleDec();
    void onSaturationDoubleDec();
    void onRedDoubleDec();
    void onGreenDoubleDec();
    void onBlueDoubleDec();

    void onUncolourized();
    void onNegatived();

    void onRotateLeft();
    void onRotateRight();

protected:
    // *** service methods ***
    bool loadImage(const QString &file_name);
    void configureInfoWidget();
    void updateRecentList();
    void onRedEdited(int dif);
    void onGreenEdited(int dif);
    void onBlueEdited(int dif);
    void onSaturationEdited(int dif);
    void onBrightnessEdited(int dif);
    // Scaling of current image with size of app's window
    void updateScale();

    // *** events ***
    void closeEvent(QCloseEvent *pEvent);

private:
    Ui::MainWindow *ui;

    // Application mode ('Mode_View' by default).
    AppMode m_mode;
    // Image which is currently showing.
    QImage m_main_image;
    // Image that is currently on the screen.
    QImage m_showed_image;
    // Image that is intermediate result of editor's work.
    QImage m_intermediate_image;
    // Scale of current image on the screen.
    double m_scale = 1.0;
    double m_scale_step = 1.2;
    // Widget to show current image.
    LabelViewer m_screne_label;
    // List and widget for recent opened images.
    QMenu* m_recent_files_menu = nullptr;
    QStringList m_recent_files;
    // Menu for choose of application mode.
    QMenu* m_mode_menu = nullptr;
    // Context menues for each mode.
    QMenu* m_context_menu_viewer = nullptr;
    QMenu* m_context_menu_editor = nullptr;
    QMenu* m_context_menu_browser = nullptr;
    // Maximum count of stored recent images.
    const int m_max_recent_count = 10;
    // Name of currently opened image.
    QFileInfo m_current_image_fileinfo;
    // Title for mouse position viewing.
    QLabel m_pos_label;
    // Setting of application.
    QSettings settings;
    // Signal mapper to help mode changing.
    QSignalMapper* m_mode_mapper;
    // Directories of last open/save action.
    QString m_last_open_dir;
    QString m_last_save_dir;
    // Widget to show information about application.
    QWidget* m_info_widget = nullptr;
    // History of editor's transformations.
    EditHistory* m_edit_history = nullptr;
};

#endif // MAINWINDOW_H
