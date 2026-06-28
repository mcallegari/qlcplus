#include "core/ShaderManager.h"

#include <QtCore/QFile>

ShaderManager::ShaderManager(QRhi *rhi)
    : m_rhi(rhi)
{
}

QRhiShaderResourceBindings *ShaderManager::getOrCreateBindings(const QByteArray &key)
{
    if (m_srbCache.contains(key))
        return m_srbCache.value(key);

    // TODO: create SRB based on key.
    QRhiShaderResourceBindings *srb = nullptr;
    m_srbCache.insert(key, srb);
    return srb;
}

QRhiShaderStage ShaderManager::loadStage(QRhiShaderStage::Type type, const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open shader:" << path;
        return {};
    }

    const QByteArray payload = f.readAll();
    const QShader shader = QShader::fromSerialized(payload);
    if (!shader.isValid())
    {
        qWarning() << "Invalid shader payload:" << path;
        return {};
    }
    return QRhiShaderStage(type, shader);
}
