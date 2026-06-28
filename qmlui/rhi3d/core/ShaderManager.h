#pragma once

#include <QtCore/QHash>
#include <QtCore/QByteArray>
#include <rhi/qrhi.h>

class ShaderManager
{
public:
    explicit ShaderManager(QRhi *rhi);

    QRhiShaderResourceBindings *getOrCreateBindings(const QByteArray &key);
    QRhiShaderStage loadStage(QRhiShaderStage::Type type, const QString &path);

private:
    QRhi *m_rhi = nullptr;
    QHash<QByteArray, QRhiShaderResourceBindings *> m_srbCache;
};
