// Replaces internal "qrc:/*.png" and "file:" image URIs with web-friendly URIs
// Replaces "file:" links
function replaceqrc()
{
    var imgs = document.images;
    for (var i = 0; i < imgs.length; i++)     
    {
        var src = imgs[i].src;
        src = src.replace("qrc:", "../gfx");
        imgs[i].src = src.replace("file:///", "");
    }

    var links = document.links;
    for (var i = 0; i < links.length; i++)     
    {
        var href = links[i].href;
        links[i].href = href.replace("file://", "");
    }
}

