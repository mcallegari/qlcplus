project(scripts)

set(SCRIPT_FILES
    alternate.js
    balls.js
    blinder.js
    circles.js
    circular.js
    evenodd.js
    fill.js
    fillfromcenter.js
    fillunfill.js
    fillunfillfromcenter.js
    fillunfillsquaresfromcenter.js
    fireworks.js
    flyingobjects.js
    gradient.js
    lines.js
    marquee.js
    noise.js
    onebyone.js
    opposite.js
    plasma.js
    randomcolumn.js
    randomfillcolumn.js
    randomfillrow.js
    randomfillsingle.js
    randompixelperrow.js
    randompixelperrowmulticolor.js
    randomrow.js
    randomsingle.js
    sinewave.js
    snowbubbles.js
    squares.js
    squaresfromcenter.js
    starfield.js
    stripes.js
    stripesfromcenter.js
    strobe.js
    verticalfall.js
    waves.js
)

# Only perform the copy if we're building for Android
if(ANDROID)
    # Define the destination directory for the script files within the Android package
    set(SCRIPTS_ASSETS_DIR "${ANDROID_PACKAGE_SOURCE_DIR}/${INSTALLROOT}/${RGBSCRIPTDIR}")

    # Create the scripts assets directory if it doesn't exist
    file(MAKE_DIRECTORY ${SCRIPTS_ASSETS_DIR})

    # Copy the script files to the assets directory
    foreach(SCRIPT_FILE IN LISTS SCRIPT_FILES)
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${SCRIPT_FILE} DESTINATION ${SCRIPTS_ASSETS_DIR})
    endforeach()
endif()

install(FILES ${SCRIPT_FILES} DESTINATION ${INSTALLROOT}/${RGBSCRIPTDIR})
