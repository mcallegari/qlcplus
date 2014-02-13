// Development tool access
var testAlgo;

(
    function()
    {
        var algo = new Object;
        algo.apiVersion = 1;
        algo.name = "Fill Unfill Squares From Center";
        algo.author = "plg";

        algo.rgbMap = function(width, height, rgb, step)
        {
            var widthCenter = Math.floor((parseInt(width) + 1) / 2) - 1;
            var heightCenter = Math.floor((parseInt(height) + 1) / 2) - 1;
            var isWidthEven = (width % 2 == 0);
            var isHeightEven = (height % 2 == 0);
            var centerStep = (widthCenter > heightCenter ? widthCenter : heightCenter) + 1;

            var map = new Array(height);
            for (var y = 0; y < height; y++)
            {
                map[y] = new Array();
                for (var x = 0; x < width; x++)
                {
                    if (step < centerStep)
                    {
                        if ((x <= widthCenter + step + (isWidthEven ? 1 : 0 ) && x >= widthCenter - step) &&
                                (y <= heightCenter + step + (isHeightEven ? 1 : 0) && y >= heightCenter - step))
                            map[y][x] = rgb;
                        else
                            map[y][x] = 0;
                    }
                    else
                    {
                        var step2 = step - centerStep;
                        if ((x <= widthCenter + step2 + (isWidthEven ? 1 : 0 ) && x >= widthCenter - step2) &&
                                (y <= heightCenter + step2 + (isHeightEven ? 1 : 0) && y >= heightCenter - step2))
                            map[y][x] = 0;
                        else
                            map[y][x] = rgb;
                    }
                }
            }

            return map;
        }

        algo.rgbMapStepCount = function(width, height)
        {
            width = parseInt(width);
            height = parseInt(height);
            var max = width > height ? width : height;
            return (Math.floor((max + 1) / 2)) * 2 - 1;
        }

        // Development tool access
        testAlgo = algo;

        return algo;
    }
)()
