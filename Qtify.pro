QT+= widgets qml quick
CONFIG += console

TARGET = Qtify

include(libQtSpotify/libQtSpotify.pri)

SOURCES += \
    main.cpp

OTHER_FILES += \
    qml/main.qml

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    qml/Login.qml

OTHER_FILES += \
    qml/MainUI.qml

OTHER_FILES += \
    qml/Footer.qml

OTHER_FILES += \
    qml/Header.qml

OTHER_FILES += \
    qml/Center.qml

OTHER_FILES += \
    qml/Playlists.qml

OTHER_FILES += \
    qml/PlaylistView.qml

OTHER_FILES += \
    qml/CustomButton.qml

OTHER_FILES += \
    qml/CustomSliderDelegate.qml

OTHER_FILES += \
    qml/NowPlaying.qml

OTHER_FILES += \
    qml/SpotifyImage.qml

OTHER_FILES += \
    qml/BusyIndicator.qml

OTHER_FILES += \
    qml/TrackListView.qml

OTHER_FILES += \
    qml/SearchResults.qml

OTHER_FILES += \
    qml/VerticalScrollBarDelegate.qml

OTHER_FILES += \
    qml/HorizontalScrollBarDelegate.qml

MOC_DIR = ./.moc
OBJECTS_DIR = ./.obj
UI_DIR = ./.ui
RCC_DIR = ./.rcc


macx {
    LIBSPOTIFY.files = $$PWD/libspotify/lib/libspotify.framework
    LIBSPOTIFY.path = Contents/Frameworks
    QMAKE_BUNDLE_DATA += LIBSPOTIFY

    ICON = qtify_icon.icns
} else:win32 {

} else {
    QMAKE_LFLAGS += -Wl,-rpath,$$PWD/libspotify/lib
}
