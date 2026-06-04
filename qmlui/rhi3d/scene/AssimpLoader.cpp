/*
  Q Light Controller Plus
  AssimpLoader.cpp

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

#include "scene/AssimpLoader.h"

#include <QDebug>

#ifdef RHIPIPELINE_NO_ASSIMP
static bool s_assimpDisabledLogged = false;

bool AssimpLoader::loadModel(const QString &path, RhiScene &scene)
{
    Q_UNUSED(path);
    Q_UNUSED(scene);
    if (!s_assimpDisabledLogged)
    {
        s_assimpDisabledLogged = true;
        qWarning() << "AssimpLoader: Assimp support disabled, cannot load model";
    }
    return false;
}

bool AssimpLoader::loadModel(const QString &path, RhiScene &scene, bool append)
{
    Q_UNUSED(path);
    Q_UNUSED(scene);
    Q_UNUSED(append);
    if (!s_assimpDisabledLogged)
    {
        s_assimpDisabledLogged = true;
        qWarning() << "AssimpLoader: Assimp support disabled, cannot load model";
    }
    return false;
}
#else
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <assimp/config.h>
#if __has_include(<assimp/GltfMaterial.h>)
#include <assimp/GltfMaterial.h>
#elif __has_include(<assimp/pbrmaterial.h>)
#include <assimp/pbrmaterial.h>
#endif
#include <functional>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QLatin1Char>
#include <QtCore/QUrl>
#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QtGui/QMatrix4x4>
#include <QtGui/QVector3D>
#include <QtGui/QVector2D>

static QMatrix4x4 toQtMatrix(const aiMatrix4x4 &m)
{
    QMatrix4x4 out;
    out.setRow(0, QVector4D(m.a1, m.a2, m.a3, m.a4));
    out.setRow(1, QVector4D(m.b1, m.b2, m.b3, m.b4));
    out.setRow(2, QVector4D(m.c1, m.c2, m.c3, m.c4));
    out.setRow(3, QVector4D(m.d1, m.d2, m.d3, m.d4));
    return out;
}

static QString resolveTexturePath(const QString &modelPath, const QString &texturePath)
{
    if (texturePath.isEmpty())
        return QString();
    const QUrl url(texturePath);
    if (url.isValid() && url.isLocalFile())
        return url.toLocalFile();
    const QFileInfo textureInfo(texturePath);
    if (textureInfo.isAbsolute())
        return texturePath;
    const QUrl modelUrl(modelPath);
    const QString modelFile = modelUrl.isValid() && modelUrl.isLocalFile()
            ? modelUrl.toLocalFile()
            : modelPath;
    const QFileInfo modelInfo(modelFile);
    const QString candidate = QDir(modelInfo.absolutePath()).filePath(texturePath);
    if (QFileInfo::exists(candidate))
        return candidate;
    return QDir::current().filePath(texturePath);
}

static QString resolveModelPath(const QString &path)
{
    const QUrl url(path);
    QString resolved = (url.isValid() && url.isLocalFile()) ? url.toLocalFile() : path;
    const QFileInfo info(resolved);
    if (info.exists())
        return info.absoluteFilePath();
    return resolved;
}

static QImage loadEmbeddedTexture(const aiTexture *texture)
{
    if (!texture)
        return QImage();
    if (texture->mHeight == 0)
    {
        const uchar *data = reinterpret_cast<const uchar *>(texture->pcData);
        QImage image = QImage::fromData(data, int(texture->mWidth));
        return image.isNull() ? QImage() : image.convertToFormat(QImage::Format_RGBA8888);
    }
    const int width = int(texture->mWidth);
    const int height = int(texture->mHeight);
    QImage image(reinterpret_cast<const uchar *>(texture->pcData), width, height, QImage::Format_RGBA8888);
    return image.copy();
}

static bool loadTextureFromPath(const aiScene *scene,
                                const QString &modelPath,
                                const aiString &texturePath,
                                QString &outPath,
                                QImage &outImage)
{
    if (texturePath.length == 0)
        return false;
    const QString texString = QString::fromUtf8(texturePath.C_Str());
    if (texString.startsWith(QLatin1String("data:"), Qt::CaseInsensitive))
    {
        const int comma = texString.indexOf(QLatin1Char(','));
        if (comma <= 0)
            return false;
        const QString meta = texString.left(comma);
        const QString payload = texString.mid(comma + 1);
        if (!meta.contains(QLatin1String(";base64"), Qt::CaseInsensitive))
            return false;
        const QByteArray decoded = QByteArray::fromBase64(payload.toUtf8());
        QImage image = QImage::fromData(decoded);
        if (!image.isNull())
        {
            outPath.clear();
            outImage = image.convertToFormat(QImage::Format_RGBA8888);
            return true;
        }
        return false;
    }
    if (texString.startsWith(QLatin1Char('*')))
    {
        bool ok = false;
        const int index = texString.mid(1).toInt(&ok);
        if (ok && scene && index >= 0 && index < int(scene->mNumTextures))
        {
            outImage = loadEmbeddedTexture(scene->mTextures[index]);
            return !outImage.isNull();
        }
        return false;
    }
    const QString resolved = resolveTexturePath(modelPath, texString);
    outPath = resolved;
    QImage image(resolved);
    if (!image.isNull())
    {
        outImage = image.convertToFormat(QImage::Format_RGBA8888);
        return true;
    }
    return false;
}

static bool loadTextureForType(const aiScene *scene,
                               const QString &modelPath,
                               const aiMaterial *mat,
                               aiTextureType type,
                               QString &outPath,
                               QImage &outImage)
{
    aiString texturePath;
    if (aiGetMaterialTexture(mat, type, 0, &texturePath) != AI_SUCCESS)
        return false;
    return loadTextureFromPath(scene, modelPath, texturePath, outPath, outImage);
}

static QImage buildMetallicRoughnessMap(const QImage &metalness, const QImage &roughness)
{
    if (metalness.isNull() && roughness.isNull())
        return QImage();

    const QImage metal = metalness.isNull()
            ? QImage()
            : metalness.convertToFormat(QImage::Format_RGBA8888);
    const QImage rough = roughness.isNull()
            ? QImage()
            : roughness.convertToFormat(QImage::Format_RGBA8888);

    QSize size;
    if (!metal.isNull())
        size = metal.size();
    else
        size = rough.size();

    QImage metalScaled = metal;
    QImage roughScaled = rough;
    if (!metalScaled.isNull() && metalScaled.size() != size)
        metalScaled = metalScaled.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    if (!roughScaled.isNull() && roughScaled.size() != size)
        roughScaled = roughScaled.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    QImage combined(size, QImage::Format_RGBA8888);
    for (int y = 0; y < size.height(); ++y)
    {
        const QRgb *metalLine = metalScaled.isNull() ? nullptr : reinterpret_cast<const QRgb *>(metalScaled.constScanLine(y));
        const QRgb *roughLine = roughScaled.isNull() ? nullptr : reinterpret_cast<const QRgb *>(roughScaled.constScanLine(y));
        QRgb *outLine = reinterpret_cast<QRgb *>(combined.scanLine(y));
        for (int x = 0; x < size.width(); ++x)
        {
            const int metalValue = metalLine ? qRed(metalLine[x]) : 0;
            const int roughValue = roughLine ? qRed(roughLine[x]) : 255;
            outLine[x] = qRgba(0, roughValue, metalValue, 255);
        }
    }
    return combined;
}

static void readMaterial(const aiScene *scene, const QString &modelPath, const aiMaterial *mat, Material &out)
{
    aiColor4D color;
    if (aiGetMaterialColor(mat, AI_MATKEY_BASE_COLOR, &color) == AI_SUCCESS)
    {
        out.baseColor = QVector3D(color.r, color.g, color.b);
        out.baseAlpha = color.a;
    }
    else if (aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &color) == AI_SUCCESS)
    {
        out.baseColor = QVector3D(color.r, color.g, color.b);
        out.baseAlpha = color.a;
    }

    float metallic = 0.0f;
    if (aiGetMaterialFloat(mat, AI_MATKEY_METALLIC_FACTOR, &metallic) == AI_SUCCESS)
        out.metalness = metallic;

    float roughness = 0.5f;
    if (aiGetMaterialFloat(mat, AI_MATKEY_ROUGHNESS_FACTOR, &roughness) == AI_SUCCESS)
        out.roughness = roughness;

#ifdef AI_MATKEY_GLTF_OCCLUSION_STRENGTH
    float occlusionStrength = 1.0f;
    if (aiGetMaterialFloat(mat, AI_MATKEY_GLTF_OCCLUSION_STRENGTH, &occlusionStrength) == AI_SUCCESS)
        out.occlusion = occlusionStrength;
#endif

    if (aiGetMaterialColor(mat, AI_MATKEY_COLOR_EMISSIVE, &color) == AI_SUCCESS)
        out.emissive = QVector3D(color.r, color.g, color.b);

    loadTextureForType(scene, modelPath, mat, aiTextureType_BASE_COLOR, out.baseColorMapPath, out.baseColorMap);
    if (out.baseColorMap.isNull())
        loadTextureForType(scene, modelPath, mat, aiTextureType_DIFFUSE, out.baseColorMapPath, out.baseColorMap);

    loadTextureForType(scene, modelPath, mat, aiTextureType_NORMALS, out.normalMapPath, out.normalMap);
    if (out.normalMap.isNull())
        loadTextureForType(scene, modelPath, mat, aiTextureType_HEIGHT, out.normalMapPath, out.normalMap);

    loadTextureForType(scene, modelPath, mat, aiTextureType_LIGHTMAP, out.occlusionMapPath, out.occlusionMap);
    loadTextureForType(scene, modelPath, mat, aiTextureType_EMISSIVE, out.emissiveMapPath, out.emissiveMap);

    QString metalPath;
    QString roughPath;
    QImage metalImage;
    QImage roughImage;
    if (loadTextureForType(scene, modelPath, mat, aiTextureType_UNKNOWN,
                           out.metallicRoughnessMapPath, out.metallicRoughnessMap))
        return;
    const bool hasMetal = loadTextureForType(scene, modelPath, mat, aiTextureType_METALNESS, metalPath, metalImage);
    const bool hasRough = loadTextureForType(scene, modelPath, mat, aiTextureType_DIFFUSE_ROUGHNESS, roughPath, roughImage);

    if (hasMetal || hasRough)
    {
        out.metallicRoughnessMap = buildMetallicRoughnessMap(metalImage, roughImage);
        if (hasMetal)
            out.metallicRoughnessMapPath = metalPath;
        else
            out.metallicRoughnessMapPath = roughPath;
    }
    else
    {
        loadTextureForType(scene, modelPath, mat, aiTextureType_UNKNOWN,
                           out.metallicRoughnessMapPath, out.metallicRoughnessMap);
    }

    int twoSided = 0;
    if (aiGetMaterialInteger(mat, AI_MATKEY_TWOSIDED, &twoSided) == AI_SUCCESS)
        out.doubleSided = (twoSided != 0);

#ifdef AI_MATKEY_GLTF_ALPHAMODE
    aiString alphaMode;
    if (aiGetMaterialString(mat, AI_MATKEY_GLTF_ALPHAMODE, &alphaMode) == AI_SUCCESS)
    {
        const QString mode = QString::fromUtf8(alphaMode.C_Str()).toUpper();
        if (mode == QLatin1String("MASK"))
            out.alphaMode = Material::AlphaMode::Mask;
        else if (mode == QLatin1String("BLEND"))
            out.alphaMode = Material::AlphaMode::Blend;
        else
            out.alphaMode = Material::AlphaMode::Opaque;
    }
#endif

#ifdef AI_MATKEY_GLTF_ALPHACUTOFF
    float alphaCutoff = 0.5f;
    if (aiGetMaterialFloat(mat, AI_MATKEY_GLTF_ALPHACUTOFF, &alphaCutoff) == AI_SUCCESS)
        out.alphaCutoff = alphaCutoff;
#endif
}

bool AssimpLoader::loadModel(const QString &path, RhiScene &scene)
{
    return loadModel(path, scene, false);
}

bool AssimpLoader::loadModel(const QString &path, RhiScene &scene, bool append)
{
#ifdef QLC_RHI_DEBUG_ASSIMP
    const int originalMeshCount = scene.meshes().size();
#endif
    if (!append)
        scene.meshes().clear();
#ifdef QLC_RHI_DEBUG_ASSIMP
    const int baseMeshCount = scene.meshes().size();
#endif

    Assimp::Importer importer;
    importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 60.0f);
    const QString resolvedPath = resolveModelPath(path);
    if (resolvedPath.isEmpty())
    {
        qWarning() << "AssimpLoader: empty model path";
        return false;
    }
    if (!QFileInfo::exists(resolvedPath))
    {
        qWarning() << "AssimpLoader: model file does not exist:" << resolvedPath
                   << "(original path:" << path << ")";
    }

    const aiScene *ai = importer.ReadFile(resolvedPath.toStdString(),
                                         aiProcess_Triangulate |
                                         aiProcess_GenSmoothNormals |
                                         aiProcess_GenUVCoords |
                                         aiProcess_CalcTangentSpace |
                                         aiProcess_JoinIdenticalVertices);
    if (!ai)
    {
        qWarning() << "AssimpLoader: failed to import model" << resolvedPath
                   << "error:" << importer.GetErrorString();
        return false;
    }
    if (ai->mRootNode == nullptr)
    {
        qWarning() << "AssimpLoader: imported scene has no root node for" << resolvedPath;
        return false;
    }

    std::function<void(aiNode *, const QMatrix4x4 &)> visitNode;
    visitNode = [&](aiNode *node, const QMatrix4x4 &parent)
    {
        const QMatrix4x4 local = toQtMatrix(node->mTransformation);
        const QMatrix4x4 world = parent * local;

        const QString nodeName = QString::fromUtf8(node->mName.C_Str());
        for (unsigned int i = 0; i < node->mNumMeshes; ++i)
        {
            const aiMesh *m = ai->mMeshes[node->mMeshes[i]];
            Mesh mesh;
            if (!nodeName.isEmpty())
                mesh.name = nodeName;
            else if (m->mName.length > 0)
                mesh.name = QString::fromUtf8(m->mName.C_Str());
            mesh.modelMatrix = world;
            mesh.vertices.reserve(int(m->mNumVertices));
            mesh.indices.reserve(int(m->mNumFaces * 3));
            QVector3D minV;
            QVector3D maxV;
            bool haveBounds = false;

            const bool flipV = resolvedPath.endsWith(QLatin1String(".gltf"), Qt::CaseInsensitive)
                    || resolvedPath.endsWith(QLatin1String(".glb"), Qt::CaseInsensitive);
            for (unsigned int v = 0; v < m->mNumVertices; ++v)
            {
                const aiVector3D pos = m->mVertices[v];
                const aiVector3D nrm = m->HasNormals() ? m->mNormals[v] : aiVector3D(0.0f, 1.0f, 0.0f);
                const bool hasUv0 = m->HasTextureCoords(0);
                aiVector3D uv = hasUv0 ? m->mTextureCoords[0][v] : aiVector3D(0.0f, 0.0f, 0.0f);
                if (flipV)
                    uv.y = 1.0f - uv.y;
                Vertex vert;
                vert.px = pos.x;
                vert.py = pos.y;
                vert.pz = pos.z;
                vert.nx = nrm.x;
                vert.ny = nrm.y;
                vert.nz = nrm.z;
                vert.u = uv.x;
                vert.v = uv.y;
                mesh.vertices.push_back(vert);

                if (!haveBounds)
                {
                    minV = QVector3D(pos.x, pos.y, pos.z);
                    maxV = minV;
                    haveBounds = true;
                }
                else
                {
                    minV.setX(qMin(minV.x(), pos.x));
                    minV.setY(qMin(minV.y(), pos.y));
                    minV.setZ(qMin(minV.z(), pos.z));
                    maxV.setX(qMax(maxV.x(), pos.x));
                    maxV.setY(qMax(maxV.y(), pos.y));
                    maxV.setZ(qMax(maxV.z(), pos.z));
                }
            }
            if (haveBounds)
            {
                mesh.boundsMin = minV;
                mesh.boundsMax = maxV;
                mesh.boundsValid = true;
            }

            if (!mesh.vertices.isEmpty())
            {
                float minU = mesh.vertices[0].u;
                float maxU = mesh.vertices[0].u;
                float minVuv = mesh.vertices[0].v;
                float maxVuv = mesh.vertices[0].v;
                for (const Vertex &vtx : mesh.vertices)
                {
                    minU = qMin(minU, vtx.u);
                    maxU = qMax(maxU, vtx.u);
                    minVuv = qMin(minVuv, vtx.v);
                    maxVuv = qMax(maxVuv, vtx.v);
                }
                const float rangeU = maxU - minU;
                const float rangeV = maxVuv - minVuv;
                if (rangeU < 1e-5f && rangeV < 1e-5f && mesh.boundsValid)
                {
                    const QVector3D extents = mesh.boundsMax - mesh.boundsMin;
                    const float ex = qMax(extents.x(), 1e-5f);
                    const float ey = qMax(extents.y(), 1e-5f);
                    const float ez = qMax(extents.z(), 1e-5f);
                    int dropAxis = 0; // 0=X, 1=Y, 2=Z
                    if (ex <= ey && ex <= ez)
                        dropAxis = 0;
                    else if (ey <= ex && ey <= ez)
                        dropAxis = 1;
                    else
                        dropAxis = 2;
                    for (Vertex &vtx : mesh.vertices)
                    {
                        const float px = vtx.px;
                        const float py = vtx.py;
                        const float pz = vtx.pz;
                        if (dropAxis == 0)
                        {
                            vtx.u = (pz - mesh.boundsMin.z()) / ez;
                            vtx.v = (py - mesh.boundsMin.y()) / ey;
                        }
                        else if (dropAxis == 1)
                        {
                            vtx.u = (px - mesh.boundsMin.x()) / ex;
                            vtx.v = (pz - mesh.boundsMin.z()) / ez;
                        }
                        else
                        {
                            vtx.u = (px - mesh.boundsMin.x()) / ex;
                            vtx.v = (py - mesh.boundsMin.y()) / ey;
                        }
                    }
                }
            }

            for (unsigned int f = 0; f < m->mNumFaces; ++f)
            {
                const aiFace &face = m->mFaces[f];
                if (face.mNumIndices != 3)
                    continue;
                mesh.indices.push_back(face.mIndices[0]);
                mesh.indices.push_back(face.mIndices[1]);
                mesh.indices.push_back(face.mIndices[2]);
            }

            if (m->mMaterialIndex < ai->mNumMaterials)
            {
                const aiMaterial *mat = ai->mMaterials[m->mMaterialIndex];
                readMaterial(ai, resolvedPath, mat, mesh.material);
            }

            scene.meshes().push_back(mesh);
        }

        for (unsigned int c = 0; c < node->mNumChildren; ++c)
            visitNode(node->mChildren[c], world);
    };

    visitNode(ai->mRootNode, QMatrix4x4());
    if (scene.meshes().isEmpty())
    {
        qWarning() << "AssimpLoader: imported model contains no meshes:" << resolvedPath;
        return false;
    }

#ifdef QLC_RHI_DEBUG_ASSIMP
    const int loadedMeshes = scene.meshes().size() - baseMeshCount;
    qDebug() << "AssimpLoader: loaded model"
             << resolvedPath
             << "append" << append
             << "loadedMeshes" << loadedMeshes
             << "sceneMeshesBefore" << originalMeshCount
             << "sceneMeshesAfter" << scene.meshes().size();

    if (resolvedPath.contains(QStringLiteral("truss_"), Qt::CaseInsensitive))
    {
        for (int i = baseMeshCount; i < scene.meshes().size(); ++i)
        {
            const Mesh &mesh = scene.meshes().at(i);
            qDebug().noquote()
                    << QString("AssimpLoader[truss]: mesh=%1 base=(%2,%3,%4) metal=%5 rough=%6 mrTexNull=%7 mrTexSize=%8x%9")
                          .arg(mesh.name)
                          .arg(mesh.material.baseColor.x(), 0, 'f', 3)
                          .arg(mesh.material.baseColor.y(), 0, 'f', 3)
                          .arg(mesh.material.baseColor.z(), 0, 'f', 3)
                          .arg(mesh.material.metalness, 0, 'f', 3)
                          .arg(mesh.material.roughness, 0, 'f', 3)
                          .arg(mesh.material.metallicRoughnessMap.isNull() ? "true" : "false")
                          .arg(mesh.material.metallicRoughnessMap.width())
                          .arg(mesh.material.metallicRoughnessMap.height());
        }
    }
#endif
    return true;
}
#endif
