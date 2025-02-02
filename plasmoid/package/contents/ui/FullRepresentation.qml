import QtQuick 2.7
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import QtQml 2.2
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 2.0 as PlasmaComponents
import martchus.syncthingplasmoid 0.6 as SyncthingPlasmoid

ColumnLayout {
    id: root

    // define minimum size
    Layout.minimumWidth: units.gridUnit * 20
    Layout.minimumHeight: units.gridUnit * 15

    // ensure keyboard events can be received after initialization
    Component.onCompleted: forceActiveFocus()

    // define function to update the size according to the settings
    // when "floating" (shown as popup) or tied to an edge
    function updateSize() {
        switch (plasmoid.location) {
        case PlasmaCore.Types.Floating:
        case PlasmaCore.Types.TopEdge:
        case PlasmaCore.Types.BottomEdge:
        case PlasmaCore.Types.LeftEdge:
        case PlasmaCore.Types.RightEdge:
            var size = plasmoid.nativeInterface.size
            parent.width = units.gridUnit * size.width
            parent.height = units.gridUnit * size.height
            break
        default:
            ;
        }
    }

    // update the size when settings changed
    Connections {
        target: plasmoid.nativeInterface
        onSizeChanged: updateSize()
    }

    // define shortcuts to trigger actions for currently selected item
    function clickCurrentItemButton(buttonName) {
        mainTabGroup.currentTab.item.view.clickCurrentItemButton(buttonName)
    }
    Shortcut {
        sequence: "Ctrl+R"
        onActivated: clickCurrentItemButton("rescanButton")
    }
    Shortcut {
        sequence: "Ctrl+P"
        onActivated: clickCurrentItemButton("resumePauseButton")
    }
    Shortcut {
        sequence: "Ctrl+O"
        onActivated: clickCurrentItemButton("openButton")
    }

    // define custom key handling for switching tabs, selecting items and filtering
    Keys.onPressed: {
        // note: event only received after clicking the tab buttons in plasmoidviewer
        // but works as expected in plasmashell
        switch (event.key) {
        case Qt.Key_Up:
            switch (event.modifiers) {
            case Qt.NoModifier:
                // select previous item in current tab
                mainTabGroup.currentTab.item.view.decrementCurrentIndex()
                break
            case Qt.ShiftModifier:
                // select previous connection
                --plasmoid.nativeInterface.currentConnectionConfigIndex
                break
            }
            break
        case Qt.Key_Down:
            switch (event.modifiers) {
            case Qt.NoModifier:
                // select next item in current tab
                mainTabGroup.currentTab.item.view.incrementCurrentIndex()
                break
            case Qt.ShiftModifier:
                // select previous connection
                ++plasmoid.nativeInterface.currentConnectionConfigIndex
                break
            }
            break
        case Qt.Key_Left:
            // select previous tab
            switch (mainTabGroup.currentTab) {
            case dirsPage:
                downloadsTabButton.clicked()
                break
            case devicesPage:
                dirsTabButton.clicked()
                break
            case downloadsPage:
                devsTabButton.clicked()
                break
            }
            break
        case Qt.Key_Right:
            // select next tab
            switch (mainTabGroup.currentTab) {
            case dirsPage:
                devsTabButton.clicked()
                break
            case devicesPage:
                downloadsTabButton.clicked()
                break
            case downloadsPage:
                dirsTabButton.clicked()
                break
            }
            break
        case Qt.Key_Enter:

            // fallthrough
        case Qt.Key_Return:
            // toggle expanded state of current item
            var currentItem = mainTabGroup.currentTab.item.view.currentItem
            if (currentItem) {
                currentItem.expanded = !currentItem.expanded
            }
            break
        case Qt.Key_Escape:
            var filter = findCurrentFilter()
            if (filter && filter.text !== "") {
                // reset filter
                filter.explicitelyShown = false
                filter.text = ""
                event.accepted = true
            } else {
                // hide plasmoid
                plasmoid.expanded = false
            }
            break
        case Qt.Key_1:
            // select directories tab
            dirsTabButton.clicked()
            break
        case Qt.Key_2:
            // select devices tab
            devsTabButton.clicked()
            break
        case Qt.Key_3:
            // select downloads tab
            downloadsTabButton.clicked()
            break
        default:
            sendKeyEventToFilter(event)
            return
        }
        event.accepted = true
    }

    function findCurrentFilter() {
        return mainTabGroup.currentTab.item.filter
    }

    function sendKeyEventToFilter(event) {
        var filter = findCurrentFilter()
        if (!filter || event.text === "" || filter.activeFocus) {
            return
        }
        if (event.key === Qt.Key_Backspace && filter.text === "") {
            filter.explicitelyShown = false
            return
        }
        if (event.matches(StandardKey.Paste)) {
            filter.paste()
        } else {
            filter.text = ""
            filter.text += event.text
        }
        filter.forceActiveFocus()
    }

    // heading and right-corner buttons
    RowLayout {
        id: toolBar
        Layout.fillWidth: true

        TinyButton {
            id: connectButton

            states: [
                State {
                    name: "disconnected"
                    PropertyChanges {
                        target: connectButton
                        text: qsTr("Connect")
                        iconSource: "view-refresh"
                    }
                },
                State {
                    name: "paused"
                    PropertyChanges {
                        target: connectButton
                        text: qsTr("Resume")
                        iconSource: "media-playback-start"
                    }
                },
                State {
                    name: "idle"
                    PropertyChanges {
                        target: connectButton
                        text: qsTr("Pause")
                        iconSource: "media-playback-pause"
                    }
                }
            ]
            state: {
                switch (plasmoid.nativeInterface.connection.status) {
                case SyncthingPlasmoid.Data.Disconnected:
                case SyncthingPlasmoid.Data.Reconnecting:
                    return "disconnected"
                case SyncthingPlasmoid.Data.Paused:
                    return "paused"
                default:
                    return "idle"
                }
            }
            tooltip: text
            onClicked: {
                switch (plasmoid.nativeInterface.connection.status) {
                case SyncthingPlasmoid.Data.Disconnected:
                case SyncthingPlasmoid.Data.Reconnecting:
                    plasmoid.nativeInterface.connection.connect()
                    break
                case SyncthingPlasmoid.Data.Paused:
                    plasmoid.nativeInterface.connection.resumeAllDevs()
                    break
                default:
                    plasmoid.nativeInterface.connection.pauseAllDevs()
                    break
                }
            }
            Shortcut {
                sequence: "Ctrl+Shift+P"
                onActivated: connectButton.clicked()
            }
        }
        TinyButton {
            id: startStopButton

            states: [
                State {
                    name: "running"
                    PropertyChanges {
                        target: startStopButton
                        visible: true
                        text: qsTr("Stop")
                        tooltip: "systemctl --user stop "
                                 + plasmoid.nativeInterface.service.unitName
                        iconSource: "process-stop"
                    }
                },
                State {
                    name: "stopped"
                    PropertyChanges {
                        target: startStopButton
                        visible: true
                        text: qsTr("Start")
                        tooltip: "systemctl --user start "
                                 + plasmoid.nativeInterface.service.unitName
                        iconSource: "system-run"
                    }
                },
                State {
                    name: "irrelevant"
                    PropertyChanges {
                        target: startStopButton
                        visible: false
                    }
                }
            ]
            state: {
                var nativeInterface = plasmoid.nativeInterface
                // the systemd unit status is only relevant when connected to the local instance
                if (!nativeInterface.local
                        || !nativeInterface.startStopEnabled) {
                    return "irrelevant"
                }
                // show start/stop button only when the configured unit is available
                var service = nativeInterface.service
                if (!service || !service.systemdAvailable) {
                    return "irrelevant"
                }
                return service.running ? "running" : "stopped"
            }
            onClicked: plasmoid.nativeInterface.service.toggleRunning()
            style: TinyButtonStyle {
            }
            Shortcut {
                sequence: "Ctrl+Shift+S"
                onActivated: {
                    if (startStopButton.visible) {
                        startStopButton.clicked()
                    }
                }
            }
        }
        Item {
            Layout.fillWidth: true
        }
        TinyButton {
            id: showNewNotifications
            tooltip: qsTr("Show new notifications")
            iconSource: "emblem-warning"
            visible: plasmoid.nativeInterface.notificationsAvailable
            onClicked: {
                plasmoid.nativeInterface.showNotificationsDialog()
                plasmoid.expanded = false
            }
            Shortcut {
                sequence: "Ctrl+N"
                onActivated: {
                    if (showNewNotifications.visible) {
                        showNewNotifications.clicked()
                    }
                }
            }
        }
        TinyButton {
            tooltip: qsTr("About Syncthing Tray")
            iconSource: "help-about"
            onClicked: {
                plasmoid.nativeInterface.showAboutDialog()
                plasmoid.expanded = false
            }
        }
        TinyButton {
            id: showOwnIdButton
            tooltip: qsTr("Show own device ID")
            iconSource: "view-barcode"
            onClicked: {
                plasmoid.nativeInterface.showOwnDeviceId()
                plasmoid.expanded = false
            }
            Shortcut {
                sequence: "Ctrl+I"
                onActivated: showOwnIdButton.clicked()
            }
        }
        TinyButton {
            id: showLogButton
            tooltip: qsTr("Show Syncthing log")
            iconSource: "text-x-generic"
            onClicked: {
                plasmoid.nativeInterface.showLog()
                plasmoid.expanded = false
            }
            Shortcut {
                sequence: "Ctrl+L"
                onActivated: showLogButton.clicked()
            }
        }
        TinyButton {
            id: rescanAllDirsButton
            tooltip: qsTr("Rescan all directories")
            iconSource: "view-refresh"
            onClicked: plasmoid.nativeInterface.connection.rescanAllDirs()
            Shortcut {
                sequence: "Ctrl+Shift+R"
                onActivated: rescanAllDirsButton.clicked()
            }
        }
        TinyButton {
            id: settingsButton
            tooltip: qsTr("Settings")
            iconSource: "preferences-other"
            onClicked: {
                plasmoid.nativeInterface.showSettingsDlg()
                plasmoid.expanded = false
            }
            Shortcut {
                sequence: "Ctrl+S"
                onActivated: settingsButton.clicked()
            }
        }
        TinyButton {
            id: webUIButton
            tooltip: qsTr("Open Syncthing")
            iconSource: ":/icons/hicolor/scalable/status/syncthing-default.svg"
            onClicked: {
                plasmoid.nativeInterface.showWebUI()
                plasmoid.expanded = false
            }
            Shortcut {
                sequence: "Ctrl+W"
                onActivated: webUIButton.clicked()
            }
        }
        TinyButton {
            text: plasmoid.nativeInterface.currentConnectionConfigName
            iconSource: "network-connect"
            paddingEnabled: true
            // FIXME: figure out why menu doesn't work in plasmoidviewer using NVIDIA driver
            // (works with plasmawindowed and plasmashell or always when using Intel graphics)
            menu: Menu {
                id: connectionConfigsMenu

                ExclusiveGroup {
                    id: connectionConfigsExclusiveGroup
                }

                Instantiator {
                    model: plasmoid.nativeInterface.connectionConfigNames

                    MenuItem {
                        text: model.modelData
                        checkable: true
                        checked: plasmoid.nativeInterface.currentConnectionConfigIndex === index
                        exclusiveGroup: connectionConfigsExclusiveGroup
                        onTriggered: {
                            plasmoid.nativeInterface.currentConnectionConfigIndex = index
                        }
                    }
                    onObjectAdded: connectionConfigsMenu.insertItem(index,
                                                                    object)
                    onObjectRemoved: connectionConfigsMenu.removeItem(object)
                }
            }
        }
    }

    PlasmaCore.SvgItem {
        Layout.preferredWidth: parent.width
        Layout.preferredHeight: 2
        elementId: "horizontal-line"
        svg: PlasmaCore.Svg {
            imagePath: "widgets/line"
        }
    }

    // global statistics and traffic
    GridLayout {
        Layout.leftMargin: 5
        Layout.fillWidth: true
        Layout.fillHeight: false
        columns: 3
        rowSpacing: 1
        columnSpacing: 4

        PlasmaCore.IconItem {
            Layout.preferredWidth: 16
            Layout.preferredHeight: 16
            source: "globe"
        }
        StatisticsView {
            Layout.leftMargin: 4
            statistics: plasmoid.nativeInterface.globalStatistics
            context: qsTr("Global")
        }

        IconLabel {
            Layout.leftMargin: 10
            iconSource: plasmoid.nativeInterface.loadFontAwesomeIcon(
                            "cloud-download-alt")
            iconOpacity: plasmoid.nativeInterface.hasIncomingTraffic ? 1.0 : 0.5
            text: plasmoid.nativeInterface.incomingTraffic
            tooltip: qsTr("Global incoming traffic")
        }

        PlasmaCore.IconItem {
            Layout.preferredWidth: 16
            Layout.preferredHeight: 16
            source: "user-home-symbolic"
        }
        StatisticsView {
            Layout.leftMargin: 4
            statistics: plasmoid.nativeInterface.localStatistics
            context: qsTr("Local")
        }

        IconLabel {
            Layout.leftMargin: 10
            iconSource: plasmoid.nativeInterface.loadFontAwesomeIcon(
                            "cloud-upload-alt")
            iconOpacity: plasmoid.nativeInterface.hasOutgoingTraffic ? 1.0 : 0.5
            text: plasmoid.nativeInterface.outgoingTraffic
            tooltip: qsTr("Global outgoing traffic")
        }
    }

    PlasmaCore.SvgItem {
        Layout.preferredWidth: parent.width
        Layout.preferredHeight: 2
        elementId: "horizontal-line"
        svg: PlasmaCore.Svg {
            imagePath: "widgets/line"
        }
    }

    // tab "widget"
    RowLayout {
        spacing: 0

        ColumnLayout {
            spacing: 0
            Layout.alignment: Qt.AlignTop | Qt.AlignLeft

            PlasmaComponents.TabBar {
                id: tabBar
                tabPosition: Qt.LeftEdge
                Layout.alignment: Qt.AlignTop | Qt.AlignLeft

                PlasmaComponents.TabButton {
                    id: dirsTabButton
                    //text: qsTr("Directories")
                    iconSource: "folder-symbolic"
                    tab: dirsPage
                }
                PlasmaComponents.TabButton {
                    id: devsTabButton
                    //text: qsTr("Devices")
                    iconSource: "network-server-symbolic"
                    tab: devicesPage
                }
                PlasmaComponents.TabButton {
                    id: downloadsTabButton
                    //text: qsTr("Downloads")
                    iconSource: "folder-download-symbolic"
                    tab: downloadsPage
                }
            }
            Item {
                Layout.fillHeight: true
            }
            TinyButton {
                id: searchButton
                iconSource: "search"
                enabled: mainTabGroup.currentTab === dirsPage
                tooltip: qsTr("Toggle filter")
                onClicked: {
                    var filter = findCurrentFilter()
                    if (!filter) {
                        return
                    }
                    if (!filter.explicitelyShown) {
                        filter.explicitelyShown = true
                        filter.forceActiveFocus()
                    } else {
                        filter.explicitelyShown = false
                        filter.text = ""
                    }
                }
            }
        }

        PlasmaCore.SvgItem {
            Layout.preferredWidth: 2
            Layout.fillHeight: true
            elementId: "vertical-line"
            svg: PlasmaCore.Svg {
                imagePath: "widgets/line"
            }
        }
        PlasmaComponents.TabGroup {
            id: mainTabGroup
            currentTab: dirsPage
            Layout.fillWidth: true
            Layout.fillHeight: true

            PlasmaExtras.ConditionalLoader {
                id: dirsPage
                when: mainTabGroup.currentTab === dirsPage
                source: Qt.resolvedUrl("DirectoriesPage.qml")
            }
            PlasmaExtras.ConditionalLoader {
                id: devicesPage
                when: mainTabGroup.currentTab === devicesPage
                source: Qt.resolvedUrl("DevicesPage.qml")
            }
            PlasmaExtras.ConditionalLoader {
                id: downloadsPage
                when: mainTabGroup.currentTab === downloadsPage
                source: Qt.resolvedUrl("DownloadsPage.qml")
            }
        }
    }
}
