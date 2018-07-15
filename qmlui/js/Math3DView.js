/*
  Q Light Controller Plus
  Math3DView.js

  Copyright (c) Massimo Callegari, Eric Arnebäck

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

function perspective(fovy, aspect, zNear, zFar)
{
    var ymax = zNear * Math.tan(fovy);
    var xmax = ymax * aspect;
    var left = -xmax;
    var right = +xmax;
    var bottom = -ymax;
    var top = +ymax;
    var f1, f2, f3, f4;
    f1 = 2.0 * zNear;
    f2 = right - left;
    f3 = top - bottom;
    f4 = zFar - zNear;
    return Qt.matrix4x4(
        f1 / f2, 0.0, 0.0, 0.0,
        0.0, f1 / f3, 0.0, 0.0,
        (right + left) / f2, (top + bottom) / f3, (-zFar - zNear) / f4, -1.0,
        0.0, 0.0, (-zFar * f1) / f4, 0.0).transposed();
}

function getLightViewMatrix(lightMatrix, panRotation, tiltRotation, lightPos)
{
    var m = Qt.matrix4x4();
    m = m.times(lightMatrix);
    m.rotate(panRotation, Qt.vector3d(0, 1, 0));
    m.rotate(tiltRotation, Qt.vector3d(1, 0, 0));

    // extract the axes of our view matrix.
    var xb = (m.times(Qt.vector4d(1.0, 0.0, 0.0, 0.0))).toVector3d();
    var yb = (m.times(Qt.vector4d(0.0, 1.0, 0.0, 0.0))).toVector3d();
    var zb = (m.times(Qt.vector4d(0.0, 0.0, 1.0, 0.0))).toVector3d();
    var left = zb;
    var u = xb;
    var forward = yb;
    var eye = lightPos;

    return Qt.matrix4x4(
        left.x, u.x, forward.x, 0.0,
        left.y, u.y, forward.y, 0.0,
        left.z, u.z, forward.z, 0.0,
        -left.dotProduct(eye), -u.dotProduct(eye), -forward.dotProduct(eye), 1.0).transposed();
}

function getLightProjectionMatrix(distCutoff, coneBottomRadius, coneTopRadius, headLength, cutoffAngle)
{
    var d = distCutoff / (coneBottomRadius / coneTopRadius - 1.0);
    var trans = -d + 0.5 * headLength;

    return perspective(cutoffAngle, 1.0,  d, d + distCutoff).times(Qt.matrix4x4(
                1.0, 0.0, 0.0, 0.0,
                0.0, 1.0, 0.0, 0.0,
                0.0, 0.0, 1.0, trans,
                0.0, 0.0, 0.0, 1.0));
}

function lookAt(eye, center, up)
{
    // compute the basis vectors.
    var forward = (center.minus(eye)).normalized(); // forward vector
    var left = (forward.crossProduct(up)).normalized(); // left vector
    var u = (left.crossProduct(forward)).normalized(); // up vector

    return Qt.matrix4x4(
        left.x, u.x, -forward.x, 0.0,
        left.y, u.y, -forward.y, 0.0,
        left.z, u.z, -forward.z, 0.0,
        -left.dotProduct(eye), -u.dotProduct(eye), forward.dotProduct(eye), 1.0).transposed();
}

function getLightDirection(transform, panTransform, tiltTransform)
{
    var m = transform.matrix;
    if (panTransform) {
        m = transform.matrix.times(panTransform.matrix);
    }
    if (tiltTransform) {
        m = m.times(tiltTransform.matrix);
    }
    m = m.times(Qt.vector4d(0.0, -1.0, 0.0, 0.0));

    return (m.toVector3d().normalized());
}

function getLightViewProjectionScaleOffsetMatrix(lightViewProjectionMatrix)
{
    return Qt.matrix4x4(
            0.5, 0.0, 0.0, 0.5,
            0.0, 0.5, 0.0, 0.5,
            0.0, 0.0, 0.5, 0.5,
            0.0, 0.0, 0.0, 1.0).times(lightViewProjectionMatrix);
}
