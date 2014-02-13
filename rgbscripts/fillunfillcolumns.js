// Development tool access
var testAlgo;

(
    function()
    {
        var algo = new Object;
        algo.apiVersion = 1;
        algo.name = "Fill Unfill Columns";
        algo.author = "plg";

        algo.rgbMap = function(width, height, rgb, step)
        {
            var map = new Array(height);
            for (var y = 0; y < height; y++)
            {
                map[y] = new Array();
                for (var x = 0; x < width; x++)
                {
                    if (step < width)
                    {
                        if (x <= step)
                            map[y][x] = rgb;
                        else
                            map[y][x] = 0;
                    }
                    else
                    {
                        if (x > step - width)
                            map[y][x] = rgb;
                        else
                            map[y][x] = 0;
                    }
                }
            }

            return map;
        }

        algo.rgbMapStepCount = function(width, height)
        {
            return (width * 2) - 1;
        }

        // Development tool access
        testAlgo = algo;

        return algo;
    }
)()
