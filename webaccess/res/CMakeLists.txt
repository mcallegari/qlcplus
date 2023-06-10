project(webfiles)

# Set the files for the webfiles target
set(WEBFILES
    common.css
    keypad.html
    networkconfig.js
    simpledesk.css
    simpledesk.js
    virtualconsole.css
    virtualconsole.js
    configuration.js
    websocket.js
)

# Set the path for the webfiles target
set(WEBFILES_PATH "${INSTALLROOT}/${WEBFILESDIR}")

# Install the webfiles target to the specified path
install(FILES ${WEBFILES} DESTINATION ${WEBFILES_PATH})