echo "Processing $1..."
for dep in `otool -L $1 | grep opt | cut -d ' ' -f 1`
do
    if [[ "$dep" == *$HOMEBREW_PREFIX* ]]; then
        subst=${dep##*/}
        echo "Regular lib: $dep ($subst)"
        # check missing dependency
        if [ ! -f ~/QLC+.app/Contents/Frameworks/$subst ]; then
            echo "Dependency missing: $subst. Adding it to target..."
            cp $dep ~/QLC+.app/Contents/Frameworks/
        fi

        install_name_tool -change $dep @executable_path/../Frameworks/$subst $1
    fi
done