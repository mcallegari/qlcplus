import QtQuick 2.15

Item {
    id: viewCube
    property var renderer: null
    property var snapCamera: null
    width: 120
    height: 120
    z: 10

    property vector3d camForward: Qt.vector3d(0, 0, -1)
    property vector3d camRight: Qt.vector3d(1, 0, 0)
    property vector3d camUp: Qt.vector3d(0, 1, 0)

    function vdot(a, b) { return a.x * b.x + a.y * b.y + a.z * b.z }
    function vcross(a, b) {
        return Qt.vector3d(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        )
    }
    function vlen(a) { return Math.sqrt(a.x * a.x + a.y * a.y + a.z * a.z) }
    function vnorm(a) {
        var l = vlen(a)
        if (l <= 0.00001)
            return Qt.vector3d(0, 0, -1)
        return Qt.vector3d(a.x / l, a.y / l, a.z / l)
    }

    function updateBasis() {
        if (!renderer)
            return
        var f = vnorm(Qt.vector3d(renderer.cameraTarget.x - renderer.cameraPosition.x,
                                  renderer.cameraTarget.y - renderer.cameraPosition.y,
                                  renderer.cameraTarget.z - renderer.cameraPosition.z))
        var upWorld = Qt.vector3d(0, 1, 0)
        var r = vcross(f, upWorld)
        if (vlen(r) < 0.0001)
            r = vcross(f, Qt.vector3d(0, 0, 1))
        r = vnorm(r)
        var u = vnorm(vcross(r, f))
        camForward = f
        camRight = r
        camUp = u
    }

    function cameraDistance() {
        if (!renderer)
            return 5.0
        var dx = renderer.cameraTarget.x - renderer.cameraPosition.x
        var dy = renderer.cameraTarget.y - renderer.cameraPosition.y
        var dz = renderer.cameraTarget.z - renderer.cameraPosition.z
        var dist = Math.sqrt(dx * dx + dy * dy + dz * dz)
        return Math.max(dist, 0.5)
    }

    function faceForward(faceIndex) {
        switch (faceIndex) {
        case 0: return Qt.vector3d(0, 0, 1)   // back (-Z face)
        case 1: return Qt.vector3d(0, 0, -1)  // front (+Z face)
        case 2: return Qt.vector3d(0, 1, 0)   // bottom (-Y face)
        case 3: return Qt.vector3d(0, -1, 0)  // top (+Y face)
        case 4: return Qt.vector3d(1, 0, 0)   // left (-X face)
        case 5: return Qt.vector3d(-1, 0, 0)  // right (+X face)
        default: return Qt.vector3d(0, 0, -1)
        }
    }

    function snapToDirection(dir) {
        if (!renderer)
            return
        var n = vnorm(dir)
        var dist = cameraDistance()
        var target = renderer.cameraTarget
        renderer.cameraPosition = Qt.vector3d(target.x - n.x * dist,
                                              target.y - n.y * dist,
                                              target.z - n.z * dist)
        renderer.cameraTarget = target
    }

    function pointInTriangle(px, py, ax, ay, bx, by, cx, cy) {
        var v0x = cx - ax
        var v0y = cy - ay
        var v1x = bx - ax
        var v1y = by - ay
        var v2x = px - ax
        var v2y = py - ay

        var dot00 = v0x * v0x + v0y * v0y
        var dot01 = v0x * v1x + v0y * v1y
        var dot02 = v0x * v2x + v0y * v2y
        var dot11 = v1x * v1x + v1y * v1y
        var dot12 = v1x * v2x + v1y * v2y

        var denom = dot00 * dot11 - dot01 * dot01
        if (Math.abs(denom) < 0.00001)
            return false
        var invDenom = 1.0 / denom
        var u = (dot11 * dot02 - dot01 * dot12) * invDenom
        var v = (dot00 * dot12 - dot01 * dot02) * invDenom
        return u >= 0.0 && v >= 0.0 && (u + v) <= 1.0
    }

    function pointInQuad(px, py, p0, p1, p2, p3) {
        return pointInTriangle(px, py, p0.x, p0.y, p1.x, p1.y, p2.x, p2.y)
                || pointInTriangle(px, py, p0.x, p0.y, p2.x, p2.y, p3.x, p3.y)
    }

    function hitFaceAt(x, y) {
        var cx = width * 0.5
        var cy = height * 0.5
        var scale = width * 0.32

        function rot(v) {
            return Qt.vector3d(
                vdot(v, viewCube.camRight),
                vdot(v, viewCube.camUp),
                vdot(v, Qt.vector3d(-viewCube.camForward.x, -viewCube.camForward.y, -viewCube.camForward.z))
            )
        }
        function proj(v) {
            return Qt.point(cx + v.x * scale, cy - v.y * scale)
        }

        var verts = [
            Qt.vector3d(-1, -1, -1), Qt.vector3d(1, -1, -1), Qt.vector3d(1, 1, -1), Qt.vector3d(-1, 1, -1),
            Qt.vector3d(-1, -1, 1), Qt.vector3d(1, -1, 1), Qt.vector3d(1, 1, 1), Qt.vector3d(-1, 1, 1)
        ]
        var faces = [
            [0, 1, 2, 3], [4, 5, 6, 7], [0, 1, 5, 4], [3, 2, 6, 7], [0, 3, 7, 4], [1, 2, 6, 5]
        ]

        var rverts = []
        for (var i = 0; i < verts.length; ++i)
            rverts.push(rot(verts[i]))

        var candidates = []
        for (var f = 0; f < faces.length; ++f) {
            var face = faces[f]
            var depth = (rverts[face[0]].z + rverts[face[1]].z + rverts[face[2]].z + rverts[face[3]].z) * 0.25
            candidates.push({ index: f, depth: depth })
        }

        candidates.sort(function(a, b) { return b.depth - a.depth })

        for (var c = 0; c < candidates.length; ++c) {
            var idx = candidates[c].index
            var faceVerts = faces[idx]
            var p0 = proj(rverts[faceVerts[0]])
            var p1 = proj(rverts[faceVerts[1]])
            var p2 = proj(rverts[faceVerts[2]])
            var p3 = proj(rverts[faceVerts[3]])
            if (pointInQuad(x, y, p0, p1, p2, p3))
                return idx
        }

        return -1
    }

    Canvas {
        id: cubeCanvas
        anchors.fill: parent
        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            var cx = width * 0.5
            var cy = height * 0.5
            var scale = width * 0.32
            var lightDir = Qt.vector3d(0.4, 0.6, 0.7)
            var lightLen = Math.sqrt(lightDir.x * lightDir.x + lightDir.y * lightDir.y + lightDir.z * lightDir.z)
            if (lightLen > 0.0001)
                lightDir = Qt.vector3d(lightDir.x / lightLen, lightDir.y / lightLen, lightDir.z / lightLen)

            function rot(v) {
                return Qt.vector3d(
                    vdot(v, viewCube.camRight),
                    vdot(v, viewCube.camUp),
                    vdot(v, Qt.vector3d(-viewCube.camForward.x, -viewCube.camForward.y, -viewCube.camForward.z))
                )
            }
            function proj(v) {
                return Qt.point(cx + v.x * scale, cy - v.y * scale)
            }

            var verts = [
                Qt.vector3d(-1, -1, -1), Qt.vector3d(1, -1, -1), Qt.vector3d(1, 1, -1), Qt.vector3d(-1, 1, -1),
                Qt.vector3d(-1, -1, 1), Qt.vector3d(1, -1, 1), Qt.vector3d(1, 1, 1), Qt.vector3d(-1, 1, 1)
            ]
            var edges = [
                [0, 1], [1, 2], [2, 3], [3, 0], [4, 5], [5, 6], [6, 7], [7, 4], [0, 4], [1, 5], [2, 6], [3, 7]
            ]
            var faces = [
                [0, 1, 2, 3], [4, 5, 6, 7], [0, 1, 5, 4], [3, 2, 6, 7], [0, 3, 7, 4], [1, 2, 6, 5]
            ]

            var rverts = []
            for (var i = 0; i < verts.length; ++i)
                rverts.push(rot(verts[i]))

            function faceDepth(face) {
                return (rverts[face[0]].z + rverts[face[1]].z + rverts[face[2]].z + rverts[face[3]].z) * 0.25
            }

            function faceNormal(face) {
                var a = rverts[face[0]]
                var b = rverts[face[1]]
                var c = rverts[face[2]]
                var ab = Qt.vector3d(b.x - a.x, b.y - a.y, b.z - a.z)
                var ac = Qt.vector3d(c.x - a.x, c.y - a.y, c.z - a.z)
                return vnorm(vcross(ab, ac))
            }

            var faceOrder = faces.slice()
            faceOrder.sort(function(a, b) { return faceDepth(a) - faceDepth(b) })

            for (var f = 0; f < faceOrder.length; ++f) {
                var face = faceOrder[f]
                var n = faceNormal(face)
                var lit = Math.max(0.1, vdot(n, lightDir))
                var shade = Math.floor(90 + 140 * lit)
                ctx.fillStyle = "rgb(" + shade + "," + shade + "," + shade + ")"
                ctx.beginPath()
                var p0 = proj(rverts[face[0]])
                ctx.moveTo(p0.x, p0.y)
                for (var k = 1; k < 4; ++k) {
                    var pk = proj(rverts[face[k]])
                    ctx.lineTo(pk.x, pk.y)
                }
                ctx.closePath()
                ctx.fill()
            }

            ctx.lineWidth = 2
            ctx.strokeStyle = "#f3e86b"
            ctx.beginPath()
            for (var e = 0; e < edges.length; ++e) {
                var a = proj(rverts[edges[e][0]])
                var b = proj(rverts[edges[e][1]])
                ctx.moveTo(a.x, a.y)
                ctx.lineTo(b.x, b.y)
            }
            ctx.stroke()
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        property point pressPos: Qt.point(0, 0)
        property bool dragging: false

        onPressed: function(mouse) {
            pressPos = Qt.point(mouse.x, mouse.y)
            dragging = false
        }
        onPositionChanged: function(mouse) {
            if (!pressed || !renderer)
                return
            var dx = mouse.x - pressPos.x
            var dy = mouse.y - pressPos.y
            if (Math.abs(dx) > 1 || Math.abs(dy) > 1) {
                dragging = true
                renderer.rotateFreeCamera(dx * 0.2, dy * 0.2)
                pressPos = Qt.point(mouse.x, mouse.y)
            }
        }
        onReleased: function(mouse) {
            if (!renderer || dragging)
                return
            var faceIndex = hitFaceAt(mouse.x, mouse.y)
            if (faceIndex < 0)
                return
            var dir = faceForward(faceIndex)
            if (snapCamera)
                snapCamera(dir)
            else
                snapToDirection(dir)
        }
    }

    Component.onCompleted: {
        updateBasis()
        cubeCanvas.requestPaint()
    }

    Connections {
        target: renderer
        function onCameraPositionChanged() { viewCube.updateBasis(); cubeCanvas.requestPaint() }
        function onCameraTargetChanged() { viewCube.updateBasis(); cubeCanvas.requestPaint() }
    }
}
