# Scene editor lifetime regression

Configure a desktop build with `-Dqmlui=ON`, then run:

```sh
cmake --build build --target check-sceneeditor
```

This builds the real frontend backend and runs four focused Qt tests offscreen:
destroyed delegates, replacement registration, fixture-list rebuilding, and
replacement Scene consoles. The target also runs as a dependency of `check`.
It is excluded from ordinary builds to avoid recompiling the frontend solely
for tests.

For 100 native Scene open/add-fixture/edit/close/GC cycles, run the test executable
with `QLC_SCENE_UI_TEST=1` and the native Qt platform plugin. For example, on
Windows with the build and Qt DLL directories on `PATH`:

```powershell
$env:QLC_SCENE_UI_TEST = '1'
$env:QT_QPA_PLATFORM = 'windows'
./build/qmlui/test/sceneeditor_test.exe
```

The native test creates two generic dimmers and one Scene in memory, uses
temporary QSettings, and does not load a show or patch external I/O.
