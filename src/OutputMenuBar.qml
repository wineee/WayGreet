// Copyright (C) 2025 rewine <luhongxu@deepin.org>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import WayGreet

Item {
    required property PrimaryOutput output

    width: output.width
    height: menuBar.contentHeight
    x: output.x
    y: output.y

    ToolBar {
        id: menuBar

        width: parent.width

        Row {
            anchors.fill: parent

            ToolButton {
                text: "Quit"
                onClicked: Qt.quit()
            }

            ToolButton {
                text: "Scale"
                onClicked: scaleMenu.popup()

                Menu {
                    id: scaleMenu

                    MenuItem {
                        text: "100%"
                        onClicked: {
                            output.setScale(1)
                        }
                    }

                    MenuItem {
                        text: "125%"
                        onClicked: {
                            output.setScale(1.25)
                        }
                    }

                    MenuItem {
                        text: "150%"
                        onClicked: {
                            output.setScale(1.5)
                        }
                    }

                    MenuItem {
                        text: "175%"
                        onClicked: {
                            output.setScale(1.75)
                        }
                    }

                    MenuItem {
                        text: "200%"
                        onClicked: {
                            output.setScale(2)
                        }
                    }
                }
            }

            ToolButton {
                text: "Rotation"

                onClicked: rotationMenu.popup()

                Menu {
                    id: rotationMenu

                    MenuItem {
                        text: "Normal"
                        onClicked: {
                            output.setTransform(WaylandOutput.Normal)
                        }
                    }

                    MenuItem {
                        text: "R90"
                        onClicked: {
                            output.setTransform(WaylandOutput.R90)
                        }
                    }

                    MenuItem {
                        text: "R270"
                        onClicked: {
                            output.setTransform(WaylandOutput.R270)
                        }
                    }
                }
            }

            Label {
                text: Helper.workspace.currentIndex
                color: "red"
            }

            ToolButton {
                text: "Output"

                onClicked: outputMenu.popup()

                Menu {
                    id: outputMenu

                    MenuItem {
                        text: "Add Output"
                        onClicked: {
                            Helper.addFakeOutput()
                        }
                    }

                    MenuItem {
                        text: (Helper.outputMode === Helper.OutputMode.Copy) ? "Extension Mode" : "Copy Mode"
                        onClicked: {
                            if (Helper.outputMode === Helper.OutputMode.Copy)
                                Helper.outputMode = Helper.OutputMode.Extension
                            else
                                Helper.outputMode = Helper.OutputMode.Copy
                        }
                    }
                }
            }
        }
    }
}
