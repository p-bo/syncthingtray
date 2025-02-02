import QtQuick 2.0
import org.kde.plasma.plasmoid 2.0
import org.kde.kquickcontrolsaddons 2.0

Item {
    id: syncthingApplet

    Plasmoid.switchWidth: units.gridUnit * 25
    Plasmoid.switchHeight: units.gridUnit * 20

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation
    Plasmoid.compactRepresentation: CompactRepresentation {
    }
    Plasmoid.fullRepresentation: Loader {
        source: "FullRepresentation.qml"
        onLoaded: item.updateSize()
    }

    Plasmoid.icon: "syncthingtray"
    Plasmoid.toolTipMainText: plasmoid.nativeInterface.statusText
    Plasmoid.toolTipSubText: plasmoid.nativeInterface.additionalStatusText
    Plasmoid.toolTipItem: Loader {
        source: "ToolTipView.qml"
    }

    Plasmoid.hideOnWindowDeactivate: true

    function action_showWebUI() {
        plasmoid.nativeInterface.showWebUI()
    }

    function action_showSettings() {
        plasmoid.nativeInterface.showSettingsDlg()
    }

    function action_rescanAllDirs() {
        plasmoid.nativeInterface.connection.rescanAllDirs()
    }

    function action_restartSyncthing() {
        plasmoid.nativeInterface.connection.restart()
    }

    function action_showLog() {
        plasmoid.nativeInterface.showLog()
    }

    function action_showErrors() {
        plasmoid.nativeInterface.showInternalErrorsDialog()
    }

    function action_showAboutDialog() {
        plasmoid.nativeInterface.showAboutDialog()
    }

    Component.onCompleted: {
        plasmoid.removeAction("configure")
        plasmoid.setAction("showWebUI", qsTr("Web UI"), "internet-web-browser")
        plasmoid.setAction("showSettings", qsTr("Settings"), "configure")
        plasmoid.setAction("showLog", qsTr("Log"), "text-x-generic")
        plasmoid.setAction(
                    "showErrors", qsTr("Internal errors"),
                    ":/icons/hicolor/scalable/emblems/emblem-important-old.svg")
        plasmoid.setAction("rescanAllDirs", qsTr("Rescan all directories"),
                           "folder-sync")
        plasmoid.setAction("restartSyncthing", qsTr("Restart Syncthing"),
                           "system-reboot")
        plasmoid.setAction("showAboutDialog", qsTr("About"), "help-about")
    }
}
