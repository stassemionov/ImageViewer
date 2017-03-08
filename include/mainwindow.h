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
    void onImageScaled(bool direction);
    void onUndo();
    void onRedo();
    void onColourEdited();
    void onUncolourized();
    void onNegatived();

protected:
    // *** service methods ***
    bool loadImage(const QString &file_name);
    void configureInfoWidget();
    void updateRecentList();

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
    QImage* m_intermediate_image = nullptr;
    // Image that is buffer for colour edition.
    QImage* m_colour_buffer_image = nullptr;
    // Scale of current image on the screen.
    double m_scale = 1.0;
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
    // Action to save current image.
    QAction * m_save_action = nullptr;
    // Signal mapper to help mode changing.
    QSignalMapper* m_mode_mapper;
    // Directories of last open/save action.
    QString m_last_open_dir;
    QString m_last_save_dir;
    // Widget to show information about application.
    QWidget* m_info_widget = nullptr;
    // History of editor's transformations.
    EditHistory* m_edit_history = nullptr;
    // Flag that is meaning uncolorized state of image.
    bool m_uncolourized = false;
    // Flag that is meaning inverted state of image.
    bool m_negatived = false;
};

#endif // MAINWINDOW_H
