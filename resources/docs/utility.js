// Replaces internal "qrc:/*.png" image URIs with web-friendly URIs
function replaceqrc()
{
    var imgs = document.images;
    for (var i = 0; i < imgs.length; i++)     
    {
        var src = imgs[i].src;
        imgs[i].src = src.replace("qrc:", "../icons/png");
    }
}

