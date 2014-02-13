// Development tool access
var testAlgo;

(
    function()
    {
        var algo = new Object;
        algo.apiVersion = 1;
        algo.name = "Fill Unfill Rows";
        algo.author = "plg";

        algo.rgbMap = function(width, height, rgb, step)
        {
            var map = new Array(height);
            for (var y = 0; y < height; y++)
            {
                map[y] = new Array();
                for (var x = 0; x < width; x++)
                {
                    if (step < height)
                    {
                        if (y <= step)
                            map[y][x] = rgb;
                        else
                            map[y][x] = 0;
                    }
                    else
                    {
                        if (y > step - height)
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
            return (height * 2) - 1;
        }

        // Development tool access
        testAlgo = algo;

        return algo;
    }
)()
