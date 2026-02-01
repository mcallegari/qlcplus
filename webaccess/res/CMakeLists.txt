project(webfiles)

# Set the files for the webfiles target
set(WEBFILES_COMMON
    common.css
    keypad.html
    networkconfig.js
    configuration.js
    websocket.js
    favicon.ico
    favicon-192x192.png
)

set(WEBFILES_QML
    simpledesk-v5.css
    simpledesk-v5.js
    webaccess-v5.css
    webaccess-v5.js
    webaccess-v5.html
)

set(WEBFILES_CLASSIC
    simpledesk.css
    simpledesk.js
    virtualconsole.css
    virtualconsole.js
)

if(qmlui)
    set(WEBFILES ${WEBFILES_COMMON} ${WEBFILES_QML})
else()
    set(WEBFILES ${WEBFILES_COMMON} ${WEBFILES_CLASSIC})
endif()

# Set the path for the webfiles target
set(WEBFILES_PATH "${INSTALLROOT}/${WEBFILESDIR}")

# Install the webfiles target to the specified path
install(FILES ${WEBFILES} DESTINATION ${WEBFILES_PATH})
