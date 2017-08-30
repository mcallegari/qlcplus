/*
  Q Light Controller Plus
  SelectionGeometry.qml

  Copyright (c) Massimo Callegari

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

import Qt3D.Core 2.0
import Qt3D.Render 2.0
import Qt3D.Extras 2.0

GeometryRenderer
{
    id: customMesh
    instanceCount: 1
    indexOffset: 0
    firstInstance: 0
    vertexCount: 24
    primitiveType: GeometryRenderer.Lines

    function vertexBufferData()
    {
        // Vertices
        var dimension = 0.5
        var vArray = new Float32Array(8 * 3);
        vArray[0]  = -dimension; vArray[1]  = -dimension; vArray[2]  = -dimension;
        vArray[3]  = dimension;  vArray[4]  = -dimension; vArray[5]  = -dimension;
        vArray[6]  = dimension;  vArray[7]  = -dimension; vArray[8]  = dimension;
        vArray[9]  = -dimension; vArray[10] = -dimension; vArray[11] = dimension;
        vArray[12] = -dimension; vArray[13] = dimension;  vArray[14] = -dimension;
        vArray[15] = dimension;  vArray[16] = dimension;  vArray[17] = -dimension;
        vArray[18] = dimension;  vArray[19] = dimension;  vArray[20] = dimension;
        vArray[21] = -dimension; vArray[22] = dimension;  vArray[23] = dimension;

        return vArray;
    }

    function indexBufferData()
    {
        var iArray = new Uint16Array(24);

        iArray[0]  = 0; iArray[1]  = 1;
        iArray[2]  = 1; iArray[3]  = 2;
        iArray[4]  = 2; iArray[5]  = 3;
        iArray[6]  = 3; iArray[7]  = 0;
        iArray[8]  = 0; iArray[9]  = 4;
        iArray[10] = 1; iArray[11] = 5;
        iArray[12] = 2; iArray[13] = 6;
        iArray[14] = 3; iArray[15] = 7;
        iArray[16] = 4; iArray[17] = 5;
        iArray[18] = 5; iArray[19] = 6;
        iArray[20] = 6; iArray[21] = 7;
        iArray[22] = 7; iArray[23] = 4;

        return iArray;
    }

    Buffer
    {
        id: vertexBuffer
        type: Buffer.VertexBuffer
        data: vertexBufferData()
    }

    Buffer
    {
        id: indexBuffer
        type: Buffer.IndexBuffer
        data: indexBufferData()
    }

    geometry:
        Geometry
        {
            Attribute
            {
                attributeType: Attribute.VertexAttribute
                vertexBaseType: Attribute.Float
                vertexSize: 3
                byteOffset: 0
                byteStride: 0
                count: 8
                name: defaultPositionAttributeName
                buffer: vertexBuffer
            }

            Attribute
            {
                attributeType: Attribute.IndexAttribute
                vertexBaseType: Attribute.UnsignedShort
                vertexSize: 1
                byteOffset: 0
                byteStride: 0
                count: 24
                buffer: indexBuffer
            }
        }
}

