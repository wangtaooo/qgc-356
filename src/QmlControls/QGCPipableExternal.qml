/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


import QtQuick                      2.3
import QtQuick.Controls             1.2

import QGroundControl               1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.Controls      1.0
import QGroundControl.Palette       1.0

Item {
    id: pip

    property bool isHidden:  false
    property bool isDark:    false

    // As a percentage of the window width
    //作为窗口宽度的百分比
    property real maxSize: 0.75
    property real minSize: 0.10

    property bool inPopup: false
    property bool enablePopup: true

    signal  activated()
    signal  hideIt(bool state)//点击的信号
    signal  newWidth(real newWidth)
    signal  popup()

    MouseArea {
        id: pipMouseArea
        anchors.fill: parent
        enabled:      !isHidden
        hoverEnabled: true
        onClicked: {
            pip.activated()
        }
    }

    // MouseArea to drag in order to resize the PiP area
    //鼠标区域拖动以调整PiP区域的大小
    MouseArea {
        id: pipResize
        anchors.top: parent.top
        anchors.right: parent.right
        height: ScreenTools.minTouchPixels
        width: height
        property real initialX: 0
        property real initialWidth: 0

        onClicked: {
            // TODO propagate
        }

        // When we push the mouse button down, we un-anchor the mouse area to prevent a resizing loop
        //当我们按下鼠标按钮时，我们将取消鼠标区域的锚定，以防止重新调整大小的循环
        onPressed: {
            pipResize.anchors.top = undefined // Top doesn't seem to 'detach' 
            pipResize.anchors.right = undefined // This one works right, which is what we really need 这台运转正常，这才是我们真正需要的
            pipResize.initialX = mouse.x
            pipResize.initialWidth = pip.width
        }

        // When we let go of the mouse button, we re-anchor the mouse area in the correct position
        //当我们松开鼠标按钮时，我们将鼠标区域重新锚定在正确的位置
        onReleased: {
            pipResize.anchors.top = pip.top
            pipResize.anchors.right = pip.right
        }

        // Drag
        onPositionChanged: {
            if (pipResize.pressed) {
                var parentW = pip.parent.width // flightView
                var newW = pipResize.initialWidth + mouse.x - pipResize.initialX
                if (newW < parentW * maxSize && newW > parentW * minSize) {
                    newWidth(newW)
                }
            }
        }
    }

    // Resize icon
    //缩放窗口的图标
/*    Image {
        source:         "/qmlimages/pipResize.svg"
        fillMode:       Image.PreserveAspectFit
        mipmap: true
        anchors.right:  parent.right
        anchors.top:    parent.top
        visible:        !isHidden && (ScreenTools.isMobile || pipMouseArea.containsMouse) && !inPopup
        height:         ScreenTools.defaultFontPixelHeight * 2.5
        width:          ScreenTools.defaultFontPixelHeight * 2.5
        sourceSize.height:  height
    }
*/
    // Resize pip window if necessary when main window is resized
    property int pipLock: 2

    Connections {
        target: pip.parent
        onWidthChanged: {
            // hackity hack...
            // don't fire this while app is loading/initializing (it happens twice)
            if (pipLock) {
                pipLock--
                return
            }

            var parentW = pip.parent.width

            if (pip.width > parentW * maxSize) {
                newWidth(parentW * maxSize)
            } else if (pip.width < parentW * minSize) {
                newWidth(parentW * minSize)
            }
        }
    }

     //-- PIP Popup Indicator
     //PIP 弹出指标
 /*   Image {
        id:             popupPIP
        source:         "/qmlimages/PiP.svg"
        mipmap:         true
        fillMode:       Image.PreserveAspectFit
        anchors.left:   parent.left
        anchors.top:    parent.top
        visible:        !isHidden && !inPopup && !ScreenTools.isMobile && enablePopup && pipMouseArea.containsMouse
        height:         ScreenTools.defaultFontPixelHeight * 2.5
        width:          ScreenTools.defaultFontPixelHeight * 2.5
        sourceSize.height:  height
        MouseArea {
            anchors.fill: parent
            onClicked: {
                inPopup = true
                pip.popup()
            }
        }
    }
*/
    //-- PIP Corner Indicator
    //PIP 角落指示器
    Image {
        id:             closePIP
        source:         "/qmlimages/pipHide.svg"
        mipmap:         true
        fillMode:       Image.PreserveAspectFit
        anchors.left:   parent.left
        anchors.bottom: parent.bottom
        visible:        !isHidden && (ScreenTools.isMobile || pipMouseArea.containsMouse)
        height:         ScreenTools.defaultFontPixelHeight * 2.5
        width:          ScreenTools.defaultFontPixelHeight * 2.5
        sourceSize.height:  height
        MouseArea {
            anchors.fill: parent
            onClicked: {
                pip.hideIt(true)
            }
        }
    }

    //-- Show PIP
    //视频小框收起来的图标
    Rectangle {
        id:                     openPIP
        anchors.left :          parent.left
        anchors.bottom:         parent.bottom
        height:                 ScreenTools.defaultFontPixelHeight * 2
        width:                  ScreenTools.defaultFontPixelHeight * 2
//        radius:                 ScreenTools.defaultFontPixelHeight / 3
        visible:                isHidden
        color:                  isDark ? Qt.rgba(0,0,0,0.75) : Qt.rgba(0,0,0,0.5)
//        color:                  "red"

        //加载弹出视频框的图标
        Image {
            width:              parent.width  * 0.75
            height:             parent.height * 0.75
            sourceSize.height:  height
            source:             "/res/buttonRight.svg"
            mipmap:             true
            fillMode:           Image.PreserveAspectFit
            anchors.verticalCenter:     parent.verticalCenter
            anchors.horizontalCenter:   parent.horizontalCenter
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                pip.hideIt(false)
            }
        }
    }
}
