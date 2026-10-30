import QtQuick 2.15

import ".."

// Blender-style navigation gizmo: one bubble per world axis direction.
// Click a bubble to look along that axis, drag to orbit,
// click the home button to restore the default camera.
Item {
    id: viewGizmo
    property var renderer: null
    // optional override, called with the desired camera forward direction
    property var snapCamera: null

    // emitted when the home button is clicked
    signal homeRequested()

    width: 116
    height: 116
    z: 10

    readonly property real gizmoRadius: 50
    readonly property real axisLength: 34
    readonly property real bubbleRadius: 10
    readonly property real cx: width - gizmoRadius - 2
    readonly property real cy: height - gizmoRadius - 2

    property vector3d camForward: Qt.vector3d(0, 0, -1)
    property vector3d camRight: Qt.vector3d(1, 0, 0)
    property vector3d camUp: Qt.vector3d(0, 1, 0)

    property int hoverAxis: -1
    property bool hoverGizmo: false

    // index: 0 = +X, 1 = +Y, 2 = +Z, 3 = -X, 4 = -Y, 5 = -Z
    readonly property var axisColors: ["#e63d3d", "#54b83d", "#3d6bf2"]
    readonly property var axisLabels: ["X", "Y", "Z"]

    function axisDir(i) {
        var s = i < 3 ? 1 : -1
        switch (i % 3) {
        case 0: return Qt.vector3d(s, 0, 0)
        case 1: return Qt.vector3d(0, s, 0)
        default: return Qt.vector3d(0, 0, s)
        }
    }

    function vdot(a, b) { return a.x * b.x + a.y * b.y + a.z * b.z }
    function vcross(a, b) {
        return Qt.vector3d(a.y * b.z - a.z * b.y,
                           a.z * b.x - a.x * b.z,
                           a.x * b.y - a.y * b.x)
    }
    function vlen(a) { return Math.sqrt(vdot(a, a)) }
    function vscale(a, k) { return Qt.vector3d(a.x * k, a.y * k, a.z * k) }
    function vadd(a, b) { return Qt.vector3d(a.x + b.x, a.y + b.y, a.z + b.z) }
    function vsub(a, b) { return Qt.vector3d(a.x - b.x, a.y - b.y, a.z - b.z) }
    function vnorm(a) {
        var l = vlen(a)
        if (l <= 0.00001)
            return Qt.vector3d(0, 0, -1)
        return vscale(a, 1.0 / l)
    }

    function updateBasis() {
        if (!renderer)
            return
        var f = vnorm(vsub(renderer.cameraTarget, renderer.cameraPosition))
        var r = vcross(f, Qt.vector3d(0, 1, 0))
        // same fallback as the renderer camera when looking straight up/down
        if (vlen(r) < 0.0001)
            r = vcross(f, Qt.vector3d(0, 0, -1))
        r = vnorm(r)
        camForward = f
        camRight = r
        camUp = vnorm(vcross(r, f))
    }

    // screen position and depth (positive = toward the viewer) of each bubble
    function projectedAxes() {
        var list = []
        for (var i = 0; i < 6; ++i) {
            var v = axisDir(i)
            list.push({
                index: i,
                x: cx + vdot(v, camRight) * axisLength,
                y: cy - vdot(v, camUp) * axisLength,
                depth: -vdot(v, camForward)
            })
        }
        return list
    }

    function axisAt(x, y) {
        var axes = projectedAxes()
        axes.sort(function(a, b) { return b.depth - a.depth })
        for (var i = 0; i < axes.length; ++i) {
            var dx = x - axes[i].x
            var dy = y - axes[i].y
            if (dx * dx + dy * dy <= bubbleRadius * bubbleRadius)
                return axes[i].index
        }
        return -1
    }

    function insideGizmo(x, y) {
        var dx = x - cx
        var dy = y - cy
        return dx * dx + dy * dy <= gizmoRadius * gizmoRadius
    }

    function cameraDistance() {
        if (!renderer)
            return 5.0
        return Math.max(vlen(vsub(renderer.cameraTarget, renderer.cameraPosition)), 0.5)
    }

    /* ---------------- camera animation ---------------- */

    property vector3d animFromTarget
    property vector3d animToTarget
    property vector3d animFromDir
    property vector3d animToDir
    property real animFromDist: 1
    property real animToDist: 1
    property real animT: 0

    function slerpDir(a, b, t) {
        var d = Math.max(-1, Math.min(1, vdot(a, b)))
        if (d > 0.9999)
            return b
        if (d < -0.9999) {
            // opposite directions: turn around the current screen up axis
            var axis = camUp
            var ang = Math.PI * t
            var c = Math.cos(ang), s = Math.sin(ang)
            return vnorm(vadd(vscale(a, c), vscale(vcross(axis, a), s)))
        }
        var theta = Math.acos(d)
        var sinT = Math.sin(theta)
        var wa = Math.sin((1 - t) * theta) / sinT
        var wb = Math.sin(t * theta) / sinT
        return vnorm(vadd(vscale(a, wa), vscale(b, wb)))
    }

    onAnimTChanged: {
        if (!renderer || !camAnim.running)
            return
        var t = animT
        var target = vadd(animFromTarget, vscale(vsub(animToTarget, animFromTarget), t))
        var dir = slerpDir(animFromDir, animToDir, t)
        var dist = animFromDist + (animToDist - animFromDist) * t
        renderer.cameraTarget = target
        renderer.cameraPosition = vadd(target, vscale(dir, dist))
    }

    NumberAnimation {
        id: camAnim
        target: viewGizmo
        property: "animT"
        from: 0
        to: 1
        duration: 250
        easing.type: Easing.InOutQuad
    }

    // move the camera to look at toTarget from toPos
    function animateCamera(toPos, toTarget) {
        if (!renderer)
            return
        camAnim.stop()
        var fromOffset = vsub(renderer.cameraPosition, renderer.cameraTarget)
        var toOffset = vsub(toPos, toTarget)
        animFromTarget = renderer.cameraTarget
        animToTarget = toTarget
        animFromDir = vnorm(fromOffset)
        animToDir = vnorm(toOffset)
        animFromDist = vlen(fromOffset)
        animToDist = vlen(toOffset)
        animT = 0
        camAnim.start()
    }

    function snapToDirection(forward) {
        if (!renderer)
            return
        var n = vnorm(forward)
        // keep a tiny tilt on top/bottom views so orbiting keeps working
        if (Math.abs(n.y) > 0.999)
            n = vnorm(Qt.vector3d(0, n.y, -0.03))
        var target = renderer.cameraTarget
        animateCamera(vsub(target, vscale(n, cameraDistance())), target)
    }

    function viewFromAxis(i) {
        var v = axisDir(i)
        // clicking the axis we're already looking from flips to the opposite side
        if (vdot(camForward, v) < -0.99)
            v = vscale(v, -1)
        var forward = vscale(v, -1)
        if (snapCamera)
            snapCamera(forward)
        else
            snapToDirection(forward)
    }

    /* ---------------- drawing ---------------- */

    function shade(hex, k) {
        // mix the color toward the dark background by factor k
        var c = Qt.color(hex)
        var bg = [0.16, 0.17, 0.20]
        return "rgb(" + Math.round((c.r * (1 - k) + bg[0] * k) * 255) + ","
                      + Math.round((c.g * (1 - k) + bg[1] * k) * 255) + ","
                      + Math.round((c.b * (1 - k) + bg[2] * k) * 255) + ")"
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        antialiasing: true

        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            ctx.clearRect(0, 0, width, height)

            // background disc
            ctx.beginPath()
            ctx.arc(cx, cy, gizmoRadius, 0, Math.PI * 2)
            ctx.fillStyle = hoverGizmo ? "rgba(255,255,255,0.12)" : "rgba(255,255,255,0.05)"
            ctx.fill()

            var axes = projectedAxes()
            axes.sort(function(a, b) { return a.depth - b.depth })

            for (var i = 0; i < axes.length; ++i) {
                var a = axes[i]
                var axis = a.index % 3
                var positive = a.index < 3
                var color = axisColors[axis]
                var hovered = a.index === hoverAxis

                // stem from the center to the bubble edge
                var dx = a.x - cx
                var dy = a.y - cy
                var len = Math.sqrt(dx * dx + dy * dy)
                if (len > bubbleRadius) {
                    var ex = cx + dx * (len - bubbleRadius) / len
                    var ey = cy + dy * (len - bubbleRadius) / len
                    ctx.beginPath()
                    ctx.moveTo(cx, cy)
                    ctx.lineTo(ex, ey)
                    ctx.lineWidth = 3
                    ctx.lineCap = "round"
                    ctx.strokeStyle = positive ? color : shade(color, 0.35)
                    ctx.stroke()
                }

                // bubble
                ctx.beginPath()
                ctx.arc(a.x, a.y, positive ? bubbleRadius : bubbleRadius - 1.5, 0, Math.PI * 2)
                if (positive) {
                    ctx.fillStyle = color
                    ctx.fill()
                } else {
                    ctx.fillStyle = hovered ? shade(color, 0.3) : shade(color, 0.75)
                    ctx.fill()
                    ctx.lineWidth = 2.5
                    ctx.strokeStyle = shade(color, 0.2)
                    ctx.stroke()
                }

                if (hovered) {
                    ctx.beginPath()
                    ctx.arc(a.x, a.y, bubbleRadius + 1, 0, Math.PI * 2)
                    ctx.lineWidth = 1.5
                    ctx.strokeStyle = "#ffffff"
                    ctx.stroke()
                }

                if (positive || hovered) {
                    ctx.fillStyle = "#ffffff"
                    ctx.font = "bold 12px sans-serif"
                    ctx.textAlign = "center"
                    ctx.textBaseline = "middle"
                    ctx.fillText((positive ? "" : "-") + axisLabels[axis], a.x, a.y + 0.5)
                }
            }

            // center dot
            ctx.beginPath()
            ctx.arc(cx, cy, 2.5, 0, Math.PI * 2)
            ctx.fillStyle = "#c8c8c8"
            ctx.fill()
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        property point pressPos: Qt.point(0, 0)
        property bool dragging: false
        property bool pressedInside: false

        function updateHover(x, y) {
            var inside = insideGizmo(x, y)
            var axis = (inside && !dragging) ? axisAt(x, y) : -1
            if (inside !== hoverGizmo || axis !== hoverAxis) {
                hoverGizmo = inside
                hoverAxis = axis
                canvas.requestPaint()
            }
        }

        onExited: {
            hoverGizmo = false
            hoverAxis = -1
            canvas.requestPaint()
        }
        onPressed: function(mouse) {
            pressedInside = insideGizmo(mouse.x, mouse.y)
            mouse.accepted = pressedInside
            pressPos = Qt.point(mouse.x, mouse.y)
            dragging = false
        }
        onPositionChanged: function(mouse) {
            if (!pressed || !renderer) {
                updateHover(mouse.x, mouse.y)
                return
            }
            var dx = mouse.x - pressPos.x
            var dy = mouse.y - pressPos.y
            if (dragging || Math.abs(dx) > 2 || Math.abs(dy) > 2) {
                if (!dragging) {
                    dragging = true
                    camAnim.stop()
                    hoverAxis = -1
                }
                renderer.rotateFreeCamera(dx * 0.4, dy * 0.4)
                pressPos = Qt.point(mouse.x, mouse.y)
            }
        }
        onReleased: function(mouse) {
            var wasDragging = dragging
            dragging = false
            if (!renderer || wasDragging || !pressedInside)
                return
            var axis = axisAt(mouse.x, mouse.y)
            if (axis >= 0)
                viewFromAxis(axis)
        }
    }

    // home button
    Rectangle {
        id: homeButton
        x: 10
        y: 10
        width: 35
        height: 35
        radius: width / 2
        color: homeArea.containsMouse ? Qt.rgba(1, 1, 1, 0.22) : Qt.rgba(1, 1, 1, 0.08)

        Text {
            anchors.centerIn: parent
            font.family: UISettings.fontAwesomeFontName
            font.pixelSize: parent.height * 0.55
            color: "#e0e0e0"
            text: FontAwesome.fa_house
        }

        MouseArea {
            id: homeArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: viewGizmo.homeRequested()
        }
    }

    Component.onCompleted: {
        updateBasis()
        canvas.requestPaint()
    }

    Connections {
        target: renderer
        function onCameraPositionChanged() { viewGizmo.updateBasis(); canvas.requestPaint() }
        function onCameraTargetChanged() { viewGizmo.updateBasis(); canvas.requestPaint() }
    }
}
