<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="es_ES">
<context>
    <name>ConfigureSigNet</name>
    <message>
        <location filename="configuresignet.ui" line="14"/>
        <source>Configure Sig-Net Plugin</source>
        <translation>Configurar Plugin Sig-Net</translation>
    </message>
    <message>
        <location filename="configuresignet.ui" line="24"/>
        <source>General</source>
        <translation>General</translation>
    </message>
    <message>
        <location filename="configuresignet.ui" line="40"/>
        <location filename="configuresignet.ui" line="54"/>
        <source>Generate</source>
        <translation>Generar</translation>
    </message>
    <message>
        <location filename="configuresignet.ui" line="47"/>
        <source>Root Key (K0)</source>
        <translation>Clave raíz (K0)</translation>
    </message>
    <message>
        <location filename="configuresignet.ui" line="67"/>
        <source>Seconds to wait for an interface to be ready</source>
        <translation>Segundos a esperar que esté preparada una interfaz</translation>
    </message>
    <message>
        <location filename="configuresignet.ui" line="74"/>
        <source>Scope</source>
        <translation>Ámbito</translation>
    </message>
    <message>
        <location filename="configuresignet.ui" line="81"/>
        <source>Local TUID</source>
        <translation>TUID local</translation>
    </message>
    <message>
        <location filename="configuresignet.ui" line="106"/>
        <source>Sig-Net output uses the configured K0 to derive sender and manager keys. RDM commands require a target node TUID and endpoint per universe mapping.</source>
        <translation>La salida Sig-Net utiliza la K0 configurada para derivar las claves de emisor y gestor. Los comandos RDM requieren un TUID de nodo de destino y un endpoint por cada asignación de universo.</translation>
    </message>
    <message>
        <location filename="configuresignet.ui" line="117"/>
        <source>Universes</source>
        <translation>Universos</translation>
    </message>
    <message>
        <location filename="configuresignet.ui" line="130"/>
        <source>Interface</source>
        <translation>Interfaz</translation>
    </message>
    <message>
        <location filename="configuresignet.ui" line="135"/>
        <source>Universe</source>
        <translation>Universo</translation>
    </message>
    <message>
        <location filename="configuresignet.ui" line="140"/>
        <source>Sig-Net Universe</source>
        <translation>Universo Sig-Net</translation>
    </message>
    <message>
        <location filename="configuresignet.ui" line="145"/>
        <source>Sender Endpoint</source>
        <translation>Endpoint del emisor</translation>
    </message>
    <message>
        <location filename="configuresignet.ui" line="150"/>
        <source>RDM Target TUID</source>
        <translation>TUID de destino RDM</translation>
    </message>
    <message>
        <location filename="configuresignet.ui" line="155"/>
        <source>RDM Endpoint</source>
        <translation>Endpoint RDM</translation>
    </message>
    <message>
        <location filename="configuresignet.ui" line="160"/>
        <source>Node Address</source>
        <translation>Dirección del nodo</translation>
    </message>
    <message>
        <location filename="configuresignet.ui" line="169"/>
        <source>Discovered Nodes</source>
        <translation>Nodos detectados</translation>
    </message>
    <message>
        <location filename="configuresignet.ui" line="179"/>
        <source>TUID</source>
        <translation>TUID</translation>
    </message>
    <message>
        <location filename="configuresignet.ui" line="184"/>
        <source>IP Address</source>
        <translation>Dirección IP</translation>
    </message>
    <message>
        <location filename="configuresignet.ui" line="189"/>
        <source>Label</source>
        <translation>Etiqueta</translation>
    </message>
    <message>
        <location filename="configuresignet.ui" line="194"/>
        <source>Roles</source>
        <translation>Roles</translation>
    </message>
    <message>
        <location filename="configuresignet.ui" line="199"/>
        <source>Endpoints</source>
        <translation>Endpoints</translation>
    </message>
    <message>
        <location filename="configuresignet.ui" line="204"/>
        <source>Status</source>
        <translation>Estado</translation>
    </message>
    <message>
        <location filename="configuresignet.cpp" line="104"/>
        <source>Inputs</source>
        <translation>Entradas</translation>
    </message>
    <message>
        <location filename="configuresignet.cpp" line="111"/>
        <source>Outputs</source>
        <translation>Salidas</translation>
    </message>
    <message>
        <location filename="configuresignet.cpp" line="182"/>
        <source>Node</source>
        <translation>Nodo</translation>
    </message>
    <message>
        <location filename="configuresignet.cpp" line="184"/>
        <source>Sender</source>
        <translation>Emisor</translation>
    </message>
    <message>
        <location filename="configuresignet.cpp" line="186"/>
        <source>Manager</source>
        <translation>Gestor</translation>
    </message>
    <message>
        <location filename="configuresignet.cpp" line="188"/>
        <source>Visualiser</source>
        <translation>Visualizador</translation>
    </message>
    <message>
        <location filename="configuresignet.cpp" line="191"/>
        <source>Offboarded</source>
        <translation>Desvinculado</translation>
    </message>
    <message>
        <location filename="configuresignet.cpp" line="191"/>
        <source>Onboarded</source>
        <translation>Vinculado</translation>
    </message>
    <message>
        <location filename="configuresignet.cpp" line="217"/>
        <source>Invalid TUID</source>
        <translation>TUID no válido</translation>
    </message>
    <message>
        <location filename="configuresignet.cpp" line="217"/>
        <source>The local TUID must be a 12-character hexadecimal value.</source>
        <translation>El TUID local debe ser un valor hexadecimal de 12 caracteres.</translation>
    </message>
    <message>
        <location filename="configuresignet.cpp" line="227"/>
        <source>Invalid Root Key</source>
        <translation>Clave raíz no válida</translation>
    </message>
    <message>
        <location filename="configuresignet.cpp" line="227"/>
        <source>K0 must be a 64-character hexadecimal value.</source>
        <translation>K0 debe ser un valor hexadecimal de 64 caracteres.</translation>
    </message>
</context>
<context>
    <name>SigNetPlugin</name>
    <message>
        <location filename="signetplugin.cpp" line="166"/>
        <source>This plugin provides secure DMX and RDM transport over the Sig-Net protocol.</source>
        <translation>Este plugin proporciona transporte seguro de DMX y RDM mediante el protocolo Sig-Net.</translation>
    </message>
    <message>
        <location filename="signetplugin.cpp" line="288"/>
        <source>Output</source>
        <translation>Salida</translation>
    </message>
    <message>
        <location filename="signetplugin.cpp" line="292"/>
        <location filename="signetplugin.cpp" line="316"/>
        <source>Status: Not open</source>
        <translation>Estado: No abierto</translation>
    </message>
    <message>
        <location filename="signetplugin.cpp" line="296"/>
        <location filename="signetplugin.cpp" line="320"/>
        <source>Status: Open</source>
        <translation>Estado: Abierto</translation>
    </message>
    <message>
        <location filename="signetplugin.cpp" line="297"/>
        <source>Security ready</source>
        <translation>Seguridad lista</translation>
    </message>
    <message>
        <location filename="signetplugin.cpp" line="297"/>
        <source>Yes</source>
        <translation>Sí</translation>
    </message>
    <message>
        <location filename="signetplugin.cpp" line="297"/>
        <source>No</source>
        <translation>No</translation>
    </message>
    <message>
        <location filename="signetplugin.cpp" line="298"/>
        <source>Scope</source>
        <translation>Ámbito</translation>
    </message>
    <message>
        <location filename="signetplugin.cpp" line="299"/>
        <source>Discovered nodes</source>
        <translation>Nodos detectados</translation>
    </message>
    <message>
        <location filename="signetplugin.cpp" line="300"/>
        <source>Packets sent</source>
        <translation>Paquetes enviados</translation>
    </message>
    <message>
        <location filename="signetplugin.cpp" line="312"/>
        <source>Input</source>
        <translation>Entrada</translation>
    </message>
    <message>
        <location filename="signetplugin.cpp" line="321"/>
        <source>Packets received</source>
        <translation>Paquetes recibidos</translation>
    </message>
</context>
</TS>
